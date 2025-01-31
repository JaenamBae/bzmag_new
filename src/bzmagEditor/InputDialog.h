#ifndef _INPUT_DIALOG_H
#define _INPUT_DIALOG_H

#include <QDialog>
#include <QPointF>

class QLineEdit;

class InputDialog : public QDialog {
    Q_OBJECT

public:
    explicit InputDialog(QWidget* parent = nullptr);

protected:
    bool checkPoint(const QString& text) const; // 점 파싱 함수
    bool checkValue(const std::string& text) const; // 점 파싱 함수
};

#endif // ARC_INPUT_DIALOG_H
