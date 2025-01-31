#ifndef BZMAG_ENGINE_EXPRESSIONSERVER_H
#define BZMAG_ENGINE_EXPRESSIONSERVER_H

/*
Description : Expression Server
Last Update : 2017.10.31
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "platform.h"
#include "Expression.h"
#include "core/node.h"
#include "core/ref.h"
#include "core/singleton.h"
#include "core/singleton3.h"
#include <map>
#include <muParser.h>
#include <mutex>

namespace bzmag
{
    namespace engine
    {
        class ENGINELIBRARY_API ExpressionServer : public Node, public Singleton3<ExpressionServer>
        {
        public:
            typedef std::map<String, Expression*> Expressions;
            typedef Expressions::iterator ExprIter;
            typedef Expressions::const_iterator const_ExprIter;

            ExpressionServer(void);
            virtual ~ExpressionServer(void);
            DECLARE_CLASS(ExpressionServer, Node);

            ExprIter firstExpression() { return m_Expressions.begin(); };
            ExprIter lastExpression() { return m_Expressions.end(); };
            const_ExprIter firstExpression() const { return m_Expressions.begin(); };
            const_ExprIter lastExpression() const { return m_Expressions.end(); };

            bool addExpression(Expression* expr);
            bool removeExpression(Expression* expr);

            Expression* findExpression(const String& key);

            bool checkConsistancy(const String& sKey, const String& sVal);
            bool checkValidKey(const String& sKey);

            void defineVar(const mu::string_type& name, mu::value_type& value);
            void setExpr(const mu::string_type& expr);
            const mu::varmap_type& getUsedVar() const;
            mu::value_type eval();

        private:
            Expressions m_Expressions;
            mu::Parser m_Parser;

            //mu::value_type time_, x_, y_, z_;
            std::mutex mtx_;
        };

    }
}

#endif //BZMAG_ENGINE_EXPRESSIONSERVER_H