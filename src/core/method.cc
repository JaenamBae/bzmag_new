#include "method.h"

/*#include <boost/tokenizer.hpp>*/
#include "node.h"
#include "uri.h"
#include "vector2.h"
#include "color.h"

using namespace bzmag;

//-----------------------------------------------------------------------------
Method::Method()
{
    // empty
}


//-----------------------------------------------------------------------------
Method::~Method()
{
    // empty
}


//-----------------------------------------------------------------------------
void Method::setPrototype(const String& proto)
{
    if (prototype_.size())
        return;

    prototype_ = proto;

    Tokenizer tok(prototype_.c_str(), '_');
    String out_arg = tok[0];
    name_ = tok[1];
    String in_arg = tok[2];

    parameter_.clear();
    build_paramter(out_arg, parameter_.out());
    build_paramter(in_arg, parameter_.in());
}


//-----------------------------------------------------------------------------
const String& Method::getPrototype() const
{
    return prototype_;
}


//-----------------------------------------------------------------------------
void Method::build_paramter(const String& s, Variables* v)
{
    for (size_t i = 0; i < s.size(); ++i)
    {
        // matching to python building values except uri, vector, color, 
        // object, node, list
        switch (s.at(i))
        {
        case 'v':
            continue;
        case 'b':
            v->add<bool>(false);
            break;
        case 'h':
            v->add<int16>(0);
            break;
        case 'H':
            v->add<uint16>(0);
            break;
        case 'i':
            v->add<int>(0);
            break;
        case 'I':
            v->add<unsigned int>(0);
            break;
        case 'l':
            v->add<int32>(0);
            break;
        case 'k':
            v->add<uint32>(0);
            break;
        case 'L':
            v->add<int64>(0);
            break;
        case 'K':
            v->add<uint64>(0);
            break;
        case 'f':
            v->add<float32>(0);
            break;
        case 'd':
            v->add<float64>(0);
            break;
        case 's':
            v->add<String>("");
            break;
        case 'u':
            v->add<Uri>(Uri());
            break;
        case 't':
            v->add<Vector2>(Vector2());
            break;
        case 'c':
            v->add<Color>(Color());
            break;
        case 'o':
            v->add<Object*>(0);
            break;
        case 'n':
            v->add<Node*>(0);
            break;
        case 'z':
            v->setForceList(true);
            break;
        }
    }
}
