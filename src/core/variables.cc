#include "variables.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Variables::Variables():
forceList_(false)
{
    // empty
}


//-----------------------------------------------------------------------------
Variables::~Variables()
{
    clear();
}


//-----------------------------------------------------------------------------
void Variables::clear()
{
    for (Array::iterator iter = array_.begin();
        iter != array_.end(); ++iter)
        delete *iter;
    array_.clear();
}

//-----------------------------------------------------------------------------
bool Variables::empty() const
{
    return size() == 0;
}


//-----------------------------------------------------------------------------
size_t Variables::size() const
{
    return array_.size();
}
