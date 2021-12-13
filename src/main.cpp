#include <Arduino.h>

#include <config.h>
#include <GameFSM.h>
#include <GameLogic.h>
#include "Tools.h"
#include <Animation.h>

#include "RemoteDebug.h" 

#define HOST_NAME "remotedebug"

RemoteDebug Debug;

GameFSM fsm = GameFSM();
GameLogic game = GameLogic();
Board board = Board();
Animation animator = Animation();

GameState gs = GS_BOOT;
Event ev = EV_NOTHING;

uint64_t t = 0;


void setup() {
  Serial.begin(115200);
  board.init();
  game.setupGame();
  animator.init();

/*
  Debug.begin(HOST_NAME); // Initialize the WiFi server
  Debug.setResetCmdEnabled(true); // Enable the reset command

	Debug.showProfiler(true); // Profiler (Good to measure times, to optimize codes)
	Debug.showColors(true); // Colors
  */
}


void loop(){

  t = esp_timer_get_time();
  
  // Update inputs
  ev = board.update();

  // Draw
  if ((Tools::capFPS(t, FPSTARGET) || ev) || board.lowPowerMode) {

    ev = animator.run(ev, gs, board, game);

    board.updatePixels(animator.rgbBuffer0);
  }

  //Update game logic
  ev = game.update(board, gs, ev);
  
  
  // Update State Machine
  gs = fsm.run(ev);

  ev = EV_NOTHING;
}


