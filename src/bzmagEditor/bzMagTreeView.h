#pragma once

#include <QTreeView>
#include <QStandardItemModel>
#include <QMenu>
#include <QMouseEvent>
#include <QMessageBox>
#include "core/node.h"
#include "Modeler.h"

class bzMagTreeView : public QTreeView {
    Q_OBJECT

public:
    explicit bzMagTreeView(QWidget* parent = nullptr) : QTreeView(parent) {
        setContextMenuPolicy(Qt::CustomContextMenu); // 사용자 정의 팝업 메뉴 활성화
        setSelectionMode(QAbstractItemView::ExtendedSelection); // 다중 선택 활성화
        connect(this, &QTreeView::customContextMenuRequested, this, &bzMagTreeView::showContextMenu);
    }

private slots:
    void showContextMenu(const QPoint& pos) {
        // 클릭된 위치의 QModelIndex 가져오기
        QModelIndex index = indexAt(pos);
        if (!index.isValid()) {
            return; // 유효하지 않은 위치일 경우 반환
        }

        // 모델러
        Modeler* modeler = Modeler::instance();

        // 아이템의 속성 확인
        bzmag::Node* node = static_cast<bzmag::Node*>(index.internalPointer());
        
        // 팝업 메뉴 생성
        QMenu contextMenu(this);

        if (node == modeler->getWorkingGeomRootNode()) {
            contextMenu.addAction("New Line", this, [this]() {
                emit modelerQuery("Line");
            });
            contextMenu.addAction("New Arc", this, [this]() {
                emit modelerQuery("Arc");
            });
            contextMenu.addAction("New Circle", this, [this]() {
                emit modelerQuery("Circle");
            });
            contextMenu.addAction("New Rectangle", this, [this]() {
                emit modelerQuery("Rectangle");
            });
        }
        else if (node == modeler->getWorkingCoordRootNode()) {
            contextMenu.addAction("New Coordinate System", this, [this]() {
                emit modelerQuery("CS");
            });
        }
        else if (node == modeler->getWorkingMaterialRootNode()) {
            contextMenu.addAction("New Material", this, [this]() {
                emit modelerQuery("Material");
            });
        }
        else if (node == modeler->getWorkingBCRootNode()) {
            contextMenu.addAction("New Fixed BC", this, [this]() {
                emit modelerQuery("FixedBC");
            });
            contextMenu.addAction("New Master BC", this, [this]() {
                emit modelerQuery("MasterBC");
            });
            contextMenu.addAction("New Slave BC", this, [this]() {
                emit modelerQuery("SlaveBC");
            });
            contextMenu.addAction("New Moving Band", this, [this]() {
                emit modelerQuery("MovingBand");
            });
        }
        else if (node == modeler->getWorkingExcitationRootNode()) {
            contextMenu.addAction("New Winding", this, [this]() {
                emit modelerQuery("Winding");
            });
        }
        else if (node->getType()->getName() == "GeomHeadNode") {
            // 트리모델에서 선택된 노드를 가져온다
            QModelIndexList selected_indexes = this->selectionModel()->selectedIndexes();

            bzmag::engine::GeomHeadNode* from = nullptr;
            std::vector<bzmag::engine::GeomHeadNode*> tools;
            for (auto& index : selected_indexes) {
                bzmag::Node* n = static_cast<bzmag::Node*>(index.internalPointer());
                bzmag::engine::GeomHeadNode* head = dynamic_cast<bzmag::engine::GeomHeadNode*>(n);
                if (!head) continue;

                if (!from) from = head;
                else {
                    tools.push_back(head);
                }
            }
            if (tools.size() > 0) {
                contextMenu.addAction("Subtract", this, [this, from, tools]() {
                    emit modelerBoolean(from, tools, 1);
                });
                contextMenu.addAction("Unite", this, [this, from, tools]() {
                    emit modelerBoolean(from, tools, 2);
                });
                contextMenu.addAction("Intersect", this, [this, from, tools]() {
                    emit modelerBoolean(from, tools, 3);
                });
                contextMenu.addSeparator();
                // "Assign BC" 하위 메뉴 생성
                QMenu* assignBCMenu = contextMenu.addMenu("Assign to BC");
                assignBCMenu->addAction("Assign to Fixed BC", this, [this]() {
                    emit modelerQuery("FixedBC");
                });
                assignBCMenu->addAction("Assign to Master BC", this, [this]() {
                    emit modelerQuery("MasterBC");
                });
                assignBCMenu->addAction("Assign to Slave BC", this, [this]() {
                    emit modelerQuery("SlaveBC");
                });
            }
            else if (from) {
                bzmag::engine::GeomHeadNode* head = dynamic_cast<bzmag::engine::GeomHeadNode*>(node);
                contextMenu.addAction("Move", this, [this, head]() {
                    emit modelerMove(head);
                });
                contextMenu.addAction("Rotate", this, [this, head]() {
                    emit modelerRotate(head);
                });
                contextMenu.addAction("Split", this, [this, head]() {
                    emit modelerSplit(head);
                });
                contextMenu.addAction("Clone", this, [this, head]() {
                    emit modelerClone(head);
                });
                contextMenu.addSeparator();

                // "Assign BC" 하위 메뉴 생성
                QMenu* assignBCMenu = contextMenu.addMenu("Assign to BC");
                assignBCMenu->addAction("Assign to Fixed BC", this, [this]() {
                    emit modelerQuery("FixedBC");
                });
                assignBCMenu->addAction("Assign to Master BC", this, [this]() {
                    emit modelerQuery("MasterBC");
                });
                assignBCMenu->addAction("Assign to Slave BC", this, [this]() {
                    emit modelerQuery("SlaveBC");
                });
                assignBCMenu->addAction("Assign to Moving Band", this, [this]() {
                    emit modelerQuery("MovingBand");
                });

                contextMenu.addSeparator();
                contextMenu.addAction("Delete", this, [this, head]() {
                    emit modelerDelete(head);
                });
            }
        }
        
        else if (dynamic_cast<bzmag::engine::GeomBooleanNode*>(node)) {
            contextMenu.addAction("Undo - Boolean", this, [this, node]() {
                bzmag::engine::GeomBooleanNode* boolean = dynamic_cast<bzmag::engine::GeomBooleanNode*>(node);
                emit modelerUndoBoolean(boolean);
            });
        }
        else if (node->getType()->getName() == "CSNode") {
            contextMenu.addAction("Delete", this, [this, node]() {
                emit modelerDelete(node);
            });
        }
        else if (node->getType()->getName() == "MaterialNode") {
            contextMenu.addAction("Delete", this, [this, node]() {
                emit modelerDelete(node);
            });
        }
        else if (dynamic_cast<bzmag::engine::BCNode*>(node)) {
            contextMenu.addAction("Delete", this, [this, node]() {
                emit modelerDelete(node);
            });
        }
        else if (node->getType()->getName() == "WindingNode") {
            bzmag::engine::WindingNode* winding = dynamic_cast<bzmag::engine::WindingNode*>(node);
            contextMenu.addAction("Add Coil", this, [this, winding]() {
                emit modelerCoil(winding);
            });
            contextMenu.addAction("Delete", this, [this, winding]() {
                emit modelerDelete(winding);
            });
        }
        else if (node->getType()->getName() == "CoilNode") {
            contextMenu.addAction("Delete", this, [this, node]() {
                emit modelerDelete(node);
            });
        }
        else if (node->getType()->getName() == "SolutionSetup") {
            bzmag::engine::SolutionSetup* setup = dynamic_cast<bzmag::engine::SolutionSetup*>(node);
            if (setup && setup->getNumChildren() == 0) {
                contextMenu.addAction("Transient Setup", this, [this, setup]() {
                    emit modelerTransient(setup);
                });
            }
        }
        else if (node->getType()->getName() == "Transient") {
            contextMenu.addAction("Delete", this, [this, node]() {
                emit modelerDelete(node);
            });
        }

        // 전역 좌표로 변환
        QPoint global_pos = mapToGlobal(pos);

        // 팝업 메뉴의 위치를 클릭 위치와 약간 조정
        int y_offset = 25; // 필요에 따라 조정
        QPoint adjusted_pos(global_pos.x(), global_pos.y() + y_offset);

        // 팝업 메뉴 표시
        contextMenu.exec(adjusted_pos);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        if (event->button() == Qt::RightButton) {
            // 오른쪽 마우스 클릭은 아이템 선택을 방지하고 팝업 메뉴만 표시
            emit customContextMenuRequested(event->pos());
            return; // 기본 동작 방지
        }
        // 왼쪽 마우스 클릭에 대해서만 기본 동작 수행
        QTreeView::mousePressEvent(event);
    }

    std::vector<bzmag::engine::GeomHeadNode*> getSelectedHeads() const
    {
        // 트리모델에서 선택된 노드를 가져온다
        QModelIndexList selected_indexes = this->selectionModel()->selectedIndexes();

        std::vector<bzmag::engine::GeomHeadNode*> heads;
        for (auto& index : selected_indexes) {
            bzmag::Node* n = static_cast<bzmag::Node*>(index.internalPointer());
            bzmag::engine::GeomHeadNode* head = dynamic_cast<bzmag::engine::GeomHeadNode*>(n);
            if (!head) continue;

            heads.push_back(head);
        }
        return heads;
    }

signals:
    void modelerQuery(const QString& query);
    void modelerMove(bzmag::engine::GeomHeadNode* node);
    void modelerRotate(bzmag::engine::GeomHeadNode* node);
    void modelerSplit(bzmag::engine::GeomHeadNode* node);
    void modelerClone(bzmag::engine::GeomHeadNode* node);
    void modelerUndoBoolean(bzmag::engine::GeomBooleanNode* node);
    void modelerCoil(bzmag::engine::WindingNode* node);
    void modelerTransient(bzmag::engine::SolutionSetup* node);
    void modelerBoolean(bzmag::engine::GeomHeadNode* node, std::vector<bzmag::engine::GeomHeadNode*> tools, int type);
    void modelerDelete(bzmag::Node* node);
};