
#ifndef __ANIMATION_H__
#define __ANIMATION_H__


#include "GameFSM.h"
#include "Tools.h"
#include "board.h"
#include "config.h"
#include <GameLogic.h>


class Animation {
  public:
    Animation();
    Event run(GameState gs, Board board, GameLogic game);
    void init();
    void clrRgbBuffer0();

    rgb rgbBuffer0[NUMPIXELS];

    private:

    // Animations
    Event an_boot_0(Board board, GameLogic game);
    Event an_ready_0(Board board, GameLogic game);
    Event an_spinning_0(Board board, GameLogic game);
    Event an_takeshot_0(Board board, GameLogic game);
    Event an_playercount_0(Board board, GameLogic game);

    int wrap(int in, int n); 
    float wrap360(float in); 
    float wrapf(float in, float f); 

    rgb colorFraction(rgb color, float fraction);
    void drawPixels(float fPos, float count, rgb color);
    void draw(float fPos, float count, rgb color);

    float fade(unsigned long tStart, int dur, float val1, float val2);
  
    float cap(float in);

    rgb playerColors[MAXPLAYERS];
    int spinDir;

    unsigned long t;
    unsigned long t_bg;
};

#endif /*__ANIMATION_H__*/
