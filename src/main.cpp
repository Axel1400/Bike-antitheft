#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <driver/ledc.h>
#include <NFC_bici.h>
#include <GPS_bici.h>
#include <Servo_bici.h>
#include <Temp_bici.h>
#include <WiFi.h>
#include <soc/rtc.h>
//#include <GSM.h>
#define TINY_GSM_MODEM_A6
#include <TinyGsmClient.h>
//#include <BlynkSimpleTinyGSM.h>
#include <BlynkSimpleSIM800.h>

void setup()
{
    HardwareSerial Serial1(1);
    Serial.begin(115200);
    TinyGsm modem(Serial1);
    const char apn[] = "internet.movistar.mx";
    const char user[] = "";
    const char pass[] = "";
    const char auth[] = "250f0a71806e48a3b6acad3e35aa8f58";
    Serial1.begin(115200, SERIAL_8N1, 4, 12, false);
    delay(3000);
    modem.restart();
    String modemInfo = modem.getModemInfo();
    Serial.print("Modem: ");
    Serial.println(modemInfo);
    TinyGsmClient client(modem);
    //Blynk.begin(auth, modem, apn, user, pass);
    /*
    TaskHandle_t gsmtask;
    xTaskCreate(
        bici::GSM,
        "GSM",
        10000,
        nullptr,
        1,
        &gsmtask);
    */

    TaskHandle_t gpstask;
    xTaskCreate(
        bici::GPS_task,
        "GPS",
        10000,
        nullptr,
        1,
        &gpstask);

    TaskHandle_t servoTask;

    xTaskCreate(
        bici::servoTask,
        "Servo",
        10000,
        nullptr,
        1,
        &servoTask);

    TaskHandle_t temptask;
    xTaskCreate(
        bici::Temp,
        "Temp",
        10000,
        &servoTask,
        1,
        &temptask);

    TaskHandle_t NFCtask;
    xTaskCreate(
        bici::NFC,
        "NFC",
        10000,
        &servoTask,
        1,
        nullptr);

    auto configurePin = [](uint32_t pin) {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_INTR_NEGEDGE;
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pin_bit_mask = (1ULL << pin);
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        gpio_config(&io_conf);

        gpio_set_intr_type(static_cast<gpio_num_t>(pin), GPIO_INTR_NEGEDGE);
        gpio_intr_enable(static_cast<gpio_num_t>(pin)); // Enable the pin for interrupts
    };

    configurePin(14);
    configurePin(13);
    gpio_install_isr_service(0);

    gpio_isr_handler_add(GPIO_NUM_14, [](void *pin) IRAM_ATTR {
        auto higherPriorityTask = pdFALSE;
        auto servoTask = reinterpret_cast<TaskHandle_t *>(pin);
        xTaskNotifyFromISR(*servoTask, 2, eSetBits, &higherPriorityTask);
    },
                         (void *)&servoTask);
    
    gpio_isr_handler_add(GPIO_NUM_13, [](void *pin) IRAM_ATTR {
        auto higherPriorityTask = pdFALSE;
        auto NFCtask = reinterpret_cast<TaskHandle_t *>(pin);
        xTaskNotifyFromISR(*NFCtask, 0x03, eSetBits, &higherPriorityTask);
    },
                         (void *)&NFCtask);
    pinMode(21, OUTPUT);
    Serial.print("HOLA");
    while (1)
    {
        vTaskDelay(portMAX_DELAY);
    }
}

void loop()
{
    //Blynk.run;
}
