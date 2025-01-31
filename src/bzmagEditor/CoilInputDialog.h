#ifndef COIL_INPUT_DIALOG_H
#define COIL_INPUT_DIALOG_H

#include "InputDialog.h"
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QListWidget>
#include <QPushButton>
#include <QMap>
#include "engine/WindingNode.h"
#include "engine/GeomHeadNode.h"

class CoilInputDialog : public InputDialog {
    Q_OBJECT

public:
    explicit CoilInputDialog(bzmag::engine::WindingNode* winding, std::vector<bzmag::engine::GeomHeadNode*> heads, QWidget* parent = nullptr);

    QString getName() const;
    QString getDirection() const;
    QString getTurns() const;
    std::vector<bzmag::engine::GeomHeadNode*> getSelectedNodes() const;

    void setStringList(const QStringList& strings);

private:
    QLineEdit* name_edit_;
    QComboBox* direction_combo_;
    QLineEdit* turns_edit_;

    QListWidget* available_list_widget_;
    QListWidget* selected_list_widget_;

    QPushButton* move_down_button_;
    QPushButton* move_up_button_;

    QDialogButtonBox* button_box_; // OK 및 Cancel 버튼

    void setupConnections();
    QMap<QString, bzmag::engine::GeomHeadNode*> string_to_node_;
};

#endif // COIL_INPUT_DIALOG_H
