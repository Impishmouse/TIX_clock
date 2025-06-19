/*

Робочий приклад  TIX CLOCK version 1.1.0

*/

// =======================================

#include <ArduinoJson.h> //json для аналізу інформації
#include <Adafruit_NeoPixel.h> //neopixel для управління стрічкою
#include <Led9x3Matrix.h> // Для показу еффектів.
#include <TixClockManager.h>

#include <WiFi.h>
#include <Preferences.h>
#include <WiFiManager.h>
#include <Wire.h>

#include "time.h"
#include <string> 


// ============ НАЛАШТУВАННЯ ============
char ssid[] = "StarLord_02";                  //"StarLord"; //Назва твоєї мережі WiFi
char password[] = "strongWifiPwd";              //"strongWifipwd"; //Пароль від твого WiFi

char APSsid[] = "TixClock"; //Назва точки доступу
char APPassword[] = ""; //Пароль від точки доступу
WiFiManager wm;

// ====== Налаштування яскравості =======
int brightness = 2; //Яскравість %
bool autoBrightness = true; //Ввімкнена/вимкнена авто яскравість
const int dayBrightness = 10; //Денна яскравість %
const int nightBrightness = 1; //Нічна яскравість %
const int day = 9; //Початок дня
const int night = 21; //Початок ночі


// ===== Налаштування світлодіодів ======
#define LED_PIN 13
#define LED_COUNT 27
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Led9x3Matrix ledDisplay(&strip);
TixClockManager tixManager(&strip);
String color;
static int ledColor[25];
void colorWipe(int wait);

// ======= Налаштування часу 
#define NTP_SERVER     "ua.pool.ntp.org"
#define UTC_OFFSET     10800
#define UTC_OFFSET_DST 0
const long  gmtOffset_sec = 7200;

int period = 8000;

const long dateInterval = 200000;
unsigned long lastTime;

static bool wifiConnected;

String mode = "clock"; //Режим

bool enabled = true;
// Перевірка останніх повідомлень

const int   daylightOffset_sec = 3600;

static int ledColorBlue[] =   { 1, 3, 8, 9, 10, 13, 15, 20, 21, 22, 25};
static int ledColorYellow[] = { 0, 4, 7, 6, 11, 12, 16, 17, 19, 23, 24};

void initWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  if (WiFi.status() != WL_CONNECTED)
  {
    // TODO Add indication if problem  connect to wifi

    bool res;
    res = wm.autoConnect(APSsid, APPassword);
    if (!res)
    {
      Serial.println("Помилка підключення");
      ESP.restart();
    }
    else
    {
	    // TODO add indication if connected			
      Serial.println("Підключено :)");
    }
  }
}

void initStrip() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(brightness * 2.55);
  colorWipe(60);
}

// Засвітка світлодіодів у вигляді прапора
void colorWipe(int wait) {
  int count = sizeof(ledColorYellow) / sizeof(int);
  for (int i = 0; i < count; i++) { // For each pixel in strip...
    strip.setPixelColor(ledColorBlue[i], strip.Color(0, 0, 255));
    strip.setPixelColor(ledColorYellow[i], strip.Color(255, 255, 0));//  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

struct tm getLocalTime()
{
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return timeinfo;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  return timeinfo;
}


void setup() 
{
  Serial.begin(115200);

  initStrip();

  initWiFi();

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
}

void loop() {
  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected) {
    if (enabled) {

      if (mode == "flag") 
      {
        ledDisplay.praporAnimation();
      } 
      else if (mode == "flashlight") 
      {
        if (color == "white") ledDisplay.breathAnimation(255, 255, 255);
        if (color == "red") ledDisplay.breathAnimation(255, 0, 0);
        if (color == "orange") ledDisplay.breathAnimation(255, 165, 0);
        if (color == "yellow") ledDisplay.breathAnimation(255, 255, 0);
        if (color == "green") ledDisplay.breathAnimation(0, 255, 0);
        if (color == "blue") ledDisplay.breathAnimation(0, 0, 255);
        if (color == "purple") ledDisplay.breathAnimation(128, 0, 128);
      } 
      else if ((millis() - lastTime > period ) && (mode == "clock")) 
      {
        struct tm timeInfo = getLocalTime();

        if (autoBrightness) {
		      //авто яскравість
          int hour = timeInfo.tm_hour;
          bool isDay = hour >= day && hour < night;
		      brightness = isDay ? dayBrightness : nightBrightness;
          strip.setBrightness(brightness * 2.55);
          strip.show();
        }

        
        unsigned long  time_millis = millis();
        unsigned long hv = 180000;
        lastTime = time_millis;
        
        if (mode == "clock") 
        {
          tixManager.showtime(timeInfo);
          delay(60);                           //  Pause for a moment
        }
      }
    } else {
      strip.clear();
      strip.show();
      // success_message();
    }
  } else {
    strip.clear();
    strip.show();
    delay(10000);
    ESP.restart();
  }
}