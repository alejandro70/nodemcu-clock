#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h> // https://github.com/markruys/arduino-Max72xxPanel

// Max72xxPanel
// DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
// --------------------------------------------------------------------------------
extern int pinCS2;                     // Attach CS to this pin
extern int numberOfHorizontalDisplays; // num led's horizontales
extern int numberOfVerticalDisplays;   // num led's verticales
extern int bannerFrecuency;            // In milliseconds
extern String bannerText;              // string a mostrar en banner
extern int matrixFrame;                // banner frame index
extern int spacer;                     // espaciador entre caracteres
extern int width;                      // The font width is 5 pixels
extern bool evenSecs;                  // show seconds leds? (every 1 sec)
extern char sTime[];                   // buffer para display de hora
extern Max72xxPanel matrix;            // objeto display
extern time_t prevDisplay;             // when the digital clock was displayed

//
void initMatrix();
void beginDisplayTime();
void matrixRender(String text, int i);
void matrixBannerFrame();
void matrixBanner(unsigned long duration, String text);
void displayTime();
void digitalClockDisplay();
void printDigits(int digits);
String timeToString(unsigned long t);
int setIntensity(String command);
