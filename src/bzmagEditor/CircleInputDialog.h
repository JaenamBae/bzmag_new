#ifndef CIRCLE_INPUT_DIALOG_H
#define CIRCLE_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class CircleInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit CircleInputDialog(QWidget* parent = nullptr);
    QString getName() const;      // 이름 반환
    QString getCenter() const;  // 중심 좌표 반환
    QString getRadius() const;   // 반지름 반환

private:
    QLineEdit* name_edit_;        // 이름 입력 필드
    QLineEdit* center_edit_;    // 중심 입력 필드 ("0,0" 형태)
    QLineEdit* radius_edit_;    // 반지름 입력 필드
};

#endif // CIRCLE_INPUT_DIALOG_H
