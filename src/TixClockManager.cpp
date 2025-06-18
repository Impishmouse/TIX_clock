#include "Arduino.h"
#include "TixClockManager.h"


static int ledHour1[] = { 0, 1, 2};
static int ledHour2[] = { 3, 4, 5, 6, 7, 8, 9, 10, 11};

static int ledMin1[] = { 12, 13, 14, 15, 16, 17};
static int ledMin2[] = { 18, 19, 20, 21, 22, 23, 24, 25, 26};

const int ARRAY_SIZE = 9; // Розмір масиву - максимальна кількисть світлодіодів на одну цифру.

TixClockManager::TixClockManager(Adafruit_NeoPixel* strip): _strip(strip) {}


void TixClockManager::showtime(struct tm timeInfo)
{
    int* hour_a = getSeparateDigits(timeInfo.tm_hour);
          
    showTixHour(hour_a);
    int* minutes_a = getSeparateDigits(timeInfo.tm_min);
    showTixMinutes(minutes_a);

    _strip->show();  //  Update strip to match

}


/* PRIVATE */

int* TixClockManager::getSeparateDigits (int input_digit)
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


// Генерація випадкових не повторювальних індексів масива. 
// Потмім використається щоб засвітити світлодіоди
// int led_Count - кількисть світлодіодів які треба засвітити - година або минута.
// int range_min, int range_max - відповідно кількисть світлодіодів яки можна використати. - відповідає секциї
int* TixClockManager::getRandomLeds(int led_Count, int range_max)
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


void TixClockManager::showTixHour(int* hours_a)
{
  setLedGroupAndColor(ledHour1, 3, getRandomLeds(hours_a[0], 2), hours_a[0], _strip->Color(255, 0, 0));
  setLedGroupAndColor(ledHour2, 9, getRandomLeds(hours_a[1], 8), hours_a[1], _strip->Color(0, 255, 0));
}

void TixClockManager::showTixMinutes(int* mins_a)
{
  setLedGroupAndColor(ledMin1, 6, getRandomLeds(mins_a[0], 5), mins_a[0], _strip->Color(0, 0, 255));
  setLedGroupAndColor(ledMin2, 9, getRandomLeds(mins_a[1], 8), mins_a[1], _strip->Color(255, 0, 0));
}


/*
  Передамо в 
  legGroup - масив з індексами світлодіодів для секції (десятки годин, одениці годин, ...)
  legGroup_len - довжину цього масива.
  enableIndexes - випадково вибрані індекси масива які треба засвітити.
  enabled_len - довжина масива.
  ledColor - кольор якім треба засвітити.
 */
void TixClockManager::setLedGroupAndColor (int* legGroup, int legGroup_len, int* enableIndexes, int enabled_len, uint32_t ledColor)
{
  for (int i = 0; i < legGroup_len; i++) { // For each pixel in strip...
    if (linearSearch(enableIndexes, enabled_len, i) >= 0)
    {
        _strip->setPixelColor(legGroup[i], ledColor);
    } 
    else 
    {
        _strip->setPixelColor(legGroup[i], _strip->Color(0, 0, 0));  // вимикаємо світлодіод
    }
  }
}


int TixClockManager::linearSearch(int *arr, int arr_len, int target) 
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