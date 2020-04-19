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

#define BTN_TRIGGER D0 // Activación de Access Point (AP) mode
#define ANALOG_PIN A0  // NodeMCU board ADC pin

const char ssid[] = "Tech_D0003407"; //  your network SSID (name)
const char pass[] = "Apolo@1969";    // your network password

// AppSettings
//AppSettings config(SPIFFS);

const int numReadings = 50;
int readIndex = 0;   // the index of the current reading
int analogTotal = 0; // the running total
int lastAnalog = -1; // ultimo valor intensidad

// TSL2561 - Adafruit Flora Light Sensor
// Connect SCL-D1; SDA-D2 (12345 default ID)
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
bool tslEnabled;
float luxMin;
float luxMax;

// timers
SimpleTimer timer;
int timerDisplayTime;
int timerMatrixBanner;
int timerLightSensor;

// functions
void configModeCallback(WiFiManager *);
void restart();
void luxRange();

void setup()
{
  Serial.begin(115200);

  pinMode(BTN_TRIGGER, INPUT_PULLUP);

  // Configures the gain and integration time for the TSL2561
  tsl.enableAutoRange(true);                            // Auto-gain ... switches automatically between 1x and 16x
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); // fast but low resolution
  tslEnabled = tsl.begin();
  sensor_t sensor;
  tsl.getSensor(&sensor);
  luxMin = sensor.min_value;
  luxMax = sensor.max_value;

  // Max72xxPanel
  initMatrix();
  matrixRender("booting", 31);

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS); // fast but low resolution
  luxRange();                                           // medir iluminación

// WiFiManager
  WiFiManager wifiManager;                       // Local intialization.
  wifiManager.setAPCallback(configModeCallback); // AP Configuration
  wifiManager.setBreakAfterConfig(true);         // Exit After Config Instead of connecting

  //Reset Settings - If Button Pressed
  if (digitalRead(BTN_TRIGGER) == LOW)
  {
    //Display <Reset>
    matrixRender("reset", 31);
    delay(5000);

    wifiManager.resetSettings();
    //ESP.reset();
    ESP.restart();
  }

  // Tries to connect to last known settings or else starts an access point.
  if (!wifiManager.autoConnect("NTP Clock"))
  {
    delay(3000);
    ESP.reset();
    delay(5000);
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

  // timers
  timerDisplayTime = timer.setInterval(1000L, displayTime);
  timer.disable(timerDisplayTime);
  timerMatrixBanner = timer.setInterval((long)bannerFrecuency, matrixBannerFrame);
  timer.disable(timerMatrixBanner);
  timer.setTimeout(86400000L, restart);
  if (tslEnabled)
  {
    timerLightSensor = timer.setInterval(60000L, luxRange);
  }

  // IP banner
  matrixBanner(7000L, String("IP:") + WiFi.localIP().toString().c_str());
}

void loop()
{
  timer.run();

  if (timeStatus() == timeNotSet)
  {
    now();
    delay(3000);
    ESP.restart();
  }

  if (digitalRead(BTN_TRIGGER) == LOW)
  {
    ESP.restart();
  }
}

// calle when AP mode and config portal is started
void configModeCallback(WiFiManager *myWiFiManager)
{
}

// Mide el rango de iluminación
void luxRange()
{
  if (!tslEnabled)
  {
    return;
  }

  // Get a new sensor event
  sensors_event_t event;
  tsl.getEvent(&event);

  // Display the results (light is measured in lux)
  int lux = (int)event.light;
  lux = lux < luxMax ? lux : luxMax;

  // ajustar intensidad de display
  int intensity = map(lux, luxMin, luxMax, 0, 15);
  matrix.setIntensity(intensity);

  Serial.printf("%d lux -> %d intensity\n", lux, intensity);
}

void restart()
{
  ESP.restart();
}
