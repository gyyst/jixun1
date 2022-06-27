#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

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

int buttonState[4];
int menu;

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

void setup()
{
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(500); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  display.drawLine(0, 0, 0, 128, SSD1306_WHITE);
  display.display();
}

void loop()
{
  int key = scanner();
  
}
