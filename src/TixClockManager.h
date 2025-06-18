/*
  TixClockManger.h - Library for display clock
  Jun 2025.
  Released into the public domain.
*/
#pragma once

#include "Arduino.h"
#include <Adafruit_NeoPixel.h> 

class TixClockManager {
    public:
    TixClockManager(Adafruit_NeoPixel* _strip);
    void showtime(struct tm timeInfo) ;

private:
    Adafruit_NeoPixel *_strip;
    int *getSeparateDigits(int input_digit);
    void showTixHour(int *hours_a);
    void showTixMinutes(int *mins_a);
    int *getRandomLeds(int led_Count, int range_max);
    int linearSearch(int *arr, int arr_len, int target);
    void setLedGroupAndColor(int *legGroup, int legGroup_len, int *enableIndexes, int enabled_len, uint32_t ledColor);
};