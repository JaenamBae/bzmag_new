#ifndef BZMAG_CORE_SERIALIZER_UNIFIEDSERIALIZER_H
#define BZMAG_CORE_SERIALIZER_UNIFIEDSERIALIZER_H
/**
    @ingroup bzmagCoreSerializer
    @class bzmag::UnifiedSerializer
    @brief
*/

#include <map>
#include "platform.h"
#include "define.h"
#include "string.h"
#include "serializer.h"

namespace bzmag
{
    class Object;
    class BZMAG_LIBRARY_API UnifiedSerializer : public Serializer
    {
    public:
        virtual bool serialize(const Uri& uri, Object* object);
        virtual Object* deserialize(Node* parent, const Uri& uri, const char* name);

    private:
        typedef std::map<String, Serializer*> Serializers;

    private:
        Serializers serializers_;

    };
}

#endif // BZMAG_CORE_SERIALIZER_UNIFIEDSERIALIZER_H
