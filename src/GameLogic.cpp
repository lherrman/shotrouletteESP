
#include <GameLogic.h>
#include <config.h>
#include "GameFSM.h"

GameLogic::GameLogic() {
}

Event GameLogic::update(Board board, GameState gs, Event ev) {
    
    playerPos = playerPos + (float)board.rotDiff;

    // Trigger Ball Spinning
    if(ev == EV_SPIN_TRIGGER)
    {
        spin();
    }

    // When Shot Taken, check if game is over then reset
    // if not next Player active
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

    // In Player Selection State Update nPlayers by Turning Encoder
    if (gs == GS_PLAYERCOUNT_SELECT)
    {
        if ((ev == EV_INPUT_RIGHT) && (nPlayers > 1))
        {
           nPlayers  -= 1;
        }

        if ((ev == EV_INPUT_LEFT) && (nPlayers < MAXPLAYERS))
        {
            nPlayers  += 1;
        }
    }

    return ev;
}

void GameLogic::setupGame()
{
    #if DEBUG == 1
        Serial.println("New Game Startet");
    #endif

    // Reset Holders satate
    for (int n = 0; n < NUMSHOTS; n++)
    {
        holders[n] = false;
    }
    
    nPlayers = 3;
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
    srand((unsigned int)esp_timer_get_time() + seed); // Seed
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