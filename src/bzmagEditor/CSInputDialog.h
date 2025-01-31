#ifndef CS_INPUT_DIALOG_H
#define CS_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class CSInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit CSInputDialog(QWidget* parent = nullptr);

    QString getName() const;          // 이름 반환
    QString getCenter() const;        // 중심 반환 (x,y 형태)
    QString getAngle() const;         // 각도 반환

private:
    QLineEdit* name_edit_;       // 이름 입력 필드
    QLineEdit* center_edit_;     // 중심 입력 필드 (x,y 형태)
    QLineEdit* angle_edit_;      // 각도 입력 필드
};

#endif // CS_INPUT_DIALOG_H
