

#include <Arduino.h>
#include "Board.h"
#include "config.h"

const char* ssid = "MischisIndanetz";
const char* password = "123456789";

Adafruit_NeoPixel neoPixels = Adafruit_NeoPixel(NUMPIXELS, PINPIXELDATA, NEO_GRB + NEO_KHZ800);
ESP32Encoder encoder;

//CapacitiveSensor userBtn0CapSens = CapacitiveSensor(9, 10); 


Board::Board(){

}
  
void Board::startOTA()
{
 
  setCpuFrequencyMhz(240);
  WiFi.disconnect(false);  // Reconnect the network
  WiFi.mode(WIFI_STA);    // Switch WiFi off

  delay(1000);

  otaActive = true;
  // Show status Blue
  rgb rgbBuffer[NUMPIXELS];
  for (int n=0; n < NUMPIXELS; n++)
  {
      rgb b = Tools::RGB(0.0f, 0.0f, 0.1f);
      rgbBuffer[n] = b; 
  }
  updatePixels(rgbBuffer);

 // Setup OTA Server
  Serial.println("Starting OTA Server");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }


  // ArduinoOTA.setPort(3232);
  ArduinoOTA.setHostname("shotroulette");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);

      
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([=](unsigned int progress, unsigned int total) {

      // Show Progress on Leds
      rgb rgbBuffer[NUMPIXELS] = {0.0f};
      for (int n = 0; n < (progress / (total / NUMPIXELS)); n++)
      {
        rgbBuffer[n] = Tools::RGB(0.0f, 0.5f, 0.0f); 
      }
      updatePixels(rgbBuffer);

      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      Serial.println();
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();


  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void Board::init()
{

  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_12 , 1);
  //esp_sleep_enable_ext0_wakeup(GPIO_NUM_14 , 1);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_23 , 1);
  esp_sleep_enable_timer_wakeup(1000000 / FPSTARGET);

  // Turn off unneded stuff to safe energy
  // Stop Bluetooth
  btStop();

  // Disable Wifi
  WiFi.disconnect(true);  // Disconnect from the network
  WiFi.mode(WIFI_OFF);    // Switch WiFi off
 
 // Turn Down CPU Frequency
 setCpuFrequencyMhz(80);

  neoPixels.begin();

  // Setup Rotarry Encoder
  ESP32Encoder::useInternalWeakPullResistors=UP;
	encoder.attachHalfQuad(PINROTENC0, PINROTENC1);

  // Create gamme LUT
  float gamma = 2.0f;
  for (int n=0; n < 256; n++)
  {
    gammaLUT[n] = (int) (powf(((float)n/255.0f), gamma) * 255);
  }
}

void Board::updatePixels(rgb pxls[NUMPIXELS]) 
{
  for (int p = 0; p < NUMPIXELS; p++)
  {
    // Clip Values to 0-1 and apply Gamma LUT Map
     rgb out = pxls[p];
     out.r = min(max(0.0f, out.r), 1.0f);
     out.g = min(max(0.0f, out.g), 1.0f);
     out.b = min(max(0.0f, out.b), 1.0f);
     int8_t r = gammaLUT[(int)(out.r * 255)];
     int8_t g = gammaLUT[(int)(out.g * 255)];
     int8_t b = gammaLUT[(int)(out.b * 255)];
    // Update LEDs
    neoPixels.setPixelColor(p, r, g, b);
  }
  neoPixels.show();
}


Event Board::update()
{

  Event ev = EV_NOTHING;

  ev = updateInputs();

  if (lowPowerMode && !otaActive)
  {
    esp_light_sleep_start();
  }

  // Start OTA Update Server on very long press
  if (ev == EV_INPUT_BTN_SUPERLONG && !otaActive)
  {
     startOTA();
  }

  // Activate Low Power Mode when Rotary Encoder is not moved
  lowPowerMode = (rotSpeed == 0);

  return ev;
}

Event Board::updateInputs()
{
  ArduinoOTA.handle();

  Event ev = EV_NOTHING;
  unsigned long time = millis();

  // Get Rot Encoder Diff
  static int32_t encoderCountOld = 0;
  int32_t count = (int32_t)encoder.getCount();

  rotDiff = count - encoderCountOld;
  encoderCountOld = count;

  // Get Rot Encoder speed
  static unsigned long rotSpeedTimeOld;
  static int posSpeed;
  
  posSpeed += rotDiff; 
  
  if (time - rotSpeedTimeOld > 50)
  {
    rotSpeed = posSpeed;
    posSpeed = 0;
    rotSpeedTimeOld = time;
  }
  
  
  // Navigation Left and Right with Rott Encoder
  static int posNav = 0;
  posNav += rotDiff;
  if (posNav < -100)
  {
    ev = EV_INPUT_RIGHT;
    posNav = 0;
  }
  else if (posNav > 100)
  {
    ev = EV_INPUT_LEFT;
    posNav = 0;
  }
  if (rotSpeed == 0)
  {
    posNav = 0;
  }

  //Serial.println(touchRead(PINUSERBUTTON0));

  // User Button
  static float sensorAverage = touchRead(PINUSERBUTTON0);
  uint16_t sensorValue = touchRead(PINUSERBUTTON0);
  sensorAverage = 0.0001 * sensorValue + 0.9999 * sensorAverage;

  int threshold = (int)(0.90 * sensorAverage);
 
  bool btn0 = sensorValue < threshold;
  int tShort = 20; // Time for Short Press
  int tLong = 1000; // Time for Long Press
  int tSuperLong = 4000; // Time for very long Press 
  static unsigned long tPress = 0;
  static unsigned long tPressSuperLong = 0;
  static unsigned long tOld = 0;
  static bool pressed = false; // Button already was pressed

  int delta =  (int)(millis() - tOld);

  if (btn0)
  {
    tPress += delta;
    tPressSuperLong += delta;

    if (tPressSuperLong > tSuperLong)
    {
      ev = EV_INPUT_BTN_SUPERLONG;
      Serial.println("BTN SUPER LONG");
      tPressSuperLong = 0;
      pressed = true;
    }

    if ((tPress > tLong) && !pressed)
    {
      ev = EV_INPUT_BTN_LONG;
      Serial.println("BTN LONG");
      tPress = 0;
      pressed = true;
    }
  } 
  else if ((tPress > tShort) && !pressed)
  {
    ev = EV_INPUT_BTN;
    tPress = 0;
    Serial.println("BTN SHORT");
  }
  else
  {
    tPress = 0;
    tPressSuperLong = 0;
    pressed = false;
  }
  

  tOld = millis();
  return ev;
}
  
