#include <NFC_bici.h>
#include <array>
#include <cstdint>
//constexpr uint8_t irq = 13;
constexpr uint8_t reset = 12;

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
        // Serial.println("Did not find the shield - locking up");
        versiondata = nfc.getFirmwareVersion();
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
    nfc.SAMConfig();
    constexpr auto IRQ = 0x03;
    uint32_t notifiedValue = 0;

    while (1)
    {
        bool tag;
        uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
        uint8_t uidLength;
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, portMAX_DELAY);
        portDISABLE_INTERRUPTS();
        if (notifiedValue == 0x03)
        {
            tag = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
            if (!tag)
            {
                // aqui haces algo en caso de que no se haya leido bien
                // se lee así
            }
            uint8_t readedUuid[16];
            ReadUuid(nfc, readedUuid);

            String code = "";
            for (int i = 0; i < uidLength; i++)
            {
                code += String(uid[i], HEX);
            }
            code.toUpperCase();
            Serial.println(code);
            delay(1000);

            // TODO cambiar por valor de una memoria no volatil
            uint8_t uuid[16];
            auto equals = true;
            for (auto it = readedUuid; it != readedUuid + 16; ++it)
            {
                if (*it != *uuid)
                {
                    // BOOM no son iguales 🤔
                    equals = false;
                    break;
                }
            }
            /*
        if (readedUuid == uuid)
        {
            bici::
        }
*/
        }
        portENABLE_INTERRUPTS();

        vTaskDelay(10000);
    }
}