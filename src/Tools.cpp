#include <Arduino.h>
#include <Tools.h>


bool Tools::capFPS(uint64_t time, int fps)
{
  static uint64_t timeLastFrame = 0;
  int frameDeltaTime = 1000000 / fps ;
  if ((time - timeLastFrame) > frameDeltaTime) {
    timeLastFrame = time;
    return true;
  }
  else {
    return false;
  }
}


float Tools::wrapf(float in, float f)
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

 rgb Tools::rgbMul(rgb in, float frac)
 {
     rgb out;
     out.r = in.r *= frac;
     out.g = in.g *= frac;
     out.b = in.b *= frac;

    return out;
 }

rgb Tools::rgbAdd(rgb in1, rgb in2)
{
    rgb out;
    out.r = _min(in1.r + in2.r, 1.0);
    out.g = _min(in1.g + in2.g, 1.0);
    out.b = _min(in1.b + in2.b, 1.0);
    return out;
}

rgb Tools::RGB(float r, float g, float b)
{
    rgb out;
    out.r = r;
    out.g = g;
    out.b = b;
    return out;
}

hsv Tools::rgb2hsv(rgb in)
{
    hsv         out;
    float      min, max, delta;

    min = in.r < in.g ? in.r : in.g;
    min = min  < in.b ? min  : in.b;

    max = in.r > in.g ? in.r : in.g;
    max = max  > in.b ? max  : in.b;

    out.v = max;        
    delta = max - min;
    if (delta < 0.00001)
    {
        out.s = 0;
        out.h = 0; // Undefined, maybe nan?
        return out;
    }
    if( max > 0.0 ) 
    { // NOTE: if Max is == 0, this divide would cause a crash
        out.s = (delta / max);                 
    } else {
        // if max is 0, then r = g = b = 0              
        // s = 0, h is undefined
        out.s = 0.0;
        out.h = 0.0; // Undefined
        return out;
    }
    if( in.r >= max )                          
        out.h = ( in.g - in.b ) / delta;       
    else
    if( in.g >= max )
        out.h = 2.0 + ( in.b - in.r ) / delta; 
    else
        out.h = 4.0 + ( in.r - in.g ) / delta;  

    out.h *= 60.0;  

    if( out.h < 0.0 )
        out.h += 360.0;

    return out;
}


rgb Tools::hsv2rgb(hsv in)
{
    float      hh, p, q, t, ff;
    long        i;
    rgb         out;

    if(in.s <= 0.0) {  
        out.r = in.v;
        out.g = in.v;
        out.b = in.v;
        return out;
    }
    hh = in.h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = in.v * (1.0 - in.s);
    q = in.v * (1.0 - (in.s * ff));
    t = in.v * (1.0 - (in.s * (1.0 - ff)));

    switch(i) {
    case 0:
        out.r = in.v;
        out.g = t;
        out.b = p;
        break;
    case 1:
        out.r = q;
        out.g = in.v;
        out.b = p;
        break;
    case 2:
        out.r = p;
        out.g = in.v;
        out.b = t;
        break;

    case 3:
        out.r = p;
        out.g = q;
        out.b = in.v;
        break;
    case 4:
        out.r = t;
        out.g = p;
        out.b = in.v;
        break;
    case 5:
    default:
        out.r = in.v;
        out.g = p;
        out.b = q;
        break;
    }
    return out;     
}

/*
rgb Tools::hsv2rgb(hsv hsv)
{
    rgb rgb;
    unsigned char region, remainder, p, q, t;

    if (hsv.s == 0)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }

    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6; 

    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
            break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
            break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
            break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
            break;
    }

    return rgb;
}

hsv Tools::rgb2hsv(rgb rgb) {
  hsv hsv;
    unsigned char rgbMin, rgbMax;

    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);

    hsv.v = rgbMax;
    if (hsv.v == 0)
    {
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }

    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if (hsv.s == 0)
    {
        hsv.h = 0;
        return hsv;
    }

    if (rgbMax == rgb.r)
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    else if (rgbMax == rgb.g)
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
    else
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);

    return hsv;
}
*/




