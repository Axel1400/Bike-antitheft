#include <GPS_bici.h>
//#include <Arduino.h>
#include <TinyGPS++.h>
#include <chrono>
#include <thread>
#include <tuple>

void bici::GPS_task(void *parameter)
{
    using namespace std::chrono;
    using namespace std::literals::chrono_literals;
    Serial2.begin(9600);
    auto gps = TinyGPSPlus{};
    
    auto& position = *static_cast<std::tuple<double, double>*>(parameter);
    while (1)
    {
        while (Serial2.available())
        {
            char chr = Serial2.read();
            if (gps.encode(chr))
            {
                
                std::get<0>(position) = gps.location.lat();
                std::get<1>(position) = gps.location.lng();
                
            }
        }
        std::this_thread::sleep_for(5s);
    }
}