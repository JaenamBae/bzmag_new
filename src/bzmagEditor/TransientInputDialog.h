#ifndef TRANSIENT_INPUT_DIALOG_H
#define TRANSIENT_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QDialogButtonBox>

class TransientInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit TransientInputDialog(QWidget* parent = nullptr);

    QString getStopTime() const;  // StopTime 반환
    QString getTimeStep() const; // TimeStep 반환

private:
    QLineEdit* stop_time_edit_;  // StopTime 입력 필드
    QLineEdit* time_step_edit_; // TimeStep 입력 필드
};

#endif // TRANSIENT_INPUT_DIALOG_H
