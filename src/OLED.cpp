#include <OLED.h>
#include <string.h>
#include <Wire.h>
#include <driver/adc.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <driver/touch_pad.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/task.h>
#include <SSD1306.h>
#include <Arduino.h>
#include <font8x8_basic.h>
#include <esp_adc_cal.h>
#define tag "SSD1306"
const int T[] = {0x24, 0x2E, 0x6B, 0x6B, 0x3A, 0x12, 0x00, 0x00};
auto a = 0;
void i2c_masterinit()
{
    //i2c_cmd_handle_t cmd;
    i2c_config_t i2c_config;
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = GPIO_NUM_21;
    i2c_config.scl_io_num = GPIO_NUM_22;
    i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    i2c_config.master.clk_speed = 400000;

    i2c_param_config(I2C_NUM_0, &i2c_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
}
void cleardisplay()
{

    for (uint8_t i = 0; i < 8; i++)
    {
        i2c_cmd_handle_t cmd;
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
        i2c_master_write_byte(cmd, 0xB0 | i, true);
        i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
        for (uint8_t j = 0; j < 128; j++)
        {
            i2c_master_write_byte(cmd, 0x00, true);
        }
        i2c_master_stop(cmd);
        i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
        i2c_cmd_link_delete(cmd);
    }
}
void displayinit()
{
    esp_err_t espRc;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_STREAM, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_CHARGE_PUMP, true);
    i2c_master_write_byte(cmd, 0x14, true);

    i2c_master_write_byte(cmd, OLED_CMD_SET_SEGMENT_REMAP, true); // reverse left-right mapping
    i2c_master_write_byte(cmd, OLED_CMD_SET_COM_SCAN_MODE, true); // reverse up-bottom mapping

    i2c_master_write_byte(cmd, OLED_CMD_DISPLAY_ON, true);
    i2c_master_stop(cmd);

    espRc = i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
    if (espRc == ESP_OK)
    {
        ESP_LOGI(tag, "OLED configured successfully");
    }
    else
    {
        ESP_LOGE(tag, "OLED configuration failed. code: 0x%.2X", espRc);
    }
    i2c_cmd_link_delete(cmd);
}
static void Pantalla(void *)
{
    digitalWrite(2,(!digitalRead(2)));
    Serial.println("Interrupción");
    if (a == 0)
    {
        a = 1;
    }
    else
    {
        a = 0;
    }
}
void bici::OLED(void *parameter)
{
    //Tactil

    static const char *TAG = "Touch pad";
    ESP_LOGI(TAG, "Initializing Touch Pad");
    touch_pad_init();
    touch_pad_set_fsm_mode(static_cast<touch_fsm_mode_t>(TOUCH_FSM_MODE_TIMER));
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    touch_pad_config(TOUCH_PAD_NUM9, 100);
    touch_pad_filter_start(10);
    touch_pad_set_thresh(TOUCH_PAD_NUM9, 100);
    touch_pad_isr_register(Pantalla, NULL);
    touch_pad_intr_enable();
    /*while(1)
    {
        Serial.println(touch_pad_read_filtered(TOUCH_PAD_NUM9,0));
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }*/
    // I2C

    //ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
    uint32_t bat = 0;
    uint32_t bat1 = 0;
    for (auto i = 0; i < 1024; i++)
    {
        bat = adc1_get_raw(ADC1_CHANNEL_7);
        bat1 += bat;
    }
    //4.2 100%, 3.2v 0%;;;;;;;;,3.3-100%, 2= 0%;
    bat1 /= 1024;
    double volts = (bat1 * 3.3) / 4095;
    volts = volts - 2.5;
    if (volts <= 0)
    {
        volts = 0;
    }
    volts = (volts * 100) / 0.8;
    //Pantalla OLED

    i2c_masterinit();
    displayinit();
    cleardisplay();

    //cleardisplay();
    while (1)
    {
        touch_pad_intr_enable();
        if (a == 0)
        {
            i2c_cmd_handle_t cmd;
            uint32_t x = 0;
            for (uint8_t i = 0; i < 8; i++)
            {
                cmd = i2c_cmd_link_create();
                i2c_master_start(cmd);
                i2c_master_write_byte(cmd, (OLED_I2C_ADDRESS << 1) | I2C_MASTER_WRITE, true);
                i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_CMD_SINGLE, true);
                i2c_master_write_byte(cmd, 0xB0 | i, true);
                i2c_master_write_byte(cmd, OLED_CONTROL_BYTE_DATA_STREAM, true);
                for (uint8_t j = 0; j < 128; j++)
                {
                    i2c_master_write_byte(cmd, 0xF0, true);
                    x++;
                    if (x == 8)
                    {
                        x = 0;
                    }
                }
                i2c_master_stop(cmd);
                i2c_master_cmd_begin(I2C_NUM_0, cmd, 10 / portTICK_PERIOD_MS);
                i2c_cmd_link_delete(cmd);
            }
        }
        else
        {
            cleardisplay();
        }
        touch_pad_intr_disable();
        touch_pad_clear_status();
        vTaskDelay(900 / portTICK_PERIOD_MS);
    }
}