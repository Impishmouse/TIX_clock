#include "Arduino.h"
#include "Led9x3Matrix.h"


static int ledColorBlue[] =   { 1, 3, 8, 9, 10, 13, 15, 20, 21, 22, 25};
static int ledColorYellow[] = { 0, 4, 7, 6, 11, 12, 16, 17, 19, 23, 24};

int brightness_led = 25; //Яскравість %


#define WIDTH 9
#define HEIGHT 3

// Мапа [y][x] → індекс світлодіода
const int pixelMap[3][9] = {
  { 2,  3,  8,  9, 14, 15, 20, 21, 26 }, // y=0
  { 1,  4,  7, 10, 13, 16, 19, 22, 25 }, // y=1
  { 0,  5,  6, 11, 12, 17, 18, 23, 24 }  // y=2
};

// Wi-Fi іконка (3 рівні + центральна крапка)
const bool wifiIcon[3][5] = {
  { 0, 0, 0, 0, 0 },
  { 1, 0, 0, 0, 1 },
  { 1, 1, 0, 1, 1 }
};


const bool AccPointIcon[3][7] = {
    { 0, 1, 0, 0, 1, 1, 1 },
    { 1, 0, 1, 0, 1, 1, 1 },
    { 1, 0, 1, 0, 1, 0, 0 }
  };


// Піксельні шаблони символів '1', '2', '3', 'X' (3×3)
const uint8_t symbol_1[3] = { B010, B110, B010 };
const uint8_t symbol_2[3] = { B111, B011, B110 };
const uint8_t symbol_3[3] = { B111, B011, B111 };
const uint8_t symbol_X[3] = { B101, B010, B101 };



// === Налаштування дихання ===
const float centerX = 4.0;        // центр по X (0..8)
const float centerY = 2.0;        // центр по Y (0..2)
const float maxRadius = 4.0;      // найбільший радіус дихання

const uint8_t maxBrightness = 127;
const uint8_t minBrightness = 0;

const float breathSpeed = 0.035;   // швидкість дихання

float t = 0;
// === EOF Налаштування дихання ===

// === Налаштування прапору ===
// Базові кольори
uint8_t blueR = 0, blueG = 120, blueB = 255;
uint8_t yellowR = 255, yellowG = 180, yellowB = 0;


Led9x3Matrix::Led9x3Matrix(Adafruit_NeoPixel* strip): _strip(strip) {}



void Led9x3Matrix::praporAnimation()
{
    _strip->clear();

    for (int x = 0; x < WIDTH; x++) {
      // Положення хвилі у цій колонці
      float wave = sinf(x * 0.7 + t);  // від -1 до +1
  
      for (int y = 0; y < HEIGHT; y++) {
        int pixel = pixelMap[y][x];
  
        // Колір за замовчуванням
        uint8_t baseR, baseG, baseB;
  
        if (y == 0) {
          // Верхній ряд завжди синій
          baseR = blueR; baseG = blueG; baseB = blueB;
        } else if (y == 2) {
          // Нижній ряд завжди жовтий
          baseR = yellowR; baseG = yellowG; baseB = yellowB;
        } else {
          // Середній ряд: змінюється залежно від фази хвилі
          if (wave < -0.3) {
            baseR = blueR; baseG = blueG; baseB = blueB;
          } else {
            baseR = yellowR; baseG = yellowG; baseB = yellowB;
          }
        }
  
        // Плавна анімація яскравості хвилі
        float offsetY = (float)(y - 1);  // центровано: -1, 0, +1
        float influence = cosf((wave - offsetY) * 1.8);  // хвиля з центру
        influence = (influence + 1.0) / 2.0;
  
        float brightness = constrain(influence * (1.0 - fabs(offsetY) * 0.4), 0.0, 1.0);
  
        uint8_t r = baseR * brightness;
        uint8_t g = baseG * brightness;
        uint8_t b = baseB * brightness;
  
        _strip->setPixelColor(pixel, _strip->Color(r, g, b));
      }
    }
  
    _strip->show();
    delay(30);
    t += 0.15;
}

/*
  Анімація дихання запускати в loop 
  red - red 0 - 255
  green - green 0 - 255
  blue - blue 0 - 255

 */
void Led9x3Matrix::breathAnimation(uint red, uint green, uint blue)
{
    _strip->clear();

    // Радіус на поточному кадрі
    float r = 0.5 + sin(t) * (maxRadius - 0.5);  // хвиля від 0.5 до maxRadius
  
    for (int y = 0; y < HEIGHT; y++) {
      for (int x = 0; x < WIDTH; x++) {
        int pixelIndex = pixelMap[y][x];
  
        // Відстань від центру
        float dx = x - centerX;
        float dy = y - centerY;
        float dist = sqrt(dx * dx + dy * dy);
  
        // Розрахунок яскравості
        float fade = 1.0 - fabs(dist - r) / maxRadius;  // ближче до r — яскравіше
        fade = constrain(fade, 0.0, 1.0);
        uint8_t brightness = minBrightness + (maxBrightness - minBrightness) * fade;
  
        uint8_t rVal = (red * brightness) / 255;
        uint8_t gVal = (green * brightness) / 255;
        uint8_t bVal = (blue * brightness) / 255;
  
        _strip->setPixelColor(pixelIndex, _strip->Color(rVal, gVal, bVal));
      }
    }
  
    _strip->show();
    delay(30);
    t += breathSpeed;
}


// Засвітка світлодіодів у вигляді прапора
void Led9x3Matrix::displayStaticPrapor(int wait) 
{
    int count = sizeof(ledColorYellow) / sizeof(int);
    for (int i = 0; i < count; i++) { // For each pixel in strip...
        _strip->setPixelColor(ledColorBlue[i], _strip->Color(0, 0, 255));
        _strip->setPixelColor(ledColorYellow[i], _strip->Color(255, 255, 0));//  Set pixel's color (in RAM)
        _strip->show();                          //  Update strip to match
        delay(wait);                           //  Pause for a moment
    }
}


void Led9x3Matrix::wifiIconDraw(char statusChar)
{
    _strip->clear();

    uint8_t colorR = 0;     // WiFi колір
    uint8_t colorG = 150;
    uint8_t colorB = 255;

    // Вибираємо шаблон символа
    const uint8_t* symbol;
    switch (statusChar) {
    case '1': symbol = symbol_1; break;
    case '2': symbol = symbol_2; break;
    case '3': symbol = symbol_3; break;
    default:  
        symbol = symbol_X;
        colorR = 255;
        colorG = 10;
        colorB = 10;
        break;
    }

    // Малюємо Wi-Fi іконку (координати x=0..4, y=0..2)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 5; x++) {
            if (wifiIcon[y][x]  == 0) {
                int idx = pixelMap[y][x];
                _strip->setPixelColor(idx, _strip->Color(colorR, colorG, colorB));
            }
        }
    }

    // Малюємо символ (xOffset = 6..8)
    int xOffset = 6;
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 3; x++) {
            if (bitRead(symbol[y], 2 - x)) {
                int index = pixelMap[y][xOffset + x];
                _strip->setPixelColor(index, _strip->Color(colorR, colorG, colorB)); // символ 
            }
        }
    }

    _strip->show();                          //  Update strip to match
    delay(20);
}


void Led9x3Matrix::begin()
{
    _strip->begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
    _strip->show();            // Turn OFF all pixels ASAP
    _strip->setBrightness(brightness_led * 2.55);
    //displayStaticPrapor(10);
    wifiIconDraw('x');
}

void Led9x3Matrix::wifiConnecting(int index)
{
    if (index >= 1 && index <= 3) {
        wifiIconDraw('0' + index);
    } else {
        wifiIconDraw('x'); // інше значення — помилка або невідомий статус
    }
}

void Led9x3Matrix::wifiConnectSuccess()
{
    _strip->clear();
    int xOffset = 2;
    // Малюємо Wi-Fi іконку (координати x=0..4, y=0..2)
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 5; x++) {
            if (wifiIcon[y][x]  == 0) {
                int idx = pixelMap[y][x + xOffset];
                _strip->setPixelColor(idx, _strip->Color(0, 255, 0));
            }
        }
    }

    _strip->show();                          //  Update strip to match
    delay(20);
}

void Led9x3Matrix::wifiConnectProblem()
{
    wifiIconDraw('x');
}

void Led9x3Matrix::wifiAccessPoint()
{
    _strip->clear();
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < 7; x++) {
            if (AccPointIcon[y][x]) {
                int idx = pixelMap[y][x];
                _strip->setPixelColor(idx, _strip->Color(255, 255, 255));
            }
        }
    }
    _strip->show();                          //  Update strip to match
    delay(20);
}

