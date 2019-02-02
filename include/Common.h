#pragma once

#ifdef BICI_DEBUG
    #define DEBUG_PRINT(x) Serial.print(x)
#else
    #define DEBUG_PRINT(x) (void)0
#endif