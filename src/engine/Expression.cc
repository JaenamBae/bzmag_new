#include "Expression.h"
#include "ExpressionServer.h"
#include "core/simplepropertybinder.h"

#include <algorithm>

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(Expression, Object);


//----------------------------------------------------------------------------
Expression::Expression(void) :
    value_(0), hidden_(true), userdefined_(true), expression_("0")
{
    server_ = ExpressionServer::instance();
}

//----------------------------------------------------------------------------
Expression::~Expression(void)
{

}

//----------------------------------------------------------------------------
bool Expression::setKey(const String& sKey)
{
    // 이미 키가 설정되어 있다면 실패
    if (!key_.empty()) return false;

    // 키의 유효성 체크
    if (!server_->checkValidKey(sKey))
        return false;

    // 키를 설정함
    key_ = sKey;

    // 키가 처음 설정되는 경우 서버에 등록해 바로 활용 가능하게 한다
    return enable();
}

//----------------------------------------------------------------------------
void Expression::disable()
{
    server_->removeExpression(this);
}

//----------------------------------------------------------------------------
bool Expression::enable()
{
    if (server_->addExpression(this)) {

        // 파서에 변수를 (재) 등록함
        mu::string_type ssKey(key_);
        server_->defineVar(ssKey, value_);

        return true;
    }

    return false;
}

//----------------------------------------------------------------------------
bool Expression::setExpression(const String& sExpr)
{
    // 이미 설정된 값과 같다면 더이상 진행할 필요 없음
    if (expression_ == sExpr) return true;

    // 업데이트하고자 하는 표현식에 문제가 없는지 판단
    String sKey = getKey();
    if (!server_->checkConsistancy(sKey, sExpr)) return false;

    // 기존 식의 레퍼런스 디 카운팅 (내가 참조했던 변수들에 대하여...)
    RefExprIter it;
    for (it = firstUsedItem(); it != lastUsedItem(); ++it) {
        Expression* UsedExpr = *it;
        UsedExpr->deleteLinkedExpr(this);
    }
    usedItems_.clear();

    // 새로운 식의 레퍼런스 카운팅
    mu::string_type ssVal(sExpr);
    //m_Parser.SetExpr(ssVal);
    server_->setExpr(ssVal);
    const mu::varmap_type& variables = server_->getUsedVar();
    for (auto item = variables.begin(); item != variables.end(); ++item) {
        mu::string_type sKey = item->first;

        // 주어진 key에 대한 노드찾기
        String key(sKey.c_str());
        Expression* expr = server_->findExpression(key);

        if (expr) {
            // 나를 참조하고 있는 수식 업데이트
            expr->addLinkedExpr(this);

            // 내가 참조하고 있는 수식 업데이트
            addUsedExpr(expr);
        }
    }

    // 표현식 설정
    expression_ = sExpr;

    // 수식 계산
    return update();
}

//----------------------------------------------------------------------------
void Expression::addLinkedExpr(Expression* pExpr)
{
    ExprIter it
        = std::find(linkedItems_.begin(), linkedItems_.end(), pExpr->getKey());

    if (it == linkedItems_.end())
        linkedItems_.push_back(pExpr);
}

//----------------------------------------------------------------------------
void Expression::deleteLinkedExpr(Expression* pExpr)
{
    ExprIter it
        = std::find(linkedItems_.begin(), linkedItems_.end(), pExpr->getKey());

    if (it != linkedItems_.end())
        linkedItems_.erase(it);
}

//----------------------------------------------------------------------------
void Expression::addUsedExpr(Expression* pExpr)
{
    RefExprIter it
        = std::find(usedItems_.begin(), usedItems_.end(), pExpr->getKey());

    if (it == usedItems_.end())
        usedItems_.push_back(pExpr);
}

//----------------------------------------------------------------------------
void Expression::deleteUsedExpr(Expression* pExpr)
{
    RefExprIter it
        = std::find(usedItems_.begin(), usedItems_.end(), pExpr->getKey());

    if (it != usedItems_.end())
        usedItems_.erase(it);
}

//----------------------------------------------------------------------------
float64 Expression::eval()
{
    mu::string_type sKey(key_);

    //m_Parser.SetExpr(sKey);
    //float64 val = m_Parser.Eval();

    server_->setExpr(sKey);
    float64 val = server_->eval();
    if ((std::isnan(val) || std::isinf(val))) {
        val = 0;
    }

    return val;
}

//----------------------------------------------------------------------------
bool Expression::update()
{
    String equation("%s=%s", key_.c_str(), expression_.c_str());
    mu::string_type expr(equation);
    //m_Parser.SetExpr(expr);
    //m_Parser.Eval();

    server_->setExpr(expr);
    server_->eval();

    // 나를 참조하는 수식들의 재평가
    ExprIter it;
    for (it = firstLinkedItem(); it != lastLinkedItem(); ++it) {
        Expression* pCitingExpr = *it;
        pCitingExpr->update();
    }
    return true;
}

//----------------------------------------------------------------------------
void bzmag::engine::Expression::clearBelongings()
{
    usedItems_.clear();
    server_ = nullptr;
}

//----------------------------------------------------------------------------
std::vector<String> Expression::splitByTopLevelComma(const String& input)
{
    std::vector<String> result;
    int paren_depth = 0;
    std::string current;

    for (size_t i = 0; i < input.size(); ++i)
    {
        char c = input.at(i);

        // 괄호 깊이 추적
        if (c == '(')
        {
            paren_depth++;
        }
        else if (c == ')')
        {
            if (paren_depth > 0)
                paren_depth--;
        }
        // paren_depth==0 인 상태에서의 콤마만 Split 트리거로 사용
        if (c == ',' && paren_depth == 0)
        {
            // 지금까지 누적된 토큰을 하나의 식으로
            String temp(current);
            result.push_back(temp.trim());
            current.clear();
        }
        else
        {
            current.push_back(c);
        }
    }
    // 마지막 조각도 push
    if (!current.empty())
    {
        String temp(current);
        result.push_back(temp.trim());
    }
    return result;
}

//----------------------------------------------------------------------------
void Expression::bindProperty()
{
    BIND_PROPERTY(const String&, Key, 0, &getKey);
    BIND_PROPERTY(const String&, Expression, 0, &getExpression);
    BIND_PROPERTY(const String&, Comment, &setComment, &getComment);
    BIND_PROPERTY(bool, IsHidden, &setHidden, &isHidden);
    BIND_PROPERTY(bool, IsUserDefined, &setUserDefined, &isUserDefined);
    //    BIND_PROPERTY(float64, value, 0, &eval);
}

