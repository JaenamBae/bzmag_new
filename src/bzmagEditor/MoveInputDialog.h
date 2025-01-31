#ifndef MOVE_INPUT_DIALOG_H
#define MOVE_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class MoveInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit MoveInputDialog(QWidget* parent = nullptr);
    QString getDx() const; // dx 반환
    QString getDy() const; // dy 반환

private:
    QLineEdit* dx_edit_;   // dx 입력 필드
    QLineEdit* dy_edit_;   // dy 입력 필드
};

#endif // MOVE_INPUT_DIALOG_H
