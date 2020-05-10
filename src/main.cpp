/*
 * Time sync to NTP time source
 * git remote add origin https://github.com/alejandro70/nodemcu-clock.git
 */

#include <Arduino.h>
#include <DNSServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <TimeLib.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include <SimpleTimer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#include "global.h"
#include "display.h"
#include "ntp.h"

#define BTN_TRIGGER D2 // Activación de Access Point (AP) mode
#define ANALOG_PIN A0  // NodeMCU board ADC pin

// local variables
volatile int timer1Seconds = 0;
bool apModeStarted = false;

// timers
SimpleTimer timer;
int timerDisplayTime;
int timerMatrixBanner;
int timerLightSensor;

// functions
void configModeCallback(WiFiManager *);
void restart();
void ldrRange();

// ISR to Fire when Timer is triggered
void ICACHE_RAM_ATTR onTimer1()
{
  timer1Seconds++;
  if (timer1Seconds == 20)
  {
    timer1Seconds = 0;
    if (WiFi.status() != WL_CONNECTED && !apModeStarted)
    {
      ESP.restart();
    }
  }

  // Re-Arm the timer as using TIM_SINGLE
  timer1_write(312500); //1 s
}

void setup()
{
  Serial.begin(115200);

  pinMode(BTN_TRIGGER, INPUT_PULLUP);

  {
    //Initialize NodeMCU Timer1 every 1s
    timer1_attachInterrupt(onTimer1); // Add ISR Function
    timer1_enable(TIM_DIV256, TIM_EDGE, TIM_SINGLE);
    timer1_write(312500); // 312500 / 1 tick per 3.2 us from TIM_DIV256 == 1 s interval
  }

  // Max72xxPanel
  initMatrix();
  matrixRender("WiFi?", 31);

  // WiFiManager
  WiFiManager wifiManager;                       // Local intialization.
  wifiManager.setAPCallback(configModeCallback); // AP Configuration
  wifiManager.setBreakAfterConfig(true);         // Exit After Config Instead of connecting

  //Reset Settings - If Button Pressed
  if (digitalRead(BTN_TRIGGER) == LOW)
  {
    wifiManager.resetSettings();
    ESP.restart();
  }

  // Tries to connect to last known settings or else starts an access point.
  if (!wifiManager.autoConnect("NTP Clock"))
  {
    ESP.reset();
  }

  delay(3000);
  {
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");

    // Seed Random With vVlues Unique To This Device
    uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    uint32_t seed1 =
        (macAddr[5] << 24) | (macAddr[4] << 16) |
        (macAddr[3] << 8) | macAddr[2];
    randomSeed(seed1 + micros());
    localPort = random(1024, 65535);
    udp.begin(localPort);

    // NTP config
    setSyncProvider(getNtpTime);
    setSyncInterval(5 * 60);
  }

  // timers (init disabled)
  timerDisplayTime = timer.setInterval(1000L, displayTime);
  timer.disable(timerDisplayTime);
  timerMatrixBanner = timer.setInterval((long)bannerFrecuency, matrixBannerFrame);
  timer.disable(timerMatrixBanner);
  timer.setTimeout(86400000L, restart);
  timerLightSensor = timer.setInterval(10000L, ldrRange);

  // IP banner
  matrixBanner(5000L, String("IP:") + WiFi.localIP().toString().c_str());
}

void loop()
{
  timer.run();
}

// called when AP mode and config portal is started
void configModeCallback(WiFiManager *myWiFiManager)
{
  apModeStarted = true;
}

void restart()
{
  ESP.restart();
}

void ldrRange()
{
  int sensorValue = analogRead(ANALOG_PIN);

  // ajustar intensidad de display
  int intensity = map(sensorValue, 0, 1024, 0, 4);
  matrix.setIntensity(intensity);
}
