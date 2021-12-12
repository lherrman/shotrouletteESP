

#ifndef __GAME_LOGIC_H__
#define __GAME_LOGIC_H__

#include <Arduino.h>
#include "Tools.h"
#include "board.h"
#include "config.h"
#include "GameFSM.h"

class GameLogic {
  public: 
    GameLogic();
    
    void setupGame();
    Event update(Board board, GameState gs, Event ev);
    
    void takeShot();

    void nextPlayer();
    void setPlayerColor(int n, rgb color);

    void spin();

    void spin_rare0(); // Others take 1 each
    void spin_std(int n); // You take n

    float playerPos; // Rotary encoder position 
    
    int nPlayers = 4;
    int activePlayer = 0;
    int wins[MAXPLAYERS][4]; // Holders index which shots to take for each Player in current Round
    char nWins[MAXPLAYERS]; // Number of won shot this round for each player
    char holders[NUMSHOTS]; // Tells who won which shot

  private:

  int getRandomPos();

}; 

#endif /*__GAME_LOGIC_H__*/
