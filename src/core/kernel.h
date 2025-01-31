#ifndef BZMAG_CORE_KERNEL_KERNEL_H
#define BZMAG_CORE_KERNEL_KERNEL_H
/**
@ingroup bzmagCoreKernel
@class bzmag::Kernel
@brief
*/

#include <list>
#include <map>
#include <stack>
#include "platform.h"
#include "string.h"
#include "node.h"
#include "path.h"
#include "primitivetype.h"
#include "singleton.h"

namespace bzmag
{
    class Object;
    class Module;
    class BZMAG_LIBRARY_API Kernel : public Singleton<Kernel>
    {
    public:
        typedef std::map<String, Module*> Types;
        typedef std::map<String, Module*> Modules;
        typedef std::list<Module*> InitModuleOrder;

    public:
        Kernel();
        virtual~Kernel();

        Object* create(const String& type_name);
        Node* create(const String& type_name, const Path& path);
        Node* lookup(const Path& path);

        void pushCwn(Node* object);
        Node* popCwn();
        Node* getCwn();

        void addModule(Module* module);
        Module* findModule(const String& name);
        Module* findModuleByTypeName(const String& type_name) const;
        Modules::iterator firstModule();
        Modules::iterator lastModule();
        Modules::const_iterator firstModule() const;
        Modules::const_iterator lastModule() const;
        size_t getNumModules() const;

        void addType(const String& type_name, Module* module);
        const Type* findType(const String& type_name) const;

        Node* getRoot();

    private:
        typedef std::stack<Node::RefNode> Cwn;

    private:
        Node* create_node(const String& type_name, const String& name);

    private:
        Node::RefNode root_;
        Types types_;
        Modules modules_;
        InitModuleOrder initModuleOrder_;
        Cwn cwn_;

    };

#include "kernel.inl"

}

#endif // BZMAG_CORE_KERNEL_KERNEL_H
