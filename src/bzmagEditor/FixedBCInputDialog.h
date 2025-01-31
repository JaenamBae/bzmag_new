#ifndef FIXED_BC_INPUT_DIALOG_H
#define FIXED_BC_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QDialogButtonBox>

class FixedBCInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit FixedBCInputDialog(QWidget* parent = nullptr);

    QString getName() const;  // 이름 반환
    QString getValue() const; // Value 반환

private:
    QLineEdit* name_edit_;   // 이름 입력 필드
    QLineEdit* value_edit_;  // Value 입력 필드
};

#endif // FIXED_BC_INPUT_DIALOG_H
