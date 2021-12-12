#include <Arduino.h>

#include <config.h>
#include <GameFSM.h>
#include <GameLogic.h>
#include "Tools.h"
#include <Animation.h>


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
  board.startOTA();
  game.setupGame();
  animator.init();
}


void loop(){

	Event ret = EV_NOTHING;
  t = esp_timer_get_time();
  
  // Update inputs
  ev = board.updateInputs();

  // Draw
  if (Tools::capFPS(t, FPSTARGET) || ev) {

    ev = animator.run(ev, gs, board, game);

    board.updatePixels(animator.rgbBuffer0);
  }

  //Update game logic
  ev = game.update(board, gs, ev);
 
  // Update State Machine
  gs = fsm.run(ev);

  ev = EV_NOTHING;
}


