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
  game.setupGame();
  board.init();
  board.startOTA();
}


void loop(){

	Event ret = EV_NOTHING;
  t = esp_timer_get_time();
  
  // Update inputs
  ev = board.updateInputs();

  // Draw
  if (Tools::capFPS(t, FPSTARGET)) {

    ret = animator.run(gs, board, game);
    if (ret) {ev = ret;}

    board.updatePixels(animator.rgbBuffer0);
  }

  if (ev)
  {
    Serial.println(ev);
  }

  //Update game logic
  ev = game.update(board, ev);
 

  // Update State Machine
  gs = fsm.run(ev);

  ev = EV_NOTHING;
}


