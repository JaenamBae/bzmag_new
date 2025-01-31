#ifndef BZMAG_CORE_TYPE_PATH_H
#define BZMAG_CORE_TYPE_PATH_H
/**
@ingroup bzmagCoreType
@class bzmag::Path
@brief Node of Hierarchy Path

@section Path
- AbsolutePath
- /usr/scene/fighter
- RelativePath
- fighter/weapon
*/

#include <wchar.h>
#include "platform.h"
#include "define.h"
#include "string.h"
#include "tokenizer.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Path : public String
    {
    public:
        typedef Tokenizer::iterator iterator;
        typedef Tokenizer::const_iterator const_iterator;

    public:
        Path();
        Path(const char* path);
        Path(const wchar_t* path);
        Path(const Path& path);
        Path(const String& path);

        bool isAbsolute() const;
        bool isRelative() const;

        iterator begin();
        iterator end();

        const_iterator begin() const;
        const_iterator end() const;

    private:
        mutable Tokenizer tokenizer_;
    };

#include "path.inl"

}

#endif // BZMAG_CORE_TYPE_PATH_H
