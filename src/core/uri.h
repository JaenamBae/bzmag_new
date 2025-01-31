#ifndef BZMAG_CORE_TYPE_URI_H
#define BZMAG_CORE_TYPE_URI_H
/**
@ingroup bzmagCoreType
@class bzmag::Uri
@brief Universal Resource Identifier

@section URIParts
- Protocol
- managed://
- file://
- http://
- IP
- Port
- Path
- Assembled example
- managed://local/world/heightmap.png
- file://c:/world/heightmap.png
- http://192.168.0.1:8000/world/heightmap.png
*/

#include "platform.h"
#include "string.h"

namespace bzmag
{
    class BZMAG_LIBRARY_API Uri
    {
    public:
        Uri();
        Uri(const char* uri);
        Uri(const String& uri);
        virtual ~Uri() {}

        void clear();
        bool empty() const;
        size_t size() const;

        void set(const char* uri);
        const String& get() const;
        const char* c_str() const;

        void setProtocol(const char* protocol);
        void setPackage(const char* managed);
        void setIp(const char* ip);
        void setPort(const char* port);
        void setPath(const char* path);

        const String& getProtocol() const;
        const String& getPackage() const;
        const String& getIp() const;
        const String& getPort() const;
        const String& getPath() const;

        String extractPath() const;
        String extractFile() const;
        String extractFileName() const;
        String extractExtension() const;

/*        String makePhysicalAbsolutePath() const;*/

        bool operator < (const Uri& rhs) const;
        bool operator == (const Uri& rhs) const;
        bool operator != (const Uri& rhs) const;
        operator const String& () const;

    private:
        void make_absolutepath(String* path) const;

    private:
        String uri_;
        String protocol_;
        String managed_;
        String ip_;
        String port_;
        String path_;
    };

#include "uri.inl"

}

#endif // BZMAG_CORE_TYPE_URI_H
