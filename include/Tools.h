
#ifndef __TOOLS_H__
#define __TOOLS_H__

 
typedef struct 
{
  float r;
  float g;
  float b;
} rgb;


typedef struct 
{
  float h;
  float s;
  float v;
} hsv;


class Tools 
{
  public:

    static rgb hsv2rgb(hsv in);
    static hsv rgb2hsv(rgb in);
    static rgb RGB(float r, float g, float b);
    static float wrapf(float in, float f);

    static rgb rgbMul(rgb in, float frac);
    static rgb rgbAdd(rgb in1, rgb in2);

    static bool capFPS(uint64_t time, int fps);
    
  private:
  
    double mat3[3][3];

};



#endif /*__TOOLS_H__*/
