

#ifndef __GAME_FSM_H__
#define __GAME_FSM_H__

enum GameState {
  GS_BOOT,
  GS_PLAYERCOUNT_SELECT,
  GS_READY,
  GS_SPINNING,
  GS_TAKESHOT,
  GS_PLAYERCOLORS
};

enum Event {
  EV_NOTHING,
  EV_INPUT_LEFT,
  EV_INPUT_RIGHT,
  EV_SPIN_TRIGGER,
  EV_INPUT_BTN,
  EV_INPUT_BTN_LONG,
  EV_SHOT_TAKEN,
  EV_ANIMATION_END,
  EV_RESTART
};

class GameFSM {
  public: 
    GameFSM();
    GameState run(Event ev = EV_NOTHING);
    
  private:
    GameState _currentState;
    GameState _nextState  ;
}; 

#endif /*__GAME_FSM_H__*/
