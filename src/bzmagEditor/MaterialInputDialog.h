#ifndef MATERIAL_INPUT_DIALOG_H
#define MATERIAL_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include "core/dataset.h"

class MaterialInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit MaterialInputDialog(QWidget* parent = nullptr);

    QString getName() const;                  // 이름 반환
    bool isNonLinear() const;                 // NonLinear 체크 상태 반환
    bzmag::DataSet getBHCurve() const;        // BH 데이터 반환
    QString getPermeability() const;          // Permeability 값 반환
    QString getConductivity() const;          // Conductivity 값 반환
    QString getMagnetization() const;         // Magnetization 값 반환
    QString getMagnetizationDirection() const; // MagnetizationDirection 값 반환

private slots:
    void updatePermeabilityField();           // NonLinear 체크박스 상태에 따라 Permeability 필드 업데이트
    void updateMagnetizationDirectionField(); // Magnetization 값에 따라 MagnetizationDirection 필드 활성화/비활성화

private:
    QLineEdit* name_edit_;                    // 이름 입력 필드
    QCheckBox* nonlinear_checkbox_;           // NonLinear 체크박스
    QLineEdit* permeability_edit_;            // Permeability 입력 필드
    QPushButton* bh_curve_button_;            // BH-Curve 버튼
    QLineEdit* conductivity_edit_;            // Conductivity 입력 필드
    QLineEdit* magnetization_edit_;           // Magnetization 입력 필드
    QLineEdit* magnetization_direction_edit_; // MagnetizationDirection 입력 필드
    bzmag::DataSet dataset_;
};

#endif // MATERIAL_INPUT_DIALOG_H
