#include "color.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Color::Color():
data_(0)
{
    // empty
}


//-----------------------------------------------------------------------------
Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a):
r_(r), g_(g), b_(b), a_(a)
{
    // empty
}

