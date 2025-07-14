// This JavaScript code should be pasted into the
// "Payload formatters" > "Uplink" > "Custom Javascript formatter"
// section of your device on The Things Network console.

function decodeUplink(input) {
  // Our payload is just one byte representing the moisture percentage (0-100)
  var moisture_percent = input.bytes[0];
  
  // Return a decoded JSON object.
  // This JSON can be easily read by other integrations, like Datacake.
  return {
    data: {
      // We use "temperature" as the field name to match the Datacake setup
      temperature: moisture_percent 
    },
    warnings: [],
    errors: []
  };
}
