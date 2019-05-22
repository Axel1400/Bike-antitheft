#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <driver/ledc.h>
#include <OLED.h>
#include <NFC_bici.h>
#include <GPS_bici.h>
#include <Servo_bici.h>
#include <Temp_bici.h>
#include <WiFi.h>
#include <soc/rtc.h>
#define BLYNK_PRINT Serial
#define TINY_GSM_MODEM_A6
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
#include <driver/i2c.h>
#include <tuple>
BlynkTimer timer;
auto position = std::tuple<double, double>{};
auto Temperature = std::tuple<int>{};
void setup()
{
    pinMode(2, OUTPUT);
    pinMode(27, OUTPUT);
    Serial.begin(115200);
    /*
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

    Blynk.begin(auth, modem, apn, user, pass);
    */
    TaskHandle_t servoTask;
    xTaskCreate(
        bici::servoTask,
        "Servo",
        10000,
        nullptr,
        1,
        &servoTask);

    TaskHandle_t OLEDtask;
    xTaskCreate(
        bici::OLED,
        "OLED",
        10000,
        nullptr,
        1,
        &OLEDtask);

    TaskHandle_t gpstask;
    xTaskCreate(
        bici::GPS_task,
        "GPS",
        10000,
        &position,
        1,
        &gpstask);

    TaskHandle_t temptask;
    xTaskCreate(
        bici::Temp,
        "Temp",
        10000,
        &Temperature,
        1,
        &temptask);

    TaskHandle_t NFCtask;
    xTaskCreate(
        bici::NFC,
        "NFC",
        10000,
        &servoTask,
        1,
        &NFCtask);
    /*timer.setInterval(2000, [] {
        Blynk.virtualWrite(V0, 1, std::get<0>(position), std::get<1>(position), "Bici");
        Blynk.syncVirtual(V2);
        if (V2 == 1)
        {
            Blynk.virtualWrite(V0, 1, std::get<0>(position), std::get<1>(position), "Bici");
        }
        if ((std::get<0>(Temperature)) >= 100)
        {
            auto servoTask = reinterpret_cast<TaskHandle_t *>(parameter);
            xTaskNotify(*servoTask, 3, eSetBits);
        }
    });*/
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
    //Cadena
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << 25);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    gpio_set_intr_type(static_cast<gpio_num_t>(25), GPIO_INTR_NEGEDGE);
    gpio_intr_enable(static_cast<gpio_num_t>(25)); // Enable the pin for interrupts

    gpio_isr_handler_add(GPIO_NUM_25, [](void *pin) IRAM_ATTR {
        Serial.println("Cortaron la cadena");
        auto higherPriorityTask = pdFALSE;
        auto servoTask = reinterpret_cast<TaskHandle_t *>(pin);
        Blynk.notify("Cortaron la cadena, sistema activado");
        xTaskNotifyFromISR(*servoTask, 3, eSetBits, &higherPriorityTask);
    },
                         (void *)&servoTask);

    while (1)
    {
        //Blynk.run();
        //timer.run();
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void loop()
{
}