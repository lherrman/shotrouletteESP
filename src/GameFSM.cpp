

#include <Arduino.h>
#include "GameFSM.h"


GameFSM::GameFSM()
  : _currentState(GS_BOOT)
  , _nextState(GS_BOOT){
  }


GameState GameFSM::run(Event ev)
{
  
  switch (_currentState) {
    case GS_BOOT:
      // State Bootup Animation
      if (ev == EV_ANIMATION_END) 
      {
        _nextState = GS_PLAYERCOUNT_SELECT;
      }  
      break;
  
    case GS_READY:
      // State Ready for Spin
      if (ev == EV_SPIN_TRIGGER) 
      {
        _nextState = GS_SPINNING;
      } 
      else if (ev == EV_INPUT_BTN_LONG) 
      {
        _nextState = GS_PLAYERCOUNT_SELECT;
      }
      
    break;

    case GS_SPINNING:
      // State Ball Spinning 
      if (ev == EV_ANIMATION_END) 
      {
        _nextState = GS_TAKESHOT;
      }
    break;

    case GS_TAKESHOT:
      // State Take Shot
      if (ev == EV_SHOT_TAKEN) 
      {
        _nextState = GS_READY;
      }
      else if (ev == EV_RESTART)
      {
        _nextState = GS_BOOT;
      }
    break;

    case GS_PLAYERCOUNT_SELECT:
      // State PLayercount Selection
      if (ev == EV_INPUT_BTN_LONG)
      {
        _nextState = GS_READY;
      }
    break;

    case GS_PLAYERCOLORS:
      // State PLayer Select Colors
      if (ev == EV_INPUT_BTN_LONG)
      {
        _nextState = GS_READY;
      }
    break;
  } 

  
  if (_nextState != _currentState)
  {
      _currentState = _nextState;
  }
  

   _currentState = _nextState;
  return _currentState;
}
  
