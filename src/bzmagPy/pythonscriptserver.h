#ifndef BZMAG_PYTHONPY_PYTHONSCRIPTSERVER_H
#define BZMAG_PYTHONPY_PYTHONSCRIPTSERVER_H
/**
    @ingroup bzmagPyScriptServer
    @class bzmag::bzmagPyScriptServer
    @brief
*/

#include "platform.h"
#include "core/singleton3.h"
#include "core/scriptserver.h"
#include "core/nodeeventsubscriber.h"

namespace bzmag
{
    class PYLIBLIBRARY_API bzmagPyScriptServer :
        public ScriptServer,
        public Singleton3<bzmagPyScriptServer>
    {
    public:
        bzmagPyScriptServer();
        virtual~bzmagPyScriptServer();
        DECLARE_CLASS(bzmagPyScriptServer, ScriptServer);

        static unsigned long long initialize();

        virtual bool run(const String& str, String* result);
        virtual bool call(
            const String& str,
            Parameter* parameter);
        virtual bool runFile(const Uri& uri, String* result);
    /*
    private:
        class NodeEventSubscriberImpl : public NodeEventSubscriber
        {
        public:
            void onAttachTo(Node* parent, Node* child);
            void onDetachFrom(Node* parent, Node* child);
        };

    public:
        static NodeEventSubscriberImpl nodeEventSubscriber_;
    */
    //public:
    //    static bool s_standAlone_;
    };
}

#endif // BZMAG_PYTHONPY_PYTHONSCRIPTSERVER_H

