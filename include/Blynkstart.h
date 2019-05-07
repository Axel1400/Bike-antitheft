#include <Arduino.h>
#define BLYNK_PRINT Serial
#define TINY_GSM_MODEM_A6
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
BlynkTimer timer;

void Blynkinitconfig(void)
{
    const char apn[] = "internet.itelcel.com";
    const char user[] = "webgprs";
    const char pass[] = "webgprs2002";
    const char auth[] = "250f0a71806e48a3b6acad3e35aa8f58";
    HardwareSerial Serial1(1);
    TinyGsm modem(Serial1);

    Serial1.begin(115200, SERIAL_8N1, 4, 12, false);
    delay(3000);

    modem.restart();
    String modemInfo = modem.getModemInfo();
    Serial.print("Modem: ");
    Serial.println(modemInfo);
    //modem.simUnlock("1111");
    Blynk.begin(auth, modem, apn, user, pass);
    //timer.setInterval(1000L, funcion); //timer will run every sec
}