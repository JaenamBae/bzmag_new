#ifndef MASTER_BC_INPUT_DIALOG_H
#define MASTER_BC_INPUT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

class MasterBCInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit MasterBCInputDialog(QWidget* parent = nullptr);

    QString getName() const;       // 이름 반환
    bool getDirection() const;    // 방향 반환 (bool 타입)

private:
    QLineEdit* name_edit_;        // 이름 입력 필드
    QCheckBox* direction_check_;  // Direction 체크박스
};

#endif // MASTER_BC_INPUT_DIALOG_H