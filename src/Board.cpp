

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
    .onProgress([](unsigned int progress, unsigned int total) {
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
  static float sensorAverage = 20.0f;
  uint16_t sensorValue = touchRead(PINUSERBUTTON0);
  sensorAverage = 0.0002 * sensorValue + 0.9998 * sensorAverage;

  int threshold = (int)(0.80 * sensorAverage);
 
  bool btn0 = sensorValue < threshold;
  int tShort = 20; // Time for Short Press
  int tLong = 1000; // Time for Long Press
  static int tPress = 0;
  static bool pressed = false; // Button already was pressed


  if (btn0)
  {
    tPress += 1;
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
    pressed = false;
  }
  
  return ev;
}
  
