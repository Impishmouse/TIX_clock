/*
  Led9x3Matrix.h - Library for display Air Alarm Statuses.
  Jun 2025.
  Released into the public domain.
*/
#pragma once

#include "Arduino.h"
#include <Adafruit_NeoPixel.h> //neopixel для управління стрічкою

class Led9x3Matrix {
    public:
        Led9x3Matrix(Adafruit_NeoPixel* _strip);
        void begin();
        void wifiConnecting(int index);
        void wifiConnectSuccess();
        void wifiConnectProblem();
        void wifiAccessPoint();
        void breathAnimation(uint red, uint green, uint blue);
        void praporAnimation();

      private:
        Adafruit_NeoPixel *_strip;
        void displayStaticPrapor(int wait);
        void wifiIconDraw(char statusChar);
        
};
