#include "Expression.h"

//-----------------------------------------------------------------------------
inline bool operator == (const Expression::RefExpression& s1, const String& s2)
{
    Expression* e = s1;
    return (e->getKey() == s2);
}

//-----------------------------------------------------------------------------
inline bool operator == (const Expression* s1, const String& s2)
{
    return (s1->getKey() == s2);
}

//------------------------------------------------------------------------
inline void Expression::setComment(const String& comment)
{
    comment_ = comment;
}

//-----------------------------------------------------------------------------
inline void Expression::setHidden(bool bHidden)
{
    hidden_ = bHidden;
}

//-----------------------------------------------------------------------------
inline void Expression::setUserDefined(bool userdefned)
{
    userdefined_ = userdefned;
}

//-----------------------------------------------------------------------------
inline const String& Expression::getExpression() const
{
    return expression_;
}

//-----------------------------------------------------------------------------
inline const String& Expression::getKey() const
{
    return key_;
}

//-----------------------------------------------------------------------------
inline const String& Expression::getComment() const
{
    return comment_;
}

//-----------------------------------------------------------------------------
inline bool Expression::isHidden() const
{
    return hidden_;
}


//-----------------------------------------------------------------------------
inline bool Expression::isUserDefined() const
{
    return userdefined_;
}

inline bool bzmag::engine::Expression::isIndependantVariable() const
{
    if (linkedItems_.size() > 0)
        return false;

    return true;
}

//-----------------------------------------------------------------------------
inline uint64 Expression::getLinkCount() const
{
    return linkedItems_.size();
}

//-----------------------------------------------------------------------------
inline Expression::ExprIter Expression::firstLinkedItem()
{
    return linkedItems_.begin();
}

//-----------------------------------------------------------------------------
inline Expression::ExprIter Expression::lastLinkedItem()
{
    return linkedItems_.end();
}

//-----------------------------------------------------------------------------		
inline Expression::RefExprIter Expression::firstUsedItem()
{
    return usedItems_.begin();
}

//-----------------------------------------------------------------------------
inline Expression::RefExprIter Expression::lastUsedItem()
{
    return usedItems_.end();
}
