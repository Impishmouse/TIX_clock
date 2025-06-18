/*

Робочий приклад  TIX CLOCK version 1.1.0

*/

// =======================================

#include <ArduinoJson.h> //json для аналізу інформації
#include <Adafruit_NeoPixel.h> //neopixel для управління стрічкою
#include <Led9x3Matrix.h> // Для показу еффектів.
#include <WiFi.h> //для зв'язку
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WiFiManager.h> //Керування WiFi
#include <NTPClient.h> //Час
#include <HTTPUpdate.h> //Оновлення прошивки через тг бота
#include <Wire.h> 
#include "time.h"
#include <string> 
#include <TixClockManager.h>

// ============ НАЛАШТУВАННЯ ============
char ssid[] = "StarLord_02";                  //"StarLord"; //Назва твоєї мережі WiFi
char password[] = "strongWifiPwd";              //"strongWifipwd"; //Пароль від твого WiFi

char APSsid[] = "TixClock"; //Назва точки доступу
char APPassword[] = ""; //Пароль від точки доступу


int brightness = 2; //Яскравість %
bool autoBrightness = true; //Ввімкнена/вимкнена авто яскравість
const int dayBrightness = 10; //Денна яскравість %
const int nightBrightness = 1; //Нічна яскравість %

const int day = 9; //Початок дня
const int night = 21; //Початок ночі

const long  gmtOffset_sec = 7200;


#define LED_PIN 13
#define LED_COUNT 27


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Led9x3Matrix ledDisplay(&strip);
TixClockManager tixManager(&strip);

DynamicJsonDocument doc(30000);

WiFiClientSecure client;
WiFiManager wm;
WiFiUDP ntpUDP;

const char* ntpServer = "ua.pool.ntp.org";

NTPClient timeClient(ntpUDP, ntpServer, 7200);


void colorWipe(int wait);



String color;
static int ledColor[25];

int arrAlarms = sizeof(ledColor) / sizeof(int);

// int red, green, blue;
// bool startMessage = false;
bool enable = false;
int currencyMode = 1;
int period = 8000;

const long dateInterval = 200000;
unsigned long lastTime, previousMillisA = 0, previousMillisB = 0, previousMillisC = 0, previousMillisD = 0, previousMillisE = 0, previousMillisG = 0, previousMillisH = 0, lastWeatherTime = 0, startTime = 0, previousMillisAutoSwitchDisplay = 0;


static bool wifiConnected;

String mode = "clock"; //Режим

bool enabled = true;
int disy = 0;
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

void initTime() {
  // Встановлюємо початкове значення літнього часу на false
  bool isDaylightSaving = false;
  // Отримуємо поточну дату та час з сервера NTP
  timeClient.begin();
  timeClient.update();
  String formattedTime = timeClient.getFormattedTime();
  // Розбиваємо рядок з форматованим часом на складові
  int day, month, year, hour, minute, second;
  sscanf(formattedTime.c_str(), "%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
  // Перевіряємо, чи поточний місяць знаходиться в інтервалі березень-жовтень
  if (month >= 4 && month <= 10) {
    // Якщо так, встановлюємо літній час на true
    isDaylightSaving = true;
  }
  // Встановлюємо зміщення часового поясу для врахування літнього часу
  if (isDaylightSaving) {
    timeClient.setTimeOffset(14400); // UTC+3 для України
  }
  else {
    timeClient.setTimeOffset(10800); // UTC+2 для України
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

  initTime();

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
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
        if (autoBrightness) {
		      //авто яскравість
		      timeClient.update();
		      int hour = timeClient.getHours();
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
          struct tm timeInfo = getLocalTime();

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