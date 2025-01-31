#include "tokenizer.h"

namespace bzmag
{

//-----------------------------------------------------------------------------
Tokenizer::Tokenizer(const char* s, const char sep)
{
    tokenize(s, sep);
};


//-----------------------------------------------------------------------------
size_t Tokenizer::tokenize(const char* s, const char sep)
{
    clear();
    String tmp(s);
    if (tmp.size() == 0) {
        tokens.emplace_back(s);
        return size();
    }

    tmp.resize(tmp.size()+1);
    char* b = &tmp.at(0);
    char* e = b;
    char* ee = &tmp.at(tmp.size()-1);
    bool f = true;
    
    do
    {
        if (f) { b = e; f = false; }
        if (*e==sep)
        {
            *e = 0;
            if (e-b>0) tokens.emplace_back(b);
            f = true;
        }

    } while (++e<ee);
    if (!f && e-b>0) tokens.emplace_back(b);
    

    return size();
}


//-----------------------------------------------------------------------------
void Tokenizer::clear()
{
    tokens.clear();
}


//-----------------------------------------------------------------------------
bool Tokenizer::empty() const
{
    return tokens.empty();
}


//-----------------------------------------------------------------------------
size_t Tokenizer::size() const
{
    return tokens.size();
}


//-----------------------------------------------------------------------------
Tokenizer::iterator Tokenizer::begin()
{
    return tokens.begin();
}


//-----------------------------------------------------------------------------
Tokenizer::iterator Tokenizer::end()
{
    return tokens.end();
}

//-----------------------------------------------------------------------------
Tokenizer::const_iterator Tokenizer::begin() const
{
    return tokens.begin();
}


//-----------------------------------------------------------------------------
Tokenizer::const_iterator Tokenizer::end() const
{
    return tokens.end();
}


//-----------------------------------------------------------------------------
const String& Tokenizer::operator [] (size_t index) const
{
    return tokens[index];
}

}
