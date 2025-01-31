#ifndef SPLIT_INPUT_DIALOG_H
#define SPLIT_INPUT_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDialogButtonBox>

class SplitInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit SplitInputDialog(QWidget* parent = nullptr);

    QString getSelectedPlane() const;  // 선택된 평면 반환
    QString getSelectedDirection() const;  // 선택된 방향 반환 (Positive/Negative)

private:
    QComboBox* plane_combo_box_;     // 평면 선택 콤보박스
    QRadioButton* positive_button_; // Positive 라디오 버튼
    QRadioButton* negative_button_; // Negative 라디오 버튼
    QButtonGroup* direction_group_; // Positive/Negative 버튼 그룹
};

#endif // SPLIT_INPUT_DIALOG_H
