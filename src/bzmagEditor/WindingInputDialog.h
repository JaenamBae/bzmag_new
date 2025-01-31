#ifndef WINDING_INPUT_DIALOG_H
#define WINDING_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QDialogButtonBox>

class WindingInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit WindingInputDialog(QWidget* parent = nullptr);

    QString getName() const;          // 이름 반환
    QString getCurrent() const;       // Current 값 반환
    QString getParallelBranch() const; // Parallel Branch 값 반환

private:
    QLineEdit* name_edit_;            // 이름 입력 필드
    QLineEdit* current_edit_;         // Current 입력 필드
    QLineEdit* parallel_branch_edit_; // Parallel Branch 입력 필드
};

#endif // WINDING_INPUT_DIALOG_H
