#include "object.h"
#include "define.h"
#include "simplepropertybinder.h"
#include "nodeeventpublisher.h"

using namespace bzmag;

IMPLEMENT_SUPERCLASS(Object);

//-----------------------------------------------------------------------------
uint32 Object::sID_ = 0;

//-----------------------------------------------------------------------------
Object::Object() : refCount_(0)
{
    ID_ = Object::sID_++;
    autorelease();
}

//-----------------------------------------------------------------------------
Object::~Object()
{

}

//-----------------------------------------------------------------------------
uint32 Object::release()
{
    if (--refCount_ == 0) {
        delete this;
        return 0;
    }
    return refCount_;
}

//-----------------------------------------------------------------------------
void Object::autorelease()
{
    AutoReleasePool::instance()->add(this);
}

//-----------------------------------------------------------------------------
uint32 Object::addRef()
{
    return ++refCount_;
}

//-----------------------------------------------------------------------------
uint32 Object::getRef() const
{
    return refCount_;
}

//-----------------------------------------------------------------------------
uint32 Object::getID() const
{
    return ID_;
}

//-----------------------------------------------------------------------------
bool Object::isKindOf(const String& name)
{
    return getType()->isKindOf(name);
}

//-----------------------------------------------------------------------------
bool Object::isKindOf(const Type* type)
{
    return getType()->isKindOf(type);
}

//-----------------------------------------------------------------------------
void Object::releaseMe()
{
    // 자신이 참조하는 오브젝트들에 대한 참조 해제
    clearBelongings();

    // AutoReleasePool 에서도 삭제; 레퍼런스 디카운트
    //AutoReleasePool::instance()->remove(this);
}

//-----------------------------------------------------------------------------
void Object::clearBelongings()
{
    // 현재 노드가 Ref<>로 참조하고 있는 오브젝트가 있다면 정리하는 함수임
    // 메모리 삭제 직전에 호출됨
}

//-----------------------------------------------------------------------------
void Object::bindProperty()
{
    BIND_PROPERTY(uint32, ObjectID, 0, &getID);
}

