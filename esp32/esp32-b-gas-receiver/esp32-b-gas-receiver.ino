#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// --------------------------------------------------
// ESP-NOW receive callback
// --------------------------------------------------

void onDataRecv(
    const esp_now_recv_info_t *info,
    const uint8_t *data,
    int len
)
{
    // Make a null-terminated copy of the received packet
    char packet[250];

    if (len >= sizeof(packet))
    {
        Serial.println("ERROR: Packet too large");
        return;
    }

    memcpy(packet, data, len);
    packet[len] = '\0';

    Serial.println();
    Serial.println("================================");
    Serial.println("       GAS SENSOR DATA");
    Serial.println("================================");

    Serial.print("Packet  : ");
    Serial.println(packet);

    // --------------------------------------------------
    // Expected format:
    //
    // GAS,000001,216,66,161
    //
    //        │      │   │   │
    //        │      │   │   └── CO
    //        │      │   └────── Ethanol
    //        │      └────────── NO2
    //        └───────────────── Sequence number
    // --------------------------------------------------

    char type[10];
    unsigned long sequence;
    int no2;
    int ethanol;
    int co;

    int fields = sscanf(
        packet,
        "%9[^,],%lu,%d,%d,%d",
        type,
        &sequence,
        &no2,
        &ethanol,
        &co
    );

    // --------------------------------------------------
    // Validate packet format
    // --------------------------------------------------

    if (fields != 5)
    {
        Serial.println();
        Serial.println("STATUS  : INVALID");
        Serial.println("Reason  : Invalid packet format");
        Serial.println("================================");

        return;
    }

    if (strcmp(type, "GAS") != 0)
    {
        Serial.println();
        Serial.println("STATUS  : INVALID");
        Serial.println("Reason  : Unknown packet type");
        Serial.println("================================");

        return;
    }

    // --------------------------------------------------
    // Display decoded gas values
    // --------------------------------------------------

    Serial.println();
    Serial.print("Sequence : ");
    Serial.println(sequence);

    Serial.println();

    Serial.print("NO2      : ");
    Serial.print(no2);
    Serial.println(" RAW");

    Serial.print("Ethanol  : ");
    Serial.print(ethanol);
    Serial.println(" RAW");

    Serial.print("CO       : ");
    Serial.print(co);
    Serial.println(" RAW");

    Serial.println();
    Serial.println("STATUS   : VALID");
    Serial.println("================================");
}


// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup()
{
    Serial.begin(115200);

    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println("       ESP-NOW GAS RECEIVER");
    Serial.println("================================");

    // Put ESP32 into WiFi station mode
    WiFi.mode(WIFI_STA);

    Serial.print("ESP32-B MAC: ");
    Serial.println(WiFi.macAddress());

    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ERROR: ESP-NOW initialization failed");
        return;
    }

    // Register receive callback
    esp_now_register_recv_cb(onDataRecv);

    Serial.println("ESP-NOW receiver ready.");
    Serial.println("Waiting for gas sensor data...");
}


// --------------------------------------------------
// Main loop
// --------------------------------------------------

void loop()
{
    // Nothing required here.
    // ESP-NOW reception is handled by the callback.
}