#ifndef BZMAG_ENGINE_NODE_STRINGCONVERTER_H
#define BZMAG_ENGINE_NODE_STRINGCONVERTER_H

#include "core/primitive_stringconverter.h"
#include "CSNode.h"
#include "MaterialNode.h"
#include "MasterPeriodicBCNode.h"

namespace bzmag
{
namespace engine
{
    class CSNodeStringConverter : public NodeStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<CSNode*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "CSNode";
        }
    };

    class MaterialNodeStringConverter : public NodeStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<MaterialNode*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "MaterialNode";
        }
    };

    class GeomHeadNodeStringConverter : public NodeStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<GeomHeadNode*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "GeomHeadNode";
        }
    };

    class GeomCloneToNodeStringConverter : public NodeStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<GeomCloneToNode*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "GeomCloneToNode";
        }
    };

    class MasterPeriodicBCNodeStringConverter : public NodeStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<MasterPeriodicBCNode*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "MasterPeriodicBCNode";
        }
    };
}
}

#endif // BZMAG_ENGINE_NODE_STRINGCONVERTER_H
