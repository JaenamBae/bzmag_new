#ifndef LINE_INPUT_DIALOG_H
#define LINE_INPUT_DIALOG_H

#include "InputDialog.h"

class QLineEdit;

class LineInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit LineInputDialog(QWidget* parent = nullptr);
    QString getName() const;        // 이름 반환
    QString getStartPoint() const; // 시작점 반환
    QString getEndPoint() const;   // 끝점 반환

private:
    QLineEdit* name_edit_;         // 이름 입력 필드
    QLineEdit* start_point_edit_;  // 시작점 입력 필드 ("0,0" 형태)
    QLineEdit* end_point_edit_;    // 끝점 입력 필드 ("0,0" 형태)
};

#endif // LINE_INPUT_DIALOG_H
