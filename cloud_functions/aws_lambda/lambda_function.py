import json
import os
import hashlib
import iot_api_client as iot
from oauthlib.oauth2 import BackendApplicationClient
from requests_oauthlib import OAuth2Session

def lambda_handler(event, context):
    """
    Receives data from a KPN Things destination, verifies the source, authenticates with
    Arduino Cloud, and publishes the property value.
    """
    print("Received event: " + json.dumps(event, indent=2))

    # --- Get Credentials & Secrets from Environment Variables ---
    try:
        client_id = os.environ['ARDUINO_CLIENT_ID']
        client_secret = os.environ['ARDUINO_CLIENT_SECRET']
        thing_id = os.environ['ARDUINO_THING_ID']
        property_id = os.environ['ARDUINO_PROPERTY_ID']
        kpn_secret = os.environ.get('KPN_SHARED_SECRET')
    except KeyError as e:
        print(f"ERROR: Missing environment variable: {e}")
        return {'statusCode': 500, 'body': f"Configuration error: Missing {e}"}

    # --- Verify the request is from KPN Things using the Shared Secret ---
    if kpn_secret:
        try:
            headers = {k.lower(): v for k, v in event.get('headers', {}).items()}
            received_token = headers.get('things-message-token')
            request_body_str = event.get('body', '')
            if not received_token: raise ValueError("things-message-token header not found.")
            
            input_string = request_body_str + kpn_secret
            calculated_token = hashlib.sha256(input_string.encode('utf-8')).hexdigest()

            if received_token != calculated_token:
                print("ERROR: Token mismatch. Request is not authentic.")
                return {'statusCode': 401, 'body': "Unauthorized"}
            
            print("KPN Things request verified successfully.")
        except Exception as e:
            print(f"ERROR during token verification: {e}")
            return {'statusCode': 400, 'body': "Token verification failed."}

    # --- Extract Sensor Value from KPN's SenML Payload ---
    try:
        body = json.loads(event.get('body', '{}'))
        sensor_value = next((record['v'] for record in body if record.get('n') == 'temperature'), None)
        if sensor_value is None:
            raise ValueError("Temperature reading not found in SenML payload")
        print(f"Extracted sensor value: {sensor_value}")
    except Exception as e:
        print(f"ERROR: Could not parse sensor value. Error: {e}")
        return {'statusCode': 400, 'body': "Invalid request body format."}

    # --- Authenticate with Arduino Cloud and Publish ---
    try:
        print("Attempting to fetch Arduino Cloud access token...")
        oauth_client = BackendApplicationClient(client_id=client_id)
        token_url = "https://api2.arduino.cc/iot/v1/clients/token"
        oauth = OAuth2Session(client=oauth_client)
        token_data = oauth.fetch_token(
            token_url=token_url, client_id=client_id, client_secret=client_secret,
            audience="https://api2.arduino.cc/iot", include_client_id=True
        )
        access_token = token_data.get("access_token")
        if not access_token: raise Exception("Failed to get access token from response.")
        
        print(f"Publishing value '{sensor_value}'...")
        client_config = iot.Configuration(host="https://api2.arduino.cc")
        client_config.access_token = access_token
        api_client = iot.ApiClient(client_config)
        properties_api = iot.PropertiesV2Api(api_client)
        property_value = iot.PropertyValue(value=sensor_value)
        properties_api.properties_v2_publish(thing_id, property_id, property_value)
        print("Successfully published value to Arduino Cloud.")
    except Exception as e:
        print(f"ERROR: Failed during Arduino publish step. Error: {e}")
        return {'statusCode': 500, 'body': "Arduino Cloud publish failed."}

    return {
        'statusCode': 200,
        'body': json.dumps('Successfully updated Arduino Cloud!')
    }
