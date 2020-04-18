#include <Arduino.h>
#include <TimeLib.h>

#include "display.h"
#include "global.h"

// Max72xxPanel
// DIN to MOSI and CLK to SCK (cf http://arduino.cc/en/Reference/SPI )
// --------------------------------------------------------------------------------
int pinCS = 2;                      // Attach CS to this pin
int numberOfHorizontalDisplays = 4; // num led's horizontales
int numberOfVerticalDisplays = 1;   // num led's verticales
int bannerFrecuency = 20;           // In milliseconds
String bannerText = "MAX7219";      // string a mostrar en banner
int matrixFrame = 0;                // banner frame index
int spacer = 1;                     // espaciador entre caracteres
int width = 5 + spacer;             // The font width is 5 pixels
bool evenSecs = false;              // show seconds leds? (every 1 sec)
char sTime[] = "00:00:00";          // buffer para display de hora
time_t prevDisplay = 0;             // when the digital clock was displayed

// objeto display
Max72xxPanel matrix = Max72xxPanel(
    pinCS,
    numberOfHorizontalDisplays,
    numberOfVerticalDisplays);

// Max72xxPanel
// ================================================================================

// --------------------------------------------------------------------------------
void initMatrix()
{
    matrix.setIntensity(2); // Use a value between 0 and 15 for brightness

    matrix.setPosition(0, 0, 0); // The first display is at <0, 0>
    matrix.setPosition(1, 1, 0); // The second display is at <1, 0>
    matrix.setPosition(2, 2, 0); // The third display is at <2, 0>
    matrix.setPosition(3, 3, 0); // And the last display is at <3, 0>

    matrix.setRotation(0, 1); // Display is position upside down
    matrix.setRotation(1, 1); // Display is position upside down
    matrix.setRotation(2, 1); // Display is position upside down
    matrix.setRotation(3, 1); // Display is position upside down
}

// Inicia display de hora
// --------------------------------------------------------------------------------
void beginDisplayTime()
{
    timer.disable(timerMatrixBanner);
    timer.enable(timerDisplayTime);
}

// Muestra texto en display matriz en i-esima columna de leds
// --------------------------------------------------------------------------------
void matrixRender(String text, int i)
{
    matrix.fillScreen(LOW);

    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while (x + width - spacer >= 0 && letter >= 0)
    {
        if (letter < (int)text.length())
        {
            matrix.drawChar(x, y, text[letter], HIGH, LOW, 1);
        }

        letter--;
        x -= width;
    }

    matrix.write(); // Send bitmap to display
}

void matrixBannerFrame()
{
    matrixRender(bannerText, matrixFrame);
    matrixFrame++;
    if (matrixFrame >= width * (int)bannerText.length() + matrix.width() - 1 - spacer)
    {
        matrixFrame = 0;
    }
}

// Meustra texto con rotación hacia la izquiera
// --------------------------------------------------------------------------------
void matrixBanner(unsigned long duration, String text)
{
    bannerText = text;
    matrixFrame = 0;
    timer.disable(timerDisplayTime);
    timer.enable(timerMatrixBanner);
    timer.setTimeout(duration, beginDisplayTime);
}

// show current time
// --------------------------------------------------------------------------------
void displayTime()
{
    if (timeStatus() == timeNotSet)
    {
        matrixRender("ERROR", 27);
        return;
    }

    int hh = hour();
    hh = hh <= 12 ? hh : hh - 12;
    hh = hh != 0 ? hh : 12;
    int mm = minute();
    sprintf(sTime, "%d%s%02d", hh, evenSecs ? ":" : " ", mm);
    int leftPixel = hh < 10 ? 27 : 31;

    matrixRender(sTime, leftPixel);

    evenSecs = !evenSecs;
}

void digitalClockDisplay()
{
    // digital clock display of the time
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.print(" ");
    Serial.print(day());
    Serial.print(".");
    Serial.print(month());
    Serial.print(".");
    Serial.print(year());
    Serial.println();
}

void printDigits(int digits)
{
    // utility for digital clock display: prints preceding colon and leading 0
    Serial.print(":");
    if (digits < 10)
        Serial.print('0');
    Serial.print(digits);
}

int setIntensity(String queryString)
{
    int value = queryString.toInt();
    if (value <= 12)
    {
        //config.saveMatrixIntensity(value);
        matrix.setIntensity(value);
    }

    return 0;
}

String timeToString(unsigned long t)
{
    return String(hour(t)) + ":" + minute(t) + ":" + second(t) + " " + day(t) + "/" + month(t) + "/" + year(t);
}
