#include "arduino_secrets.h"

/*
 * Dit is de gecombineerde code voor je ESP32-S3 om te verbinden met Arduino IoT Cloud,
 * sensorwaarden stabiel uit te lezen, deze in een gauge meter weer te geven,
 * en een trigger te implementeren.
 */

#include "thingProperties.h"
#include "Arduino.h" // Voor analogRead, pinMode, etc.
#include <Arduino_ConnectionHandler.h> // Voor verbindingsafhandeling

// --- Configuratie ---
// Pin voor de vochtigheidssensor
// Voor ESP32-S3 is A0 vaak een alias voor een GPIO die geschikt is voor ADC.
// Controleer de pinout van je specifieke ESP32-S3 board voor beschikbare ADC pinnen.
const int MOIST_PIN = A0;

// Kalibratiewaarden voor de bodemvochtigheidssensor (PAS DEZE AAN NA KALIBRATIE!)
// MEET DEZE WAARDEN ZELF VOOR DE BESTE NAUWKEURIGHEID!
// Dit zijn voorbeeldwaarden; vervang ze met de waarden die je gemeten hebt.
int DROGE_WAARDE = 3400; // De ruwe analoge waarde wanneer de sensor droog is
int NATTE_WAARDE = 1240; // De ruwe analoge waarde wanneer de sensor nat is

// Instellingen voor de sensoruitlezing
const int NUM_METINGEN = 10; // Aantal metingen om te middelen voor stabiliteit
const unsigned long INDIVIDUAL_MEET_INTERVAL = 50; // ms, korte pauze tussen individuele metingen

// Instellingen voor de Arduino Cloud update
const unsigned long CLOUD_UPDATE_INTERVAL = 5000; // ms, interval om sensorwaarden naar de Cloud te sturen (elke 5 seconden)

// Drempelwaarde voor de trigger
// Stel deze in op een percentage dat relevant is voor jouw toepassing (bijv. 20% voor 'droog')
const int TRIGGER_THRESHOLD_PERCENTAGE = 5;

// --- Globale variabelen (automatisch gesynchroniseerd met Arduino Cloud) ---
// Deze worden gedefinieerd in thingProperties.h.
// NIET hier opnieuw declareren, anders krijg je een "multiple definition" fout.
// int soilMoisture; // Wordt gedefinieerd in thingProperties.h
// bool trigger;     // Wordt gedefinieerd in thingProperties.h

// Timing variabele voor de non-blocking Cloud update
unsigned long lastCloudUpdateTime = 0;

// --- Functies ---

// Functie om de ruwe sensorwaarde om te zetten in een vochtigheidspercentage
int getMoisturePercentage(int rawValue) {
  // Voorkom deling door nul als de droge en natte waarden hetzelfde zijn
  if (DROGE_WAARDE == NATTE_WAARDE) {
    return 0; // Of een foutcode, afhankelijk van je voorkeur
  }
  // Bereken het percentage. De formule is '100 - X' omdat een hogere ruwe waarde droger betekent.
  int percentage = 100 - ((rawValue - NATTE_WAARDE) * 100 / (DROGE_WAARDE - NATTE_WAARDE));
  // Zorg ervoor dat het percentage tussen 0 en 100 blijft
  return constrain(percentage, 0, 100);
}

void setup() {
  Serial.begin(115200);
  // Optionele vertraging voor de SeriÃ«le Monitor om op te starten.
  // Kan verwijderd worden voor productiecode.
  delay(1500);

  // Configureer de sensorpin als input
  pinMode(MOIST_PIN, INPUT);

  // Stel de ADC-resolutie in op 12 bits voor de ESP32.
  // Dit geeft een bereik van 0 tot 4095.
  analogReadResolution(12);

  // Initialiseer de eigenschappen van de Arduino IoT Cloud Thing.
  // Deze functie wordt automatisch gegenereerd in thingProperties.h.
  initProperties();

  // Maak verbinding met de Arduino IoT Cloud.
  // ArduinoIoTPreferredConnection wordt gedefinieerd in thingProperties.h
  // en gebruikt de gegevens uit arduino_secrets.h.
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Stel het debugniveau in voor gedetailleerde verbindingsinformatie.
  // 0 = alleen fouten, 1 = basisinformatie, 2 = gedetailleerd.
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();

  Serial.println("Systeem setup voltooid. Start met meten en verbinden met IoT Cloud...");
}

void loop() {
  // Update de Arduino Cloud. Dit is essentieel voor continue communicatie.
  ArduinoCloud.update();

  // Controleer of het tijd is om nieuwe sensorwaarden te lezen en te verzenden
  if (millis() - lastCloudUpdateTime >= CLOUD_UPDATE_INTERVAL) {
    lastCloudUpdateTime = millis();

    // Lees de ruwe sensorwaarde door meerdere metingen te middelen voor stabiliteit
    long totalRawValue = 0; // Gebruik long voor de som om overloop te voorkomen
    for (int i = 0; i < NUM_METINGEN; i++) {
      totalRawValue += analogRead(MOIST_PIN);
      // Korte vertraging tussen individuele metingen voor stabiliteit van de ADC
      delay(INDIVIDUAL_MEET_INTERVAL);
    }
    int raw_moisture = totalRawValue / NUM_METINGEN;

    // Converteer de ruwe waarde naar een vochtigheidspercentage
    // 'soilMoisture' is een globale variabele gedefinieerd in thingProperties.h
    soilMoisture = getMoisturePercentage(raw_moisture);

    // SeriÃ«le output voor debugging
    Serial.print("Ruwe vochtigheidswaarde: ");
    Serial.print(raw_moisture);
    Serial.print(", Bodemvochtigheid: ");
    Serial.print(soilMoisture);
    Serial.println("%");

    // Implementeer de trigger logica
    // De trigger activeert als de bodemvochtigheid onder de drempel komt.
    // 'trigger' is een globale variabele gedefinieerd in thingProperties.h
    if (soilMoisture <= TRIGGER_THRESHOLD_PERCENTAGE) {
      if (!trigger) { // Alleen updaten als de status verandert van 'false' naar 'true'
        trigger = true;
        Serial.println("Bodemvochtigheid is LAAG! Trigger geactiveerd.");
      }
    } else {
      if (trigger) { // Alleen updaten als de status verandert van 'true' naar 'false'
        trigger = false;
        Serial.println("Bodemvochtigheid is voldoende. Trigger gedeactiveerd.");
      }
    }

    // Variabelen 'soilMoisture' en 'trigger' worden automatisch naar de Cloud gesynchroniseerd
    // omdat ze gekoppeld zijn via 'ON_CHANGE' in thingProperties.h.
    // Je hoeft hier GEEN ArduinoCloud.addProperty() of andere expliciete verzendfuncties aan te roepen.
  }
}

/*
 * Deze functies worden automatisch aangeroepen wanneer de corresponderende variabele
 * vanuit de Arduino IoT Cloud (bijv. via het dashboard) wordt gewijzigd.
 * Ze zijn alleen relevant als de variabele is ingesteld als 'READWRITE' in de Cloud.
 */

void onSoilMoistureChange() {
  Serial.print("Nieuwe bodemvochtigheidswaarde ontvangen van Cloud: ");
  Serial.println(soilMoisture);
  // Voeg hier eventueel code toe om lokaal te reageren op een wijziging vanuit de Cloud.
  // Bijvoorbeeld, als je handmatig de gewenste vochtigheid instelt via de Cloud.
}

void onTriggerChange() {
  Serial.print("Trigger waarde veranderd naar: ");
  Serial.println(trigger);
  // Voeg hier eventueel code toe om lokaal te reageren op een wijziging van de trigger vanuit de Cloud.
}