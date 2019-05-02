#include <GPS_bici.h>
#include <Arduino.h>
#include <TinyGPS++.h>
#include <chrono>
#include <thread>

void bici::GPS_task(void *parameter)
{
    
    using namespace std::chrono;
    using namespace std::literals::chrono_literals;
    Serial2.begin(9600);
    auto gps = TinyGPSPlus{};
    while (1)
    {
        while (Serial2.available())
        {
            char chr = Serial2.read();
            if (gps.encode(chr))
            {  
                
                Serial.print("Lat=");
                Serial.print(gps.location.lat(), 10);
                Serial.print(" Long=");
                Serial.print(gps.location.lng(), 10);
                Serial.println();
                
            }
        }
        std::this_thread::sleep_for(5s);
    }
    
}