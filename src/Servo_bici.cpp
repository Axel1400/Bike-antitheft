#include <Servo_bici.h>
#include <Arduino.h>
#include <chrono>
#include <thread>

enum class ServoState : int32_t
{
    Open,
    Close
};

void bici::servoTask(void *parameter)
{
    using namespace std::literals::chrono_literals;

    static auto servoState = ServoState::Open;
    constexpr int freq = 50;
    constexpr int ledChannel = 0;
    constexpr int resolution = 13;
    constexpr auto maxDutyCicle = uint32_t{1 << 13};
    constexpr auto servoPin = 2;

    uint32_t notifiedValue = 0;
    ledcAttachPin(servoPin, ledChannel);
    while (1)
    {
        xTaskNotifyWait(0, ULONG_MAX, &notifiedValue, portMAX_DELAY);
        portDISABLE_INTERRUPTS();
        if (notifiedValue == 2)
        {
            //Serial.println("Entra");
            ledcSetup(ledChannel, freq, resolution);
            switch (servoState)
            {
            case ServoState::Open:
                ledcWrite(ledChannel, maxDutyCicle * 0.15); // 819
                servoState = ServoState::Close;
                break;
            case ServoState::Close:
                ledcWrite(ledChannel, maxDutyCicle * 0.03); // 409
                servoState = ServoState::Open;
                break;
            }
            std::this_thread::sleep_for(1s);
            Serial.println("Sale");
        }
        portENABLE_INTERRUPTS();
    }
}