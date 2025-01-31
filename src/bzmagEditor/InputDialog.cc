#include "InputDialog.h"
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include "engine/Expression.h"
#include "engine/ExpressionServer.h"

using namespace bzmag::engine;

InputDialog::InputDialog(QWidget* parent): QDialog(parent)
{

}

bool InputDialog::checkPoint(const QString& text) const {
    auto expr = Expression::splitByTopLevelComma(text.toStdString());
    if (expr.size() != 2) {
        return false;
    }

    if (!checkValue(expr[0].c_str()) ||
        !checkValue(expr[1].c_str()))
    {
        return false;
    }
    return true;
}

bool InputDialog::checkValue(const std::string& text) const
{
    ExpressionServer* server = ExpressionServer::instance();
    if (!server->checkConsistancy("", text))
    {
        return false;
    }
    return true;
}