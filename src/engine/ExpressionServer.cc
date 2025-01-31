#include "ExpressionServer.h"
#include "Expression.h"
#include "core/string.h"
#include "core/autoreleasepool.h"
#include "spdlog/spdlog.h"

using namespace bzmag;
using namespace bzmag::engine;


IMPLEMENT_CLASS(ExpressionServer, Node);

//----------------------------------------------------------------------------
ExpressionServer::ExpressionServer(void)
{
    //m_Parser.SetCaseSensitive(false);
    m_Parser.DefineNameChars("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$");
    /*m_Parser.DefineVar("$TIME", &time_);
    m_Parser.DefineVar("$X", &x_);
    m_Parser.DefineVar("$Y", &y_);
    m_Parser.DefineVar("$Z", &z_);*/
}

//----------------------------------------------------------------------------
ExpressionServer::~ExpressionServer(void)
{

}

//----------------------------------------------------------------------------
bool ExpressionServer::addExpression(Expression* expr)
{
    if (findExpression(expr->getKey()))
        return false;

    m_Expressions[expr->getKey()] = expr;

    // 로그 남기기
    if (!expr->isHidden() && expr->isUserDefined()) {
        //spdlog::info("The expression with key '{}' is successfully registered.", expr->getKey().c_str());
    }

    return true;
}

//----------------------------------------------------------------------------
bool ExpressionServer::removeExpression(Expression* expr)
{
    // 로그 남기기
    if (!expr->isHidden() && expr->isUserDefined()) {
        //spdlog::info("The expression with key '{}' is successfully removed.", expr->getKey().c_str());
    }

    // 서버에서 삭제
    m_Expressions.erase(expr->getKey());

    return true;
}

//----------------------------------------------------------------------------
Expression* ExpressionServer::findExpression(const String& key)
{
    ExprIter it = m_Expressions.find(key);
    if (it != m_Expressions.end())
        return it->second;

    return 0;
}

//----------------------------------------------------------------------------
bool ExpressionServer::checkConsistancy(const String& sKey, const String& sVal)
{
    // 파서에 표현식을 설정하고,
    // 표현식에 사용된 변수들을 뽑아낸다
    mu::string_type ssKey(sKey);
    mu::string_type ssVal(sVal);

    // 표현식을 파싱해서 정상적인 수식인지 판단
    m_Parser.SetExpr(ssVal);
    mu::varmap_type variables;
    try {
        variables = m_Parser.GetUsedVar();
    }
    catch (...) {
        return false;
    }

    // 표현식에 사용된 키가 현재 주어진 표현식의 키와 동일하면 순환참조이다
    mu::varmap_type::const_iterator item;
    for (item = variables.begin(); item != variables.end(); ++item)
    {
        mu::string_type var_name = item->first;
        if (var_name == ssKey)
            return false;

        // 표현식에 사용된 키의 표현식 중 하나라도 등록되어 있지 않거나, 
        // 현재 주어진 키와 동일하면 순환참조다
        String nextKey(var_name);

        Expression* pRefExpr = findExpression(nextKey);

        if (pRefExpr != 0) {
            if (!checkConsistancy(sKey, pRefExpr->getExpression())) {
                return false;
            }
        }

        // 주어진 표현식에 사용된 변수가 등록된 변수가 아니라면 잘못된 식이다
        else
            return false;
    }

    return true;
}

//----------------------------------------------------------------------------
bool ExpressionServer::checkValidKey(const String& sKey)
{
    // 표현식을 파싱해서 정상적인 수식인지 판단
    mu::string_type key(sKey.c_str());
    m_Parser.SetExpr(key);
    mu::varmap_type variables;
    try {
        variables = m_Parser.GetUsedVar();
    }
    catch (...) {
        return false;
    }

    if (variables.size() != 1) return false;
    return true;
}

void ExpressionServer::defineVar(const mu::string_type& name, mu::value_type& value)
{
    m_Parser.DefineVar(name, &value);
}

void ExpressionServer::setExpr(const mu::string_type& expr)
{
    std::lock_guard<std::mutex> lock(mtx_);
    m_Parser.SetExpr(expr);
}

const mu::varmap_type& ExpressionServer::getUsedVar() const
{
    return m_Parser.GetUsedVar();
}

mu::value_type ExpressionServer::eval()
{
    std::lock_guard<std::mutex> lock(mtx_);
    return m_Parser.Eval();
}
