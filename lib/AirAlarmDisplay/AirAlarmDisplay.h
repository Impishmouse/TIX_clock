/*
  AirAlarmDisplay.h - Library for display Air Alarm Statuses.
  May 2023.
  Released into the public domain.
*/

#ifndef AirAlarmDisplay_h
#define AirAlarmDisplay_h

#include "Arduino.h"
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <NTPClient.h> //Час

class AirAlarmDisplay
{
    public:
      AirAlarmDisplay(int disY);
      void begin();
      void wifiConnect(char *ssid, int bitmapType);
      void wifiConnectProblem(char *APSsid, char *APPassword);
      String utf8cyr(String source);
      void oledDisplayCenter(String text, int y, int screenWidth, int offset);
      void displayTimeInfo(NTPClient timeClient, int displayMode);
      void displayWeatherInfo(int weatherId, String weather);
      void displayAlarmInfo(bool myCityEnable, String durationTransformed);
      void displayWarInfo(int warMode, bool canShow, int warData);
      void displayClear();
      void displayCurrency(int currencyMode, float usdBuy, float usdSale, float eurBuy, float eurSale);

    private:
        int _disy;
};

#endif