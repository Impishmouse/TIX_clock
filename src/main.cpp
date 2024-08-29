// =======================================

#include <ArduinoJson.h> //json для аналізу інформації
#include <Adafruit_NeoPixel.h> //neopixel для управління стрічкою
#include <WiFi.h> //для зв'язку
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <UniversalTelegramBot.h> //Telegram бот
#include <WiFiManager.h> //Керування WiFi
#include <NTPClient.h> //Час
#include <HTTPUpdate.h> //Оновлення прошивки через тг бота
#include <Wire.h> 
#include <AirAlarmDisplay.h>
#include "time.h"
#include <string> 

// ============ НАЛАШТУВАННЯ ============
char ssid[] = "StarLord";                  //"StarLord"; //Назва твоєї мережі WiFi
char password[] = "strongWifipwd";              //"strongWifipwd"; //Пароль від твого WiFi
char APSsid[] = "TixClock"; //Назва точки доступу
char APPassword[] = ""; //Пароль від точки доступу
int brightness = 70; //Яскравість %
bool autoBrightness = true; //Ввімкнена/вимкнена авто яскравість

#define BOTtoken "6048625903:AAEe-Sr6wOnMzcp-LaLQxU9FwALzpDqhpIU"
#define CHAT_ID "260761974"
int botRequestDelay = 1000;

const int day = 9; //Початок дня
const int night = 21; //Початок ночі
const int dayBrightness = 70; //Денна яскравість %
const int nightBrightness = 20; //Нічна яскравість %
const long  gmtOffset_sec = 7200;


#define LED_PIN 13
#define LED_COUNT 27


Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

DynamicJsonDocument doc(30000);

WiFiClientSecure client;
WiFiManager wm;
WiFiUDP ntpUDP;

const char* ntpServer = "ua.pool.ntp.org";

NTPClient timeClient(ntpUDP, ntpServer, 7200);
UniversalTelegramBot bot(BOTtoken, client);

String utf8cyr(String source);
void colorWipe(int wait);
int linearSearch(int *arr, int arr_len, int target);
void setLedGroupAndColor(int* legGroup, int legGroup_len, int* enableIndexes, int enabled_len, uint32_t ledColor);

unsigned long lastTimeBotRan;
unsigned long duration;
static unsigned long times[25];
String color;
static int ledColor[25];

int arrAlarms = sizeof(ledColor) / sizeof(int);

// int red, green, blue;
// bool startMessage = false;
bool enable = false;
int currencyMode = 1;
int period = 15000;

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

static int ledHour1[] = { 0, 1, 2};
static int ledHour2[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11};

static int ledMin1[] = { 12, 13, 14, 15, 16, 17};
static int ledMin2[] = { 18, 19, 20, 21, 22, 23, 24, 25, 26};

const int ARRAY_SIZE = 9; // Розмір масиву - максимальна кількисть світлодіодів на одну цифру.



void initWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // TODO  Add led breath while wifi connection
  
  //airAlarmDisplay.wifiConnect(ssid, 2);
  //delay(700);
  
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

// Відправка в телеграм повідомлення та клавіатури.
void success(String message) {
  String keyboardJson = "[[\"" + String(enabled ? "⏸💡" : "▶️💡") + "\"], [\"🔆 Змінити яскравість (" + String(autoBrightness ? "🤖": String(brightness) + "%") + ")\"], [\"🔧 Оновити прошивку\"], [\"🔄 Рестарт\"]]";
  //[\"🔢 Змінити режим (" + String(mode == "clock" ? "🕒" : mode == "flag" ? "🇺🇦" : mode == "flashlight" ? "🔦" + String(color == "white" ? "⚪" : color == "red" ? "🔴" : color == "orange" ? "🟠" : color == "yellow" ? "🟡" : color == "green" ? "🟢" : color == "blue" ? "🔵" : color == "purple" ? "🟣" : "❌")  : "❌") + ")\"],
  bot.sendMessageWithReplyKeyboard(CHAT_ID, message, "", keyboardJson, true);
}

// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    // Chat id of the requester
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Незареєстрований користувач");
      continue;
    }
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      success("Привіт, " + from_name + ".\nДля керування використовуй кнопки в меню бота");  
    }
    if (text == String(enabled ? "⏸💡" : "▶️💡")) {
      if (enabled) {
         enabled = false;
        success("⏸");
      } else { 
        enabled = true;
        success("▶️");
      }  
    }

    if (text == "🔆 Змінити яскравість (" + String(autoBrightness ? "🤖": String(brightness) + "%") + ")") {
      // bot.sendMessage(chat_id, "Введи значення у відстотках:\n*Щоб активувати авто-яскравість введи значення 0");
      String keyboardJson = "[[\"100%\", \"75%\", \"50%\", \"25%\", \"1%\"], [\"" + String(autoBrightness ? "" : "🤖 Активувати автояскравість") + "\"], [\"❌ Скасувати\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "🔆 Введи значення у відстотках:", "", keyboardJson, true);
      while (true) {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        if (numNewMessages > 0) {
          String text = bot.messages[0].text;
          if (text.toInt() >= 1 && text.toInt() <= 100) {
            autoBrightness = false;
            brightness = text.toInt();
            strip.setBrightness(brightness * 2.55);
            strip.show();
            success("✅");
            break;
          } else if (text == "❌ Скасувати") {
              success("✅");
              break;
            } else if (text == "🤖 Активувати автояскравість") {
              autoBrightness = true;
              success("🤖");
              break;
            } else {
              bot.sendMessage(chat_id, "🔆 Значення введено неправильно, введи відсоток від 1 до 100:");
              bot.sendMessage(chat_id, "🤷");
            }
        }
        delay(1000);
      }
    }
    
    if (text == "🔧 Оновити прошивку") {
      success("🔧 Щоб оновити прошивку відправ файл у форматі .bin");
    }
    if (text == "🔄 Рестарт") {
      success("🔄");
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      ESP.restart();
    }

	//	
    if (bot.messages[i].hasDocument) {
      httpUpdate.rebootOnUpdate(false);
      t_httpUpdate_return ret = (t_httpUpdate_return)3;
      bot.sendMessage(chat_id, "🔧 Завантаження прошивки...", "");
      ret = httpUpdate.update(client, bot.messages[i].file_path);
      switch (ret) {
        case HTTP_UPDATE_FAILED:
          bot.sendMessage(chat_id, "❌ " + String(httpUpdate.getLastError()) + ": " + httpUpdate.getLastErrorString(), "");
          // break;
        case HTTP_UPDATE_NO_UPDATES:
          bot.sendMessage(chat_id, "❌ Немає оновлень", "");
          // break;
        case HTTP_UPDATE_OK:
          bot.sendMessage(chat_id, "✅ Оновлення успішне", "");
          bot.sendMessage(chat_id, "🔄 Перезапуск...", "");
          numNewMessages = bot.getUpdates(bot.last_message_received + 1);
          ESP.restart();
      }    
    }
  }
}


int* getSeparateDigits (int input_digit)
{
  static int ret[2];

  if (input_digit >= 10) {
    ret[0] = input_digit / 10;   // Отримуємо десятки
    ret[1] = input_digit % 10;   // Отримуємо одиниці
  } else {
    // Якщо число менше 10, десятки залишаються нульовими
    ret[0] = 0;
    ret[1] = input_digit;
  }

  return ret;
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

void printLocalTime(struct tm timeinfo)
{
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  int* hour_a = getSeparateDigits(timeinfo.tm_hour);


  Serial.print("Hour1:");
  Serial.println(std::to_string(hour_a[0]).c_str());

  Serial.print("Hour2:");
  Serial.println(std::to_string(hour_a[1]).c_str());

  int* minutes_a = getSeparateDigits(timeinfo.tm_min);
  Serial.print("Minute1:");
  Serial.println(std::to_string(minutes_a[0]).c_str());

  Serial.print("Minute2:");
  Serial.println(std::to_string(minutes_a[1]).c_str());
}


// Генерація випадкових не повторювальних індексів масива. 
// Потмім використається щоб засвітити світлодіоди
// int led_Count - кількисть світлодіодів які треба засвітити - година або минута.
// int range_min, int range_max - відповідно кількисть світлодіодів яки можна використати. - відповідає секциї
int* getRandomLeds(int led_Count, int range_max)
{
  static int numbers[ARRAY_SIZE];  // Масив для зберігання випадкових чисел
  bool used[ARRAY_SIZE];    // Масив для перевірки використаних чисел

  // Ініціалізація масиву used
  for (int i = 0; i < ARRAY_SIZE; i++) {
    used[i] = false;
  }

  for (int i = 0; i < led_Count; i++) {
    int num;
    do {
      num = random(0, range_max + 1);  // Генерація випадкового числа
    } while (used[num]);  // Перевірка, чи число вже було використане

    numbers[i] = num;
    used[num] = true;
    //Serial.println(numbers[i]);  // Виведення числа в серійний монітор
  }
  return numbers;
}

void showTixHour(int* hours_a)
{
  setLedGroupAndColor(ledHour1, 3, getRandomLeds(hours_a[0], 2), hours_a[0], strip.Color(255, 0, 0));
  setLedGroupAndColor(ledHour2, 9, getRandomLeds(hours_a[1], 8), hours_a[1], strip.Color(0, 255, 0));
}

void showTixMinutes(int* mins_a)
{
  setLedGroupAndColor(ledMin1, 6, getRandomLeds(mins_a[0], 5), mins_a[0], strip.Color(0, 0, 255));
  setLedGroupAndColor(ledMin2, 9, getRandomLeds(mins_a[1], 8), mins_a[1], strip.Color(255, 0, 0));
}

/*
  Передамо в 
  legGroup - масив з індексами світлодіодів для секції (десятки годин, одениці годин, ...)
  legGroup_len - довжину цього масива.
  enableIndexes - випадково вибрані індекси масива які треба засвітити.
  enabled_len - довжина масива.
  ledColor - кольор якім треба засвітити.
 */
void setLedGroupAndColor (int* legGroup, int legGroup_len, int* enableIndexes, int enabled_len, uint32_t ledColor)
{
  for (int i = 0; i < legGroup_len; i++) { // For each pixel in strip...
    if (linearSearch(enableIndexes, enabled_len, i) >= 0)
    {
      strip.setPixelColor(legGroup[i], ledColor);
    } 
    else 
    {
      strip.setPixelColor(legGroup[i], strip.Color(0, 0, 0));  // вимикаємо світлодіод
    }
  }
}

int linearSearch(int *arr, int arr_len, int target) 
{
    for (int i = 0; i < arr_len; i++) 
    {
      if (arr[i] == target)
      {
        return target; // Повертає індекс знайденого елемента
      }
    }
    return -1;  // Повертає -1, якщо елемент не знайдено
}

void setup() 
{
  Serial.begin(115200);

  initStrip();

  initWiFi();

  initTime();
  
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  success("💡");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {

  wifiConnected = WiFi.status() == WL_CONNECTED;
  if (wifiConnected) {
    if (millis() > lastTimeBotRan + botRequestDelay) {
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

      while (numNewMessages) {
        Serial.println("got response");
        handleNewMessages(numNewMessages);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      }
      lastTimeBotRan = millis();
    }
    if (enabled) {
      if (millis() - lastTime > period ) {
        if (autoBrightness) {
		      //авто яскравість
		      timeClient.update();
		      int hour = timeClient.getHours();
		      bool isDay = hour >= day && hour < night;
		      brightness = isDay ? dayBrightness : nightBrightness;
          strip.setBrightness(brightness * 2.55);
          strip.show();
        }

        
        unsigned long  t = millis();
        unsigned long hv = 180000;
        lastTime = t;
        
        
        if (mode == "clock") 
        {
          struct tm timeInfo = getLocalTime();
          printLocalTime(timeInfo);
          
          int* hour_a = getSeparateDigits(timeInfo.tm_hour);
          showTixHour(hour_a);
          int* minutes_a = getSeparateDigits(timeInfo.tm_min);
          showTixMinutes(minutes_a);
          strip.show();  //  Update strip to match
          delay(60);                           //  Pause for a moment

        }

        /*if (mode == "flag") {
          int count = sizeof(ledColorYellow) / sizeof(int);
          for (int i = 0; i < count; i++) { // For each pixel in strip...
            strip.setPixelColor(ledColorBlue[i], strip.Color(0, 0, 255));
            strip.setPixelColor(ledColorYellow[i], strip.Color(255, 255, 0));//  Set pixel's color (in RAM)
            strip.show();
          }
        }
        if (mode == "flashlight") {
          for (int i = 0; i < LED_COUNT; i++) {
            if (color == "white") strip.setPixelColor(i, 255, 255, 255);
            if (color == "red") strip.setPixelColor(i, 255, 0, 0);
            if (color == "orange") strip.setPixelColor(i, 255, 165, 0);
            if (color == "yellow") strip.setPixelColor(i, 255, 255, 0);
            if (color == "green") strip.setPixelColor(i, 0, 255, 0);
            if (color == "blue") strip.setPixelColor(i, 0, 0, 255);
            if (color == "purple") strip.setPixelColor(i, 128, 0, 128);
          }
          strip.show();
        }*/
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