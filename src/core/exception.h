#ifndef BZMAG_CORE_UTILITY_EXCEPTION_H
#define BZMAG_CORE_UTILITY_EXCEPTION_H
/**
    @ingroup bzmagCoreException
    @class bzmag::Exception
    @brief 
*/

#pragma warning(disable: 4275)

#include <list>
#include <exception>
#include "platform.h"
#include "define.h"
#include "string.h"
#include "singleton.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Exception : public std::exception
    {
    public:
        Exception(
            int error_code, int line, const String& file,
            const String& function, const String& description);
        virtual~Exception() throw() {}
        
        void setErrorCode(int error_code);
        void setLine(int line);
        void setFile(const String& file);
        void setFunction(const String& function);
        void setDescription(const String& description);

        int getErrorCode() const;
        int getLine() const;
        const String& getFile() const;
        const String& getFunction() const;
        const String& getDescription() const;
        String getFullDescription() const;

    private:
        int errorCode_;
        int line_;
        String file_;
        String function_;
        String description_;
    };
    
    class BZMAG_LIBRARY_API ExceptionHandler
    {
    public:
        virtual void handle(const Exception& e)=0;
    };

    class ThrowExceptionHandler : public ExceptionHandler
    {
    public:
        virtual void handle(const Exception& e);
    };


    class BZMAG_LIBRARY_API ExceptionManager : public Singleton<ExceptionManager>
    {
    public:
        ExceptionManager()
        {

        }
        virtual~ExceptionManager()
        {
            for (ExceptionHandlers::iterator i = handlers_.begin();
                 i != handlers_.end(); ++i)
                delete *i;
        }

        void add(ExceptionHandler* handler)
        {
            handlers_.push_back(handler);
        }
        void handle(const Exception& e)
        {
            for (ExceptionHandlers::iterator i = handlers_.begin();
                 i != handlers_.end(); ++i)
                 (*i)->handle(e);
        }

    private:
        typedef std::list<ExceptionHandler*> ExceptionHandlers;

    private:
        ExceptionHandlers handlers_;
    };

#define BZMAG_THROW_EXCEPTION(err_code, desc) \
    ExceptionManager::instance()->handle(bzmag::Exception(\
        err_code, ___LINE___, ___FILE___, ___FUNCTION___, desc));

}

#endif // BZMAG_CORE_UTILITY_EXCEPTION_H
