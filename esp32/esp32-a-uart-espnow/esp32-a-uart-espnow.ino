#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

HardwareSerial STM32Serial(1);

static const int STM32_RX_PIN = 20;
static const int STM32_TX_PIN = 21;

// ESP32-B MAC address
uint8_t peerAddress[] = {
    0x58, 0x8C, 0x81, 0xA9, 0xE2, 0x18
};

void onDataSent(const wifi_tx_info_t *info, esp_now_send_status_t status)
{
    Serial.print("ESP-NOW TX: ");

    if (status == ESP_NOW_SEND_SUCCESS)
    {
        Serial.println("SUCCESS");
    }
    else
    {
        Serial.println("FAIL");
    }
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("================================");
    Serial.println(" STM32 UART -> ESP-NOW BRIDGE");
    Serial.println("================================");

    // -----------------------------
    // UART from STM32
    // -----------------------------
    STM32Serial.begin(
        115200,
        SERIAL_8N1,
        STM32_RX_PIN,
        STM32_TX_PIN
    );

    Serial.println("UART configured:");
    Serial.println("  Baud : 115200");
    Serial.println("  RX   : GPIO20");
    Serial.println("  TX   : GPIO21");

    // -----------------------------
    // WiFi / ESP-NOW
    // -----------------------------
    WiFi.mode(WIFI_STA);

    Serial.print("ESP32-A MAC: ");
    Serial.println(WiFi.macAddress());

    if (esp_now_init() != ESP_OK)
    {
        Serial.println("ERROR: ESP-NOW init failed");
        return;
    }

    esp_now_register_send_cb(onDataSent);

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, peerAddress, 6);

    // Let ESP-NOW use the current WiFi channel
    peerInfo.channel = 0;

    // No encryption for this prototype
    peerInfo.encrypt = false;

    if (esp_now_add_peer(&peerInfo) != ESP_OK)
    {
        Serial.println("ERROR: Failed to add ESP32-B peer");
        return;
    }

    Serial.println("ESP-NOW peer added.");
    Serial.println("Waiting for STM32 UART data...");
}

void loop()
{
    if (STM32Serial.available())
    {
        String message = STM32Serial.readStringUntil('\n');

        message.trim();

        if (message.length() > 0)
        {
            Serial.print("UART RX: ");
            Serial.println(message);

            esp_err_t result = esp_now_send(
                peerAddress,
                (uint8_t *)message.c_str(),
                message.length()
            );

            if (result != ESP_OK)
            {
                Serial.print("ESP-NOW send error: ");
                Serial.println(result);
            }
        }
    }
}