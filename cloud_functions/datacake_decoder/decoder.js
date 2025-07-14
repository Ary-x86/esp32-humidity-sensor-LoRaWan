function Decoder(request) {
    // --- IMPORTANT: Replace with your actual Datacake device serial number ---
    var DATACAKE_SERIAL_NUMBER = "YOUR_DATACAKE_SERIAL_NUMBER_HERE";

    // KPN Things sends data as a SenML JSON array in the request body.
    var senMLPayload = JSON.parse(request.body);

    var measurements = [];

    // Loop through the SenML array to extract all measurements.
    for (var i = 0; i < senMLPayload.length; i++) {
        var record = senMLPayload[i];

        var key = record.n;
        var value = record.v;
        if (value === undefined) value = record.vs;
        if (value === undefined) value = record.vb;

        // Add the measurement to our results, using the hardcoded serial number.
        if (key && (value !== undefined)) {
            measurements.push({
                device: DATACAKE_SERIAL_NUMBER,
                field: key.toUpperCase(),
                value: value
            });
        }
    }
    return measurements;
}
