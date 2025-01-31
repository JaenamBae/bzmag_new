#ifndef MOVING_BAND_INPUT_DIALOG_H
#define MOVING_BAND_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QDialogButtonBox>

class MovingBandInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit MovingBandInputDialog(QWidget* parent = nullptr);

    QString getName() const;             // 이름 반환
    QString getInitialPosition() const; // InitialPosition 반환
    QString getSpeed() const;            // Speed 반환

private:
    QLineEdit* name_edit_;               // 이름 입력 필드
    QLineEdit* initial_position_edit_;   // InitialPosition 입력 필드
    QLineEdit* speed_edit_;              // Speed 입력 필드
};

#endif // MOVING_BAND_INPUT_DIALOG_H
