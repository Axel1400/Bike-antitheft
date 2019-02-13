#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <driver/ledc.h>
#include <NFC_bici.h>
#include <GPS_bici.h>
#include <Servo_bici.h>
#include <WiFi.h>
#include <soc/rtc.h>
void setup()
{
    Serial.begin(115200);

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
    
    configurePin(4);
    configurePin(13);
    gpio_install_isr_service(0);

    gpio_isr_handler_add(GPIO_NUM_4, [](void *pin) IRAM_ATTR {
        auto higherPriorityTask = pdFALSE;
        auto servoTask = reinterpret_cast<TaskHandle_t *>(pin);
        xTaskNotifyFromISR(*servoTask, 2, eSetBits, &higherPriorityTask);
    },(void *)&servoTask);

    
    gpio_isr_handler_add(GPIO_NUM_13, [](void *pin) IRAM_ATTR {
        auto higherPriorityTask = pdFALSE;
        auto gpstask = reinterpret_cast<TaskHandle_t *>(pin);
        xTaskNotifyFromISR(*gpstask, 0x03, eSetBits, &higherPriorityTask);
    }, (void *)&servoTask);
    pinMode(21, OUTPUT);
/*
    while (1)
    {
        digitalWrite(21, 1);
        delay(7000);
        digitalWrite(21, 0);
        delay(7000);
    }
*/
    while(1){
        vTaskDelay(portMAX_DELAY);
    }
}

void loop()
{
}
