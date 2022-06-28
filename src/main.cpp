#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <OneWire.h>
#include <L298N.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI 9
#define OLED_CLK 10
#define OLED_DC 11
#define OLED_CS 12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

/* Comment out above, uncomment this block to use hardware SPI
#define OLED_DC     6
#define OLED_CS     7
#define OLED_RESET  8
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  &SPI, OLED_DC, OLED_RESET, OLED_CS);
*/

#define NUMFLAKES 10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
static const unsigned char PROGMEM logo_bmp[] =
    {0b00000000, 0b11000000,
     0b00000001, 0b11000000,
     0b00000001, 0b11000000,
     0b00000011, 0b11100000,
     0b11110011, 0b11100000,
     0b11111110, 0b11111000,
     0b01111110, 0b11111111,
     0b00110011, 0b10011111,
     0b00011111, 0b11111100,
     0b00001101, 0b01110000,
     0b00011011, 0b10100000,
     0b00111111, 0b11100000,
     0b00111111, 0b11110000,
     0b01111100, 0b11110000,
     0b01110000, 0b01110000,
     0b00000000, 0b00110000};
const unsigned int IN1 = 7;
const unsigned int IN2 = 8;
const unsigned int EN = 5;
int buttonState[4];
int menu;
OneWire ds(6); // on pin 6 (a 4.7K resistor is necessary)
byte i;
byte present = 0;
byte type_s = 0;
byte data[12];
byte addr[8];
float celsius;
bool flag = false;
int page;
int setFanSpeed;
int buf[128];
int temnum = 128;
int temcount = 0;
int setTemp = 45;
L298N motor(EN, IN1, IN2);
int setTime;
int setE;
int SetFanSpeed = 133;
long usedTime;
int maxE;
int lastPage;

unsigned long eTime;
int eCount;

unsigned long timebegin;
unsigned long time;

unsigned long lastTick;
int eFlag;

int scanner(void)
{
  if (buttonState[0] != digitalRead(A2))
  {
    delay(5);
    if (buttonState[0] != digitalRead(A2))
    {
      buttonState[0] = digitalRead(A2);
      if (buttonState[0] == LOW)
        return 1;
    }
  }
  if (buttonState[1] != digitalRead(A3))
  {
    delay(5);
    if (buttonState[1] != digitalRead(A3))
    {
      buttonState[1] = digitalRead(A3);
      if (buttonState[1] == LOW)
        return 2;
    }
  }
  if (buttonState[2] != digitalRead(A4))
  {
    delay(5);
    if (buttonState[2] != digitalRead(A4))
    {
      buttonState[2] = digitalRead(A4);
      if (buttonState[2] == LOW)
        return 3;
    }
  }
  if (buttonState[3] != digitalRead(A5))
  {
    delay(5);
    if (buttonState[3] != digitalRead(A5))
    {
      buttonState[3] = digitalRead(A5);
      if (buttonState[3] == LOW)
        return 4;
    }
  }
  return 0;
}
void printNowFanSpeed(int speed)
{
  if (page == 1)
  {
    if (menu == 2)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 8);
    display.print(F("now fan speed:"));
    display.print(speed);
    display.print(F("%"));
  }
  else
    return;
}
void printNowTemprature(float Temperature)
{
  if (page == 1)
  {
    if (menu == 3)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 16);
    display.print(F("now temperature:"));
    display.print(Temperature);
  }
  else
    return;
}
void printSetFanSpeed()
{
  if (page == 1)
  {
    if (menu == 4)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 24);
    display.print(F("set fan speed:"));
    display.print(setFanSpeed);
    display.print(F("%"));
    motor.setSpeed((int)(setFanSpeed / 100.0 * 123 + 132));
  }
  else
    return;
}
void printNowFanCurrent(float Current)
{
  if (page == 1)
  {
    if (menu == 5)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 32);
    display.print(F("now fan current:"));
    display.print(Current);
  }
  else
    return;
}
float getTemp()
{
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);
  for (i = 0; i < 9; i++)
  { // we need 9 bytes
    data[i] = ds.read();
    // Serial.print(data[i], HEX);
    // Serial.print(" ");
  }

  int16_t raw = (data[1] << 8) | data[0];
  if (type_s)
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10)
    {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else
  {
    byte cfg = 0x60; //设置温度传感器的采样精度，0x20 10个bits位.
    // at lower res, the low bits are undefined, so let's zero them

    if (cfg == 0x00)
      raw = raw & ~7; // 9 bit resolution, 93.75 ms

    else if (cfg == 0x20)
      raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40)
      raw = raw & ~1; // 11 bit res, 375 ms
    ////    default is 12 bit resolution, 750 ms conversion time
  }
  return (float)raw / 16.0;
}
void printNowModel()
{
  if (page != lastPage)
  {
    display.clearDisplay();
  }

  if (page == 1)
  {
    if (menu == 1)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 0);
    display.print(F("model:"));
    display.print(F("fan control"));
    lastPage = 1;
  }
  else if (page == 2)
  {
    if (menu == 1)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 0);
    display.print(F("model:"));
    display.print(F("temp control"));
    lastPage = 2;
  }
  return;
}

void printTemperatureCurve(float temperature)
{
  if (page == 2)
  {
    buf[temcount] = 68 - (int)((temperature / 75.0) * 24);
    int liney = 68 - (int)((setTemp / 75.0) * 24);
    // for (int i = 1; i < 127; i += 2)
    //   display.drawPixel(i, liney, SSD1306_WHITE);

    if (temcount < temnum - 1)
      temcount++;
    else
    {
      temcount = 0;
      display.fillRect(1, 48, 127, 14, SSD1306_BLACK);
    }

    display.drawLine(0, 48, 0, 63, SSD1306_WHITE);
    display.drawLine(0, 63, 127, 63, SSD1306_WHITE);
    for (int i = 0; i < temcount; i++)
    {
      display.drawPixel(i, buf[i], SSD1306_WHITE);
    }
  }
  else
    return;
}
int FanBuf[128];
int fancount;
void printFanSpeedCurve(int speed)
{
  if (page == 1)
  {
    FanBuf[fancount] = 63 - (int)((speed / 255) * 133);

    if (fancount < temnum - 1)
      fancount++;
    else
    {
      fancount = 0;
      display.fillRect(1, 40, 127, 22, SSD1306_BLACK);
    }

    display.drawLine(0, 40, 0, 63, SSD1306_WHITE);
    display.drawLine(0, 63, 127, 63, SSD1306_WHITE);
    for (int i = 0; i < fancount; i++)
    {
      display.drawPixel(i, FanBuf[i], SSD1306_WHITE);
    }
  }
  else
    return;
}
void printTemp(float temperature)
{
  if (page == 2)
  {
    if (menu == 0)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

    display.setCursor(0, 8); // Start at top-left corner
    // display.fillRect(30, 0, 96, 7, SSD1306_BLACK);
    display.display();
    display.print("Temp: ");
    display.print(temperature);
    display.print(" / ");
    display.print(setTemp);
    display.print(" C   ");
  }
}

void printTime()
{
  if (page == 2)
  {
    if (menu == 1)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

    time = millis() - timebegin;
    display.setCursor(0, 16);
    display.print(F("Time:"));
    // display.fillRect(30, 8, 96, 8, SSD1306_BLACK);
    display.print((int)(time / 1000.0));
    display.print(F(" / "));
    display.print(setTime);
    display.print(F(" s   "));
  }
}
void printE()
{
  if (page == 2)
  {
    if (menu == 2)
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    else
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);

    // display.fillRect(12, 16, 96, 8, SSD1306_BLACK);

    display.setCursor(0, 24);
    display.print(F("E:"));

    display.print(setE);
    display.print(F(" C   "));
  }
}

void printEffectiveTime()
{
  if (page == 2)
  {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 32);
    display.print(F("E.T/M.E:"));
    display.print(eTime / 1000.0);
    display.print(F("s "));
    display.print(maxE);
    display.print(F(" C "));
  }
}

void printcount()
{
  if (page == 2)
  {
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setCursor(0, 40);
    display.print(F("eCnt/U.T:"));
    display.print(eCount);

    display.print(F(" "));
    display.print(usedTime / 1000.0);
    display.print(F("s "));
  }
}
void reset()
{
  timebegin = millis();
  eTime = 0;
  eCount = 0;
  usedTime = -1;
  maxE = 0;
}

void setup()
{
  Serial.begin(9600);
  pinMode(A0, OUTPUT);
  pinMode(A2, INPUT_PULLUP);
  pinMode(A3, INPUT_PULLUP);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  if (!ds.search(addr) && !flag)
  {
    // Serial.println("No more addresses.");
    Serial.println();
    ds.reset_search();
    delay(250);
  }
  page = 2;
  display.setTextSize(1); // Normal 1:1 pixel scale
  setTemp = 30;
  setTime = 0;
  setE = 10;
  lastTick = millis();
  eFlag = 0;
}

void loop()
{
  int key = scanner();
  switch (key)
  {
  case 1:
    if (page == 2)
    {
      menu--;
      if (menu < 0)
        menu = 3;
      break;
    }
    else if (page == 1)
    {
      menu--;
      if (menu < 0)
        menu = 1;
      break;
    }
  case 2:
    if (page == 2)
    {
      menu++;
      if (menu > 3)
        menu = 0;
      break;
    }
    else if (page == 1)
    {
      menu++;
      if (menu > 1)
        menu = 0;
    }
  case 3:
    if (page == 2)
    {
      if (menu == 1)
        if (setTemp > 50)
        {
          setTemp--;
          display.fillRect(1, 48, 127, 14, SSD1306_BLACK);
        }
      if (menu == 2)
      {
        if (setTime > 0)
          setTime -= 10;
        reset();
      }
      if (menu == 3)
        if (setE > 0)
          setE--;
      break;
    }
    else if (page == 1)
    {
      if (menu == 0)
      {
        page--;
        if (page < 1)
          page = 2;
        break;
      }
      else if (menu == 1)
      {
        if (setFanSpeed > 0)
          setFanSpeed -= 1;
      }
    }
  case 4:
    if (page == 2)
    {
      if (menu == 1)
        if (setTemp < 75)
        {
          setTemp++;
          display.fillRect(1, 48, 127, 14, SSD1306_BLACK);
        }
      if (menu == 2)
      {
        if (setTime < 1000)
          setTime += 10;
        reset();
      }
      if (menu == 3)
        if (setE < 20)
          setE++;
      break;
    }
    else if (page == 1)
    {
      if (menu == 0)
      {
        page++;
        if (page > 2)
          page = 1;
        break;
      }
      else if (menu == 1)
      {
        if (setFanSpeed < 100)
          setFanSpeed += 1;
      }
    }
  }
  celsius = getTemp();
  Serial.println(celsius);
  printNowModel();
  printNowFanSpeed(motor.getSpeed());
  printNowTemprature(celsius);
  printSetFanSpeed();
  printNowFanCurrent(2);
  printTemperatureCurve(celsius);
  printTemp(celsius);
  printTime();
  printE();
  printEffectiveTime();
  printcount();
  display.display();
  delay(500);
}
