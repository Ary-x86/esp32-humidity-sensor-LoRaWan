# AWS Lambda Function for KPN -> Arduino Cloud Bridge

This Python script receives data from a KPN Things destination and forwards it to the Arduino Cloud API.

## Deployment

1.  Create a Python 3.x Lambda function in the AWS console.
2.  Deploy this `lambda_function.py` code.
3.  Create a Lambda Layer by zipping the dependencies from `requirements.txt` and attach it to the function.
4.  Set the following Environment Variables in the Lambda configuration:
    * `ARDUINO_CLIENT_ID`
    * `ARDUINO_CLIENT_SECRET`
    * `ARDUINO_THING_ID`
    * `ARDUINO_PROPERTY_ID`
    * `KPN_SHARED_SECRET` (from your KPN destination setup)
5.  Create a public Function URL and use it as the endpoint in your KPN Destination.
