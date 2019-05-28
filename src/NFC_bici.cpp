#include <NFC_bici.h>
#include <array>
#include <cstdint>
#include <freertos/FreeRTOS.h>
#include <soc/rtc.h>
#include <Arduino.h>
//constexpr uint8_t irq = 13;
//constexpr uint8_t reset = 12;
constexpr uint8_t mosi = 23;
constexpr uint8_t miso = 19;
constexpr uint8_t clk = 18;
constexpr uint8_t cs = 5;
inline void ReadUuid(Adafruit_PN532 &nfc, uint8_t *uuid)
{
    uint8_t *uuidIterator = uuid;
    for (auto i = 0; i < 4; ++i)
    {
        nfc.ntag2xx_ReadPage(4 + i, uuidIterator);
        uuidIterator += 4;
    }
}

void bici::NFC(void *parameter)
{
    Adafruit_PN532 nfc{clk, miso, mosi, cs};
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    while (!versiondata)
    {
        Serial.println("Did not find the shield - locking up");
        versiondata = nfc.getFirmwareVersion();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    nfc.SAMConfig();

    //constexpr auto IRQ = 0x03;
    uint32_t notifiedValue = 0;

    while (1)
    {
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, portMAX_DELAY);
        bool tag;
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
        uint8_t uidLength;
        if (notifiedValue == 0x03)
        {
            tag = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
            if (!tag)
            {
                continue;
            }
            //auto val = digitalRead(13);
            //Serial.println(val);
            uint8_t readedUuid[16];
            ReadUuid(nfc, readedUuid);
            String code = "";
            for (int i = 0; i < uidLength; i++)
            {
                code += String(uid[i], HEX);
            }
            code.toUpperCase();
            Serial.println(code);
            if (code == "47EB9AAAB5C81" || code == "1234" || code == "92FE366")//
            {
                auto servoTask = reinterpret_cast<TaskHandle_t *>(parameter);
                xTaskNotify(*servoTask, 2, eSetBits);
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}