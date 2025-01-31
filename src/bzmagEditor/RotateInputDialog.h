#ifndef ROTATE_INPUT_DIALOG_H
#define ROTATE_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class RotateInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit RotateInputDialog(QWidget* parent = nullptr);
    QString getAngle() const; // 회전 각도 반환

private:
    QLineEdit* angle_edit_;   // 회전 각도 입력 필드
};

#endif // ROTATE_INPUT_DIALOG_H
