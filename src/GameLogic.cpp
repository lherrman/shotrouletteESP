
#include <GameLogic.h>
#include <config.h>
#include "GameFSM.h"

GameLogic::GameLogic() {
}

Event GameLogic::update(Board board, Event ev) {
    
    playerPos = playerPos + (float)board.rotDiff;

    if(ev == EV_SPIN_TRIGGER)
    {
        spin();
    }

    if (ev == EV_INPUT_LEFT)
    {
        if (nPlayers > 1)
        {
             nPlayers  -= 1;
        }
        Serial.println(nPlayers);
    }

    if (ev == EV_INPUT_RIGHT)
    {
        if (nPlayers < MAXPLAYERS)
        {
             nPlayers  += 1;
        }
        Serial.println(nPlayers);
    }

    if (ev == EV_SHOT_TAKEN)
    {
        // Reset when all shots taken
        bool rst = true;
        for (int n = 0; n < NUMSHOTS; n++)
        {
            if (holders[n] == 0)
            {
                rst = false;
                break;
            }
        }
        if (rst)
        {
            setupGame();
            ev = EV_RESTART;
        }
        else
        {
            nextPlayer();
        }
    }

    return ev;
}

void GameLogic::setupGame()
{
    // Reset Holders satate
    for (int n = 0; n < NUMSHOTS; n++)
    {
        holders[n] = false;
    }

    // Set Player Colors
    #if MAXPLAYERS >= 1
        playerColors[0] = Tools::RGB(0.0f, 0.1f, 0.7f);
    #endif
    #if MAXPLAYERS >= 2
        playerColors[1] = Tools::RGB(0.1f, 0.7f, 0.1f);
    #endif
    #if MAXPLAYERS >= 3
        playerColors[2] = Tools::RGB(0.7f, 0.1f, 0.0f);
    #endif
    #if MAXPLAYERS >= 4
        playerColors[3] = Tools::RGB(1.0f, 0.0f, 1.0f);
    #endif
    #if MAXPLAYERS >= 5
        playerColors[4] = Tools::RGB(1.0f, 1.0f, 0.0f);
    #endif
    #if MAXPLAYERS >= 6
        playerColors[5] = Tools::RGB(0.4f, 0.6f, 0.1f);
    #endif
    #if MAXPLAYERS >= 7
        playerColors[6] = Tools::RGB(0.0f, 0.9f, 0.7f);
    #endif
    #if MAXPLAYERS >= 8
        playerColors[7] = Tools::RGB(1.0f, 1.0f, 0.7f);
    #endif
    
    nPlayers = 2;
    activePlayer = 0;
}

void GameLogic::nextPlayer()
{
    activePlayer = (activePlayer + 1) % nPlayers;
}

void GameLogic::spin()
{
    // Count untaken Shots
    int cnt = 0;
    for (int c = 0; c < NUMSHOTS; c++)
    {
        if (holders[c] == 0)
        {
            cnt += 1;
        }
    }

    // Select Spin Randomly
    srand(millis());
    float rndVar = (float)rand() / (float)RAND_MAX;

    float p0 = 0.20f; // Probability rare spin 0 (others take 1 each)
    float p1 = 0.10f; // Probability std spin 2 (you take 3)
    float p2 = 0.20f; // Probability std spin 3 (you take 2)
    
    if (rndVar < p0 && cnt >= nPlayers-1)
    {
        spin_rare0(); // Others take 1
    }
    else if (rndVar < p0 + p1 && cnt >= 3)
    {
        spin_std(3); // You take 3
    }
    else if (rndVar < p0 + p1 + p2 && cnt >= 2)
    {
        spin_std(2); // You take 2
    }
    else
    {
        spin_std(1); // You Take
    }
}


void GameLogic::spin_rare0() // others take 1
{
    for (int pl = 0; pl < nPlayers; pl++)
    {
        if (pl == activePlayer)
        {
            // Active Player get no Shots
            Serial.println("me");
            nWins[pl] = 0;
        }
        else
        {
            // Other Players get 1 shot each
            int winIndex = getRandomPos();
            wins[pl][0] = winIndex;
            nWins[pl] = 1;
            holders[winIndex] = pl + 1;
        }  
    }
}


void GameLogic::spin_std(int n) 
{
    // Standard Spin, active player gets n shots
    for (int pl = 0; pl < nPlayers; pl++)
    {
        nWins[pl] = 0;
    }

    for (int i= 0; i<n;i++){
        int winIndex = getRandomPos();
        wins[activePlayer][i] = winIndex;
        nWins[activePlayer] = n;
        holders[winIndex] = activePlayer + 1;
    }
}


int GameLogic::getRandomPos() 
{
     // Select Random free Poisition
    static unsigned long seed = 0;
    seed += 100;
    srand(millis() + seed); // Seed
    float rndVar = (float)(rand() / (float)RAND_MAX);
    int winIndex = (int)(rndVar * (NUMSHOTS)) ; 
    for(int n=0; n< NUMSHOTS; n++)
    {
        if (holders[winIndex] == false)
        {
            break;
        }
        else 
        {
            winIndex++;
            winIndex = winIndex % NUMSHOTS;
        }
    }
    return winIndex;
}