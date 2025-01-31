#ifndef SUBTRACT_INPUT_DIALOG_H
#define SUBTRACT_INPUT_DIALOG_H

#include <QDialog>
#include <QStringList>
#include <QMap>
#include <vector>
#include "engine/GeomHeadNode.h"

class QListWidget;
class QPushButton;

class BooleanInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit BooleanInputDialog(bzmag::engine::GeomHeadNode* head, std::vector<bzmag::engine::GeomHeadNode*> tools, int type, QWidget* parent = nullptr);

    bzmag::engine::GeomBaseNode* getLeft() const;  // 왼쪽 리스트 반환
    std::vector<bzmag::engine::GeomBaseNode*> getRightList() const; // 오른쪽 리스트 반환

private slots:
    void moveToRight();
    void moveToLeft();

private:
    QListWidget* left_list_widget_;   // 왼쪽 리스트 위젯
    QListWidget* right_list_widget_;  // 오른쪽 리스트 위젯
    QPushButton* to_right_button_;    // 오른쪽으로 이동 버튼
    QPushButton* to_left_button_;     // 왼쪽으로 이동 버튼
    QMap<QString, bzmag::engine::GeomBaseNode*> string_to_node_;
};

#endif // SUBTRACT_INPUT_DIALOG_H
