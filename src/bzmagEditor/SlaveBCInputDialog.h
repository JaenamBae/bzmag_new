#ifndef SLAVE_BC_INPUT_DIALOG_H
#define SLAVE_BC_INPUT_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QMap>
#include "engine/MasterPeriodicBCNode.h"

class SlaveBCInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit SlaveBCInputDialog(QWidget* parent = nullptr);

    QString getName() const;       // 이름 반환
    bool getDirection() const;    // 방향 반환 (bool 타입)
    bzmag::engine::MasterPeriodicBCNode* getMaster() const;    // Master 선택 반환
    QString getMatch() const;

private:
    QLineEdit* name_edit_;        // 이름 입력 필드
    QCheckBox* direction_check_;  // Direction 체크박스
    QComboBox* match_combo_;     // Master 드랍다운 리스트
    QComboBox* master_combo_;     // Master 드랍다운 리스트
    QMap<QString, bzmag::engine::MasterPeriodicBCNode*> string_to_node_;
};

#endif // SLAVE_BC_INPUT_DIALOG_H