#ifndef BZMAG_CORE_UTILITY_PRIMITIVE_STRINGCONVERTER_H
#define BZMAG_CORE_UTILITY_PRIMITIVE_STRINGCONVERTER_H

#include <strsafe.h>
#include <sstream>
#include <iostream>
#include "platform.h"
#include "define.h"
#include "kernel.h"
#include "primitivetype.h"
#include "string.h"
#include "vector2.h"
#include "dataset.h"
#include "color.h"
#include "uri.h"
#include "node.h"
#include "autoreleasepool.h"
#include "stringconverter.h"
#include "property.h"
#include "simpleproperty.h"
#include "tokenizer.h"

namespace bzmag {
    //-----------------------------------------------------------------------------
    class BZMAG_LIBRARY_API ObjectStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<Object*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "Object";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<Object*> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            Object* v = p->get(object);

            //char buf[16] = {0};
            // nullptr이면 Object가 존재하지 않으며 이때 ID는 -1로 설정
            // 정상적인 Object의 ID는 0보다 큼
            int ID = -1;
            if (v != nullptr) ID = v->getID();
            str_.format("%d", ID);
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<Object*> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            int32 ID = std::stoi(value.c_str());
            Object* v = nullptr;
            if (ID >= 0) v = AutoReleasePool::instance()->find(ID);
            if (v != nullptr || ID == -1) {
                p->set(object, v);
            }
            else {
                p->set(object, nullptr);
            }
        }

    protected:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class BZMAG_LIBRARY_API NodeStringConverter : public ObjectStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<Node*>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "Node";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<Object*> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            Object* v = p->get(object);

            // 경로를 리턴
            Node* node = dynamic_cast<Node*>(v);
            if (node) {
                str_ = node->getAbsolutePath();
            }
            else {
                str_.clear();
            }
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<Object*> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);

            // 경로로 노드 찾기
            Kernel* kernel = Kernel::instance();
            Object* v = kernel->lookup(value);
            if (v != nullptr) {
                p->set(object, v);
            }
            else {
                p->set(object, nullptr);
            }
        }
    };

    //-----------------------------------------------------------------------------
    class BoolStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<bool>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "bool";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<bool> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format(p->get(object) ? "true" : "false");
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<bool> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);

            bool v = false;
            String val(value);
            if (val == "true" || val == "True")
                v = true;
            p->set(object, v);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class Int16StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<int16>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "int16";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<int16> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<int16> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class UnsignedInt16StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<uint16>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "uint16";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<uint16> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<uint16> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class IntStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<int>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "int";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<int> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<int> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };


    //-----------------------------------------------------------------------------
    class UnsignedIntStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<unsigned int>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "uint";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<unsigned int> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<unsigned int> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class Int32StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<int32>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "int32";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<int32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<int32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };


    //-----------------------------------------------------------------------------
    class UnsignedInt32StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<uint32>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "uint32";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<uint32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<uint32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class Int64StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<int64>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "int64";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<int64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%I64d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<int64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class UnsignedInt64StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<uint64>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "uint64";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<uint64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%I64d", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<uint64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, _atoi64(value));
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class FloatStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<float32>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "float32";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<float32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%g", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<float32> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };


    //-----------------------------------------------------------------------------
    class DoubleStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<float64>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "float64";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<float64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            str_.format("%g", p->get(object));
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<float64> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class StringStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<const String&>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "string";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<const String&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            return p->get(object);
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<const String&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }
    };

    //-----------------------------------------------------------------------------
    class UriStringConverter : public StringStringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<const Uri&>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "uri";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<const Uri&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            return p->get(object).get();
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<const Uri&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            p->set(object, value);
        }
    };

    //-----------------------------------------------------------------------------
    class Vector2StringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<const Vector2&>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "vector2";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<const Vector2&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            const Vector2& v = p->get(object);
            str_.format("%lf, %lf", v.x_, v.y_);
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<const Vector2&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);

            Tokenizer tok(value.c_str(), ',');
            Vector2 v;
            int c = 0;
            for (Tokenizer::iterator i = tok.begin();
                i != tok.end(); ++i, ++c)
                v.set(c, static_cast<float>(atof(i->c_str())));
            p->set(object, v);
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class DataSetStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<const DataSet&>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "dataset";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<const DataSet&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            const DataSet& v = p->get(object);
            std::string ss = serializeToSpaceSeparated(v);
            str_ = String(ss);
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<const DataSet&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);

            DataSet v = deserializeFromSpaceSeparated(value.c_str());
            p->set(object, v);
        }

    private:
        std::string serializeToSpaceSeparated(const DataSet& dataset) {
            std::ostringstream oss;
            for (size_t i = 0; i < dataset.size(); ++i) {
                oss << dataset[i].x_ << " " << dataset[i].y_;
                if (i != dataset.size() - 1) {
                    oss << "; "; // 데이터 구분자
                }
            }
            return oss.str();
        }

        DataSet deserializeFromSpaceSeparated(const std::string& data) {
            DataSet dataset;
            std::istringstream iss(data);
            std::string segment;
            while (std::getline(iss, segment, ';')) {
                std::istringstream segmentStream(segment);
                double x, y;
                segmentStream >> x >> y;
                dataset.emplace_back(x, y);
            }
            return dataset;
        }

    private:
        String str_;
    };

    //-----------------------------------------------------------------------------
    class ColorStringConverter : public StringConverter
    {
    public:
        virtual type_id getTypeId()
        {
            return TypeId<const Color&>::id();
        }
        virtual const char* getTypeKeyword() const
        {
            return "color";
        }
        virtual const String& toString(Object* object, Property* property)
        {
            typedef SimpleProperty<const Color&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);
            const Color& v = p->get(object);
            str_.format("%d, %d, %d, %d", v.r_, v.g_, v.b_, v.a_);
            return str_;
        }
        virtual void fromString(Object* object, Property* property, const String& value)
        {
            typedef SimpleProperty<const Color&> AdaptiveProperty;
            AdaptiveProperty* p = static_cast<AdaptiveProperty*>(property);

            Tokenizer tok(value.c_str(), ',');
            Tokenizer::iterator i = tok.begin();
            Color v;
            if (i != tok.end())
            {
                v.r_ = atoi((i++)->c_str());
                if (i != tok.end())
                {
                    v.g_ = atoi((i++)->c_str());
                    if (i != tok.end())
                    {
                        v.b_ = atoi((i++)->c_str());
                        if (i != tok.end())
                            v.a_ = atoi((i++)->c_str());
                    }
                }
            }

            p->set(object, v);
        }

    private:
        String str_;
    };
}

#endif //BZMAG_CORE_UTILITY_PRIMITIVE_STRINGCONVERTER_H