#include <Temp_bici.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include <soc/rtc.h>
#include <Servo_bici.h>
#include <Arduino.h>
void bici::Temp(void *parameter)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
    while (1)
    {
        uint32_t val = adc1_get_raw(ADC1_CHANNEL_6);
        uint32_t vout = 0;
        for (auto i = 0; i < 1024; i++)
        {
            val = adc1_get_raw(ADC1_CHANNEL_6);
            vout = vout + val;
        }
        vout = vout / 1024;
        if (vout <= 496.36)
        {
            vout = 496.36;
        }
        uint32_t temp = (vout - 496.36) / 24.19;
        Serial.println(temp);
        /*
        if (temp <= 10)
        {
            auto servoTask = reinterpret_cast<TaskHandle_t *>(parameter);
            xTaskNotify(*servoTask, 2, eSetBits);
        }
        */
        if (temp >= 100)
        {
            auto servoTask = reinterpret_cast<TaskHandle_t *>(parameter);
            xTaskNotify(*servoTask, 3, eSetBits);
        }
        vTaskDelay(500);
    }
}