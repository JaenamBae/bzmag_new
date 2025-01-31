#ifndef RECT_INPUT_DIALOG_H
#define RECT_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class RectInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit RectInputDialog(QWidget* parent = nullptr);
    QString getName() const;       // 이름 반환
    QString getStartPoint() const; // 시작점 반환
    QString getDx() const;          // 너비 반환
    QString getDy() const;          // 높이 반환

private:
    QLineEdit* name_edit_;         // 이름 입력 필드
    QLineEdit* start_point_edit_;  // 시작점 입력 필드 ("0,0" 형태)
    QLineEdit* dx_edit_;           // 너비 입력 필드
    QLineEdit* dy_edit_;           // 높이 입력 필드
};

#endif // RECT_INPUT_DIALOG_H
