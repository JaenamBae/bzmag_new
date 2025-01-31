#ifndef BZMAG_CORE_UTILITY_TOKENIZER_H
#define BZMAG_CORE_UTILITY_TOKENIZER_H
/**
@ingroup bzmagCoreUtility
@class bzmag::Tokenizer
@brief Tokenize given String with the separator
*/

#include <vector>
#include "platform.h"
#include "string.h"

namespace bzmag
{ 

class BZMAG_LIBRARY_API Tokenizer
{
public:
    typedef std::vector<String> Tokens;
    typedef Tokens::iterator iterator;
    typedef Tokens::const_iterator const_iterator;

public:
    Tokenizer(const char* s, const char sep);

    size_t tokenize(const char* s, const char sep);

    void clear();
    bool empty() const;
    size_t size() const;

    iterator begin();
    iterator end();

    const_iterator begin() const;
    const_iterator end() const;

    const String& operator [] (size_t index) const;


private:
    Tokens tokens;
};

}

#endif // BZMAG_CORE_UTILITY_TOKENIZER_H