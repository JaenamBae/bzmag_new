#include "node.h"

#include <algorithm>
#include "simplepropertybinder.h"
#include "nodeeventpublisher.h"

using namespace bzmag;

IMPLEMENT_CLASS(Node, Object);

//-----------------------------------------------------------------------------
Node::Node() : parent_(0)
{
    NodeEventPublisher::instance()->addRefSingleton();
}

//-----------------------------------------------------------------------------
Node::~Node()
{
    // clear children
    for (NodeIterator iter = firstChildNode();
        iter != lastChildNode(); ++iter)
    {
        Node* node = *iter;
        node->parent_ = 0;
    }
    children_.clear();

    if (parent_)
    {
        parent_->onRemoveNode(this);
        onDetachFrom(parent_);
        NodeEventPublisher::instance()->onDetachFrom(parent_, this);
    }
    parent_ = 0;


    NodeEventPublisher::instance()->releaseSingleton();
}

//-----------------------------------------------------------------------------
void Node::setName(const String& name)
{
    if (parent_)
    {
        // check node name where parent children nodes
        Node* node = parent_->findChild(name);
        if (node)
            return;

        Node* parent = parent_;
        detach();
        name_ = name;
        parent->attach(this);
    }
    else
        name_ = name;

    onSetName(name);
}

//-----------------------------------------------------------------------------
Path Node::getAbsolutePath()
{
    std::stack<Node*> s;
    Node* cur = this;
    while (cur)
    {
        if (cur->parent_)
            s.push(cur);
        cur = cur->parent_;
    }

    String path;
    while (!s.empty())
    {
        path += "/";
        path += s.top()->getName();
        s.pop();
    }

    return path;
}

//-----------------------------------------------------------------------------
// Modified : 2017.09.27 ; attach 이전에 detach 부터 시킴
void Node::attach(Node* child)
{
    // 만약 현재 자식노드들 중에 새로 attach하려는 node의 이름과 같은 노드가 있다면 실패
    if (findChild(child->getName())) return;

    child->detach();
    children_.push_back(Nodes::value_type(child));
    child->parent_ = this;

    onAddNode(child);
    child->onAttachTo(this);
    NodeEventPublisher::instance()->onAttachTo(this, child);
}

//-----------------------------------------------------------------------------
Node::NodeIterator Node::detach()
{
    Node* parent = parent_;
    parent_ = 0;
    if (parent)
    {
        //NodeIterator fi = std::find(parent->children_.begin(),
        //    parent->children_.end(), getName());

        String name = getName();
        NodeIterator fi = std::find_if(parent->children_.begin(), parent->children_.end(), [&name](const RefNode& node) {
            return node->getName() == name;
        });

        if (parent->children_.end() != fi)
        {
            NodeEventPublisher::instance()->onDetachFrom(parent, this);
            this->onDetachFrom(parent);
            parent->onRemoveNode(this);
            return parent->children_.erase(fi);
        }
    }
    return lastChildNode();
}

//-----------------------------------------------------------------------------
Node* Node::findChild(const String& name)
{
    //NodeIterator iter = std::find(children_.begin(),
    //    children_.end(), name);

    NodeIterator iter = std::find_if(children_.begin(), children_.end(), [&name](const RefNode& node) {
        return node->getName() == name;
    });

    if (children_.end() == iter)
        return nullptr;
    return *iter;
}

//-----------------------------------------------------------------------------
Node* Node::findPrevChild(const String& name)
{
    //NodeIterator iter = std::find(children_.begin(),
    //    children_.end(), name);

    NodeIterator iter = std::find_if(children_.begin(), children_.end(), [&name](const RefNode& node) {
        return node->getName() == name;
    });

    if (children_.end() == iter || children_.begin() == iter)
        return nullptr;

    return *(--iter);
}

//-----------------------------------------------------------------------------
Node* Node::findNextChild(const String& name)
{
    //NodeIterator iter = std::find(children_.begin(),
    //    children_.end(), name);

    NodeIterator iter = std::find_if(children_.begin(), children_.end(), [&name](const RefNode& node) {
        return node->getName() == name;
    });

    if (children_.end() == iter || children_.end() == ++iter)
        return nullptr;

    return *iter;
}

//-----------------------------------------------------------------------------
Node* Node::relativeNode(const Path& path)
{
    if (path == ".")
        return this;
    else if (path == "..")
        return getParent();
    else
    {
        Node* cur = this;
        for (Path::const_iterator token = path.begin(); token != path.end(); ++token)
        {
            Node* child = cur->findChild(*token);
            if (nullptr == child)
                return nullptr;
            cur = child;
        }
        return cur;
    }
}

//-----------------------------------------------------------------------------
void Node::onSetName(const String& name)
{
    // empty
}

//-----------------------------------------------------------------------------
void Node::onAttachTo(Node* parent)
{
    // empty
}

//-----------------------------------------------------------------------------
void Node::onDetachFrom(Node* parent)
{
    // empty
}

//-----------------------------------------------------------------------------
void Node::onAddNode(Node* node)
{
    // empty
}

//-----------------------------------------------------------------------------
void Node::onRemoveNode(Node* node)
{
    // empty
}

//-----------------------------------------------------------------------------
void bzmag::Node::releaseMe()
{
    // 나를 부모노드에서 분리
    detach();

    // 자식 노드 모두 클리어; Node 타입으로 클리어하면 모두 해제됨
    clearChildren<Node>();

    // 현재 노드 메모리 헤제
    Object::releaseMe();
}

//-----------------------------------------------------------------------------
bool Node::update()
{
    // empty
    return true;
}

//-----------------------------------------------------------------------------
void Node::bindProperty()
{
    BIND_PROPERTY(size_t, num_children, 0, &getNumChildren);
    BIND_PROPERTY(const String&, name, &setName, &getName);
}
