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

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


// ============ НАЛАШТУВАННЯ  BLUETOOTH ============

BLECharacteristic* pCharacteristic;
BLEServer* pServer;
bool deviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


std::string dayTime = "08:00";
std::string nightTime = "21:00";
uint8_t selectedColor[3] = {255, 255, 255}; // RGB

int currentMode = 1;
/*  Опис режимів роботи : 
  0 - вимкнено 
  1 - годинник
  2 - Дихання - підсвітка - зі зміною кольору.
  3 - Режин анімації прапора.
 */


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
int day = 9; //Початок дня
int night = 21; //Початок ночі


// ===== Налаштування світлодіодів ======
#define LED_PIN 13
#define LED_COUNT 27
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Led9x3Matrix ledDisplay(&strip);
TixClockManager tixManager(&strip);

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

// Bluetooth callbacks 

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    Serial.print("📨 Received: ");
    Serial.println(value.c_str());
  
    if (value.find("MODE:") == 0) {
      currentMode = atoi(value.substr(5).c_str());
      Serial.printf("➡️ Mode set to %d\n", currentMode);
  
    } else if (value.find("BRIGHT:") == 0) {
      brightness = atoi(value.substr(7).c_str());
      Serial.printf("💡 Brightness set to %d\n", brightness);
  
      strip.setBrightness(brightness);
      strip.show();

    } else if (value.find("AUTO:") == 0) {
      bool autoB = value.substr(5) == "1";
      autoBrightness = autoB;
      Serial.printf("🌓 AutoBrightness: %s\n", autoB ? "ON" : "OFF");
  
    } else if (value.find("DAY:") == 0) {
      dayTime = value.substr(4);  // expected format "HH:MM"
      day = atoi(dayTime.substr(0, 2).c_str());
      Serial.printf("🌞 Day time set to %s\n", dayTime.c_str());
  
    } else if (value.find("NIGHT:") == 0) {
      nightTime = value.substr(6);
      night = atoi(nightTime.substr(0, 2).c_str());
      Serial.printf("🌙 Night time set to %s\n", nightTime.c_str());
  
    } else if (value.find("COLOR:") == 0) {
      String hexColor = value.substr(6).c_str(); // "#RRGGBB"
      long colorValue = strtol(hexColor.c_str() + 1, nullptr, 16); // Skip '#'
      uint8_t r = (colorValue >> 16) & 0xFF;
      uint8_t g = (colorValue >> 8) & 0xFF;
      uint8_t b = colorValue & 0xFF;
      selectedColor[0] = r;
      selectedColor[1] = g;
      selectedColor[2] = b;
      Serial.printf("🎨 Color set to #%02X%02X%02X\n", r, g, b);
  
    } else if (value == "STATUS?") {
      String status = "MODE:" + String(currentMode) + "\n" +
                      "BRIGHT:" + String(brightness) + "\n" +
                      "AUTO:" + String(autoBrightness ? 1 : 0) + "\n" +
                      "DAY:" + String(dayTime.c_str()) + "\n" +
                      "NIGHT:" + String(nightTime.c_str()) + "\n" +
                      "COLOR:#" + String(selectedColor[0], HEX) + 
                                String(selectedColor[1], HEX) + 
                                String(selectedColor[2], HEX);
      pCharacteristic->setValue(status.c_str());
      pCharacteristic->notify();
      Serial.println("📤 Sent full STATUS");
      return;
    }
  
    // Після будь-якої зміни — можна надіслати підтвердження, якщо потрібно
  }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("⚠️ Device disconnected. Restarting advertising...");

    // ❗️ Оновлення: перезапустити рекламу
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->start();
  }
};


void setup() 
{
  Serial.begin(115200);

  Serial.println("BLE INIT...");
  BLEDevice::init("ESP32_BLE");
  Serial.println("BLE Device Initialized");

  pServer = BLEDevice::createServer();
  Serial.println("BLE Server Created");
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ |
    BLECharacteristic::PROPERTY_WRITE |
    BLECharacteristic::PROPERTY_NOTIFY
  );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
  pCharacteristic->setValue("ESP Ready");
  Serial.println("ESP Ready");
  pService->start();
  
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("✅ Advertising with service UUID");

  initStrip();

  initWiFi();

  configTime(UTC_OFFSET, UTC_OFFSET_DST, NTP_SERVER);
}

void loop() {
  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected) {
    if (currentMode != 0) {

      if (currentMode == 3) 
      {
        ledDisplay.praporAnimation();
      } 
      else if (currentMode == 2) 
      {
        ledDisplay.breathAnimation(selectedColor[0], selectedColor[1], selectedColor[2]);
      } 
      else if ((millis() - lastTime > period ) && (currentMode == 1)) 
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
        
        if (currentMode == 1) 
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