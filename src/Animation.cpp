
#include <Arduino.h>
#include <Animation.h>
#include "config.h"
#include "GameFSM.h"
#include "Tools.h"
#include "board.h"
#include <GameLogic.h>

Animation::Animation()
: t(0) // frames elapsed since last state change
, t_bg(0) // frames elapsed since startup for background animation
{
}

Event Animation::run(GameState gs, Board board, GameLogic game)
{   
    static GameState gs_old;
    Event ev;
    t += 1; 
    t_bg += 1;

    // Detect State change
    if (gs != gs_old) {

        #if DEBUG == 1
                Serial.print("State ");
                Serial.print(gs);
                Serial.println(" entered");
        #endif

        t = 0;  
        gs_old = gs;
    }
    
    // Jump in animation state
    switch (gs) {
    case GS_BOOT:

        ev = an_boot_0(board, game);

    break;

    case GS_PLAYERCOUNT_SELECT:

        ev = an_playercount_0(board, game);

    break;

    case GS_READY:
        
        ev = an_ready_0(board, game);
        
    break;

    case GS_SPINNING:
    
      ev = an_spinning_0(board, game);

    break;

    case GS_TAKESHOT:

      ev = an_takeshot_0(board, game);

    break;
    
    default:
        ev = EV_NOTHING;
    break;
    }

    return ev;
}


Event Animation::an_boot_0(Board board, GameLogic game) 
{
    static float playerPosOld = {0.0f};
    float delta = game.playerPos - playerPosOld;
    playerPosOld = game.playerPos;

    // Color Cycle
    hsv a;
    for (int n=0; n < NUMPIXELS; n++)
    {
         a.h = wrap360((360 * (n + (t/5))) / NUMPIXELS);
         a.s = 1.0f;
         a.v = fade(300, 80, 0.8f, 0.0f) * fade(0, 10, 0.0f, 1.0f);
        rgb b = Tools::hsv2rgb(a);
        rgbBuffer0[n] = b; 
    }

    if ((a.v <= 0.0f || abs(delta) > 2 ) && t > 100)
    {
      return EV_ANIMATION_END;
    }
    else
    {
      return EV_NOTHING;
    }
}

Event Animation::an_ready_0(Board board, GameLogic game)
{
  clrRgbBuffer0();
  
  // Player Position Variables
  static float posSmooth = 0.0f;
  static float playerPosOld = {0.0f};
  static int spinDirOld = {0};
  static float speed = {0.0f};

  // Auto Drift Varaibles
  static int noInputTimer = 0;
  int autoDriftDelay = 500;

  // Delta of Rotary Encoder Pos
 float delta = game.playerPos - playerPosOld;

  // Stops moving when entring States
  if (t == 0)
  {
    noInputTimer = autoDriftDelay;
    delta = 0;
  }
  
  // Update Speed change
  speed += delta;
  speed *= 0.95f;

 // Auto Drift after no input
  noInputTimer += 1;
  float autoDrift = (float)(noInputTimer > autoDriftDelay)*min((noInputTimer-autoDriftDelay)/100.0f, 5.0f);
  if (delta != 0)
  {
    noInputTimer = 0;
  }

  // Update Position
  posSmooth += wrapf(0.01f * (speed + delta + autoDrift), NUMPIXELS);

  // Detect Spin Direction 
  if (delta != 0)
  {
     spinDir = delta / abs(delta);
  }
  
  if (spinDir != spinDirOld)
  {
    // Reset Speed on changed direction
    speed = 0;
    spinDirOld = spinDir;
  }
 
  // Wrap position arround circle
  float pos = wrapf((posSmooth), NUMPIXELS);


  // Draw playercolors of taken shotglasses
  if (autoDrift != 0)
  {
    for (int n=0; n<NUMSHOTS-1; n++)
    { 
      if(game.holders[n] != 0)
      {
      rgbBuffer0[2*n + 1] =  Tools::rgbMul(game.playerColors[game.holders[n]-1], 0.1);
      }  
    }
  }

  // Draw ball
  static float width = 1.0f;
  width =  _max(_min(width + (0.1*(autoDrift-0.5f)) , 5.0f), 1.0f);
  float fadeFac =  min(1.0f,((float)t)/50.0f);
  draw(pos- (width/2) , width, Tools::rgbMul(game.playerColors[game.activePlayer], fadeFac));
 
  playerPosOld = game.playerPos;

  // Trigger Spin
  if ((abs(speed) >  170.0f ) && t>5)
  {
    speed = 0.0f;
    return EV_SPIN_TRIGGER;

  }
  else
  {
    return EV_NOTHING;
  }
}


Event Animation::an_spinning_0(Board board, GameLogic game)
{
  clrRgbBuffer0();
 
  float v0 = 2.0f;
  int n = 20;
  int winPixels[MAXWINS];
  float s[MAXWINS]; // Starting dists to win pos
  float a[MAXWINS]; // calculatet acceleration
  static float p[MAXWINS];  // positions
  static float v[MAXWINS]; // speeds
  bool visible[MAXWINS];

  float width = 1.0f; // width of ball

  int i = 0; // Array Index

  // Initialize ball variables
  if (t == 0)
  {
    for (int n = 0; n < MAXWINS; n++)
    {
      v[n] = 0.0f;
      p[n] = 0.0f;
      visible[n] = false;
    }
    v[0] = v0;
  }

  for (int pl = 0; pl < MAXPLAYERS; pl++) // For every Player
  {
    for (int wn=0; wn<game.nWins[pl]; wn++) // For every of his wins
    {
      // Set Initial Velocity
      if (t == 0)
      {
        v[i] = v0;
      }
      
      // Calculate winning pixel positions
      // from winning shot indeces
      winPixels[i] = 2 * game.wins[pl][wn] + 1;

      // Calculate distance from start to winning pos
      // depending on spinning direction

      if (spinDir == 1)
      {
          s[i] = (((float)NUMPIXELS * (n+i+i))) + (v0/2) + winPixels[i];
      }
      else
      {
          s[i] = (((float)NUMPIXELS * (n+i+i))) + (v0/2) - winPixels[i];
      }

      // Calculate needed Drag factor
      a[i] = (v0*v0)/(2.0f * s[i]); 

      // Update velocity
      v[i] -= a[i]; 
      if (v[i] <= 0.0f)
      {
        v[i] = 0.0f;
      }

       // Update position and Wrap position arround circle
      if ((v[i] > 0.0f) && (spinDir == 1))
      {
        p[i] = wrapf(p[i] + v[i], NUMPIXELS);
      }
      else if ((v[i] > 0.0f))
      {
        p[i] = wrapf(p[i] - v[i], NUMPIXELS);
      }

      // Draw balls
      // For first part of Spin show only first ball
      // then add balls that are close to p[0]
      rgb col;
      visible[0] = true;
      if (v[0] < v0 / 3)
      {
        col = game.playerColors[pl];
        for (int n = 0; n < MAXWINS; n++)
        {
          if ((abs(p[n] - p[0]) < 1) && (abs(v[n] - v[0]) < 1))
          {
            visible[n] = true;
          }
        }
      }
      else
      {
        col = game.playerColors[game.activePlayer];
      }

      // If only others need to drink, flash rainbow colors
      // at start of split
      if (game.nWins[game.activePlayer] == 0 && v[0] < v0/2 && v[0] > v0/3)
      {
        hsv a;
        a.h = wrap360(5.0f * t);
        a.s = 1;
        a.v = 1;
        col = Tools::hsv2rgb(a);
      }
     
      if (visible[i])
      {
        draw(p[i], width + (v[i] * 0.3f), col);
      }

      i += 1;
    }  
  }

  // Animation done when all balls stand
  bool done = true;
  for (int n = 0; n < MAXWINS; n++)
  {
    if (v[n] != 0.0f)
    {
      done = false;
    }
  }

  // End animation
  if (done && t > 5)
  { 
    return EV_ANIMATION_END;
  }
  else 
  {
    return EV_NOTHING;
  }
}


Event Animation::an_takeshot_0(Board board, GameLogic game)
{
  clrRgbBuffer0();

  // Draw balls
  float width = 0.6f + (0.4 * cos((t)/40.0f));
  int fadeDelay = 1200;
  float fadeDur = 50.0f;
  float fadeFac = min((fadeDur - max(0,((int)t-fadeDelay))) / fadeDur, 1.0f);
  int winPixel;
  for (int pl = 0; pl < MAXPLAYERS; pl++)
  {
    for (int wn=0; wn<game.nWins[pl]; wn++)
    {
      winPixel = 2*game.wins[pl][wn] + 1;
      draw(winPixel - (width/2) + 0.5, width, Tools::rgbMul(game.playerColors[pl], fadeFac));
    }
  }
  
  // Get Position dealta (rot enc speed) to trigger next State
  static float playerPosOld = {0.0f};
  float delta = (playerPosOld - game.playerPos);
  playerPosOld = game.playerPos;

  // Trigger end of animation
  if ((abs(delta) > 1 && t > 1) || t > fadeDelay+fadeDur)
  {
    return EV_SHOT_TAKEN;
  }
  else
  {
    return EV_NOTHING;
  }
}


 Event Animation::an_playercount_0(Board board, GameLogic game)
 {
    clrRgbBuffer0();

    static float pos[MAXPLAYERS + 1] = {0.0f};
    pos[0] = 0.0f;
    pos[MAXPLAYERS] = NUMPIXELS;
    static float old = 0;
    for (int i = 1; i <= MAXPLAYERS; i++)
    {
      float left = (pos[i + 1] - pos[i]);
      float right = (pos[i] - pos[i - 1]);

      if (i < game.nPlayers)
      {
        pos[i] += 0.02 * (left - right) * game.nPlayers;
      
      }
      else if (pos[i] < NUMPIXELS)
      {
        pos[i] += 0.05f * left * game.nPlayers; 
        
      }
      draw(pos[i]-right, right , game.playerColors[i-1]);

    }


    return EV_NOTHING;
 }


rgb Animation::colorFraction(rgb color, float fraction)
{
   fraction = min(1.0f, fraction);
   color = Tools::rgbMul(color, fraction);
   return color;
}
  

float Animation::fade(unsigned long tStart, int dur, float val1, float val2)
{
  float out = val1;

  if (t > tStart)
  {
    float sp = val2 - val1;
    sp = sp / float(dur);
    out = max(min(val1 + ((t-tStart) * sp), 1.0f), 0.0f) ;
  }
  return out;
}


float Animation::cap(float in)
{
  // Cap flaot between 0 and 1
  return min(max(in, 0.0f), 1.0f);
}


void Animation::draw(float fPos, float count, rgb color)
{  

  fPos = wrapf(fPos, NUMPIXELS);

  if (fPos + count < (float)NUMPIXELS)
  {
    drawPixels(fPos, count, color);
  }
  else
  { 
    drawPixels(fPos, (float)NUMPIXELS - fPos, color);
    drawPixels(0.0f, fPos + count - NUMPIXELS  , color);
  }
}


void Animation::drawPixels(float fPos, float count, rgb color)
{
  //calculate value of First Pixel
  float facFirstPixel = 1.0f - (fPos - (long) (fPos));
  float amtFirstPixel = min(facFirstPixel, count);
  float remaining = min(count, NUMPIXELS - fPos);
  int iPos = fPos;
  
  if ((fPos + count) <= NUMPIXELS)
  {
      // Blend in color of first partial pixel
    if (remaining > 0.0f)
    {
      rgbBuffer0[iPos].r += colorFraction(color, amtFirstPixel).r;
      rgbBuffer0[iPos].g += colorFraction(color, amtFirstPixel).g;
      rgbBuffer0[iPos++].b += colorFraction(color, amtFirstPixel).b;
      remaining -= amtFirstPixel;
    }

    // Draw Pixels in middle
    while (remaining > 1.0f)
    {
      rgbBuffer0[iPos].r += color.r;
      rgbBuffer0[iPos].g += color.g;
      rgbBuffer0[iPos++].b += color.b;
      remaining --;
    }

    // Draw tail pixel
    if (remaining > 0.0f)
    {
      rgbBuffer0[iPos].r += colorFraction(color, remaining).r;
      rgbBuffer0[iPos].g += colorFraction(color, remaining).g;
      rgbBuffer0[iPos++].b += colorFraction(color, remaining).b;
    }
  }   
}


float Animation::wrap360(float in)
{
  if (in < 360. && in >= 0) {
    return in;
  }
  else if (in >= 360.) {
    float out = fmod(in, 360.);
    return out;
  }
  else {
    float out = fmod(-in, 360.);
    out = 360. - out;
    return out;
  }
}


float Animation::wrapf(float in, float f)
{
  if (in < f && in >= 0) {
    return in;
  }
  else if (in >= f) {
    float out = fmod(in, f);
    return out;
  }
  else {
    float out = fmod(-in, f);
    ;
    out = f - out;
    return out;
  }
}


int Animation::wrap(int in, int n)
{
    if (in >= 0) return in % n;
  else return n -  (((-in)) % n);
}


void Animation::clrRgbBuffer0() 
{
    for (int n=0; n<NUMPIXELS; n++)
    {
        rgbBuffer0[n].r = 0;
        rgbBuffer0[n].g = 0;
        rgbBuffer0[n].b = 0;
    }
}