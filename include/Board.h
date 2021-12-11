
#ifndef __BOARD_H__
#define __BOARD_H__

#include "Tools.h"
#include "GameFSM.h"
#include "config.h"
#include <Adafruit_NeoPixel.h>
#include <ESP32Encoder.h>

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

class Board {
  public:
    Board();
    void init();
    void startOTA();
    Event updateInputs();
    void updatePixels(rgb pxls[NUMPIXELS]);
    bool sensors[NUMSHOTS] = {false};
    bool userbtn0 = false;

    int rotDiff = 0;
    float rotSpeed = 0.0f;

  private:
   uint8_t gammaLUT[256];

};

#endif
