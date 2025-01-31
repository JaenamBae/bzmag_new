#include "SignalManager.h"
#include "MainWindow.h"
#include "ProgressWidget.h"
#include "RenderView.h"
#include "PropertyView.h"
#include "bzMagTreeView.h"
#include "TreeModel.h"
#include "ExpressionTableModel.h"
#include "TemplateNode.h"
#include "TemplateManager.h"
#include "PostPlotNode.h"
#include "PythonConsole.h"
#include "TemplatePropertyDialog.h"
#include "DataInputDialog.h"
#include "ChartDialog.h"
#include "ExpressionDialog.h"
#include "engine/DataSetNode.h"
#include "engine/ExpressionServer.h"
#include "core/kernel.h"
#include <QTreeView>

using namespace bzmag;
using namespace bzmag::engine;

void SignalManager::registerMainWindow(MainWindow* window)
{
    main_window_ = window;
}

void SignalManager::registerProgressWidget(ProgressWidget* widget)
{
    if (!widget) return;

    progress_widget_ = widget;
    if (main_window_) {
        connect(widget, &ProgressWidget::taskStarted, main_window_, &MainWindow::onTaskStartTriggered);
        connect(widget, &ProgressWidget::taskStopped, main_window_, &MainWindow::onStopTriggered);
    }
}

void SignalManager::registerRenderView(RenderView* view) {
    if (!view) return;

    render_view_ = view;
    connect(view, &RenderView::objectPicked, this, [=](Node* node, bool multi_selection) {
        onObjectPicked(node, multi_selection, view);
    });

    if (progress_widget_) {
        connect(render_view_, &RenderView::progressUpdated, progress_widget_, &ProgressWidget::updateProgress);
        connect(render_view_, &RenderView::labelUpdated, progress_widget_, &ProgressWidget::updateLabel);
    }

    connect(view, &RenderView::modelerQuery, main_window_, &MainWindow::onModelerQuery);
    connect(view, &RenderView::modelerMove, main_window_, &MainWindow::onModelerMove);
    connect(view, &RenderView::modelerRotate, main_window_, &MainWindow::onModelerRotate);
    connect(view, &RenderView::modelerSplit, main_window_, &MainWindow::onModelerSplit);
    connect(view, &RenderView::modelerClone, main_window_, &MainWindow::onModelerClone);
    connect(view, &RenderView::modelerBoolean, main_window_, &MainWindow::onModelerBoolean);
    connect(view, &RenderView::modelerDelete, main_window_, &MainWindow::onModelerDelete);
}

void SignalManager::registerTreeView(bzMagTreeView* view) {
    if (!view) return;

    tree_view_ = view;

    // 클릭 이벤트: '[*]' 설정
    connect(view, &bzMagTreeView::clicked, this, [=](const QModelIndex& index) {
        if (suppress_signal_ || !index.isValid()) return;

        // TreeModel을 가져옴
        TreeModel* model = dynamic_cast<TreeModel*>(view->model());
        if (!model) return;

        // index에서 Node를 가져옴
        Node* node = static_cast<Node*>(index.internalPointer());
        if (!node) return;

        // TreeModel에 선택된 항목 설정
        model->selectItem(node);
        model->layoutChanged();

        // 모델러에 선택된 CS 항목 설정
        Modeler* modeler = Modeler::instance();
        CSNode* cs = dynamic_cast<CSNode*>(node);
        if(cs) modeler->setCurrentCSNode(cs);

        // 선택된 모든 항목 가져오기
        QModelIndexList selectedIndexes = tree_view_->selectionModel()->selectedIndexes();
        if (selectedIndexes.size() > 1) {
            onObjectPicked(node, true, view);
        }
        else {
            onObjectPicked(node, false, view);
        }
    });

    connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
        [=](const QItemSelection& selected, const QItemSelection& deselected) {
        // 렌더뷰에서 모든 아이템 선택 해제
        onObjectPicked(nullptr, false, view);

        // 선택된 항목 처리
        //QModelIndexList selectedIndexes = selected.indexes();
        QModelIndexList selectedIndexes = tree_view_->selectionModel()->selectedIndexes();
        for (const QModelIndex& index : selectedIndexes) {
            // index에서 Node를 가져옴
            Node* node = static_cast<Node*>(index.internalPointer());
            if (!node) return;

            // 모델러에 선택된 CS 항목 설정
            Modeler* modeler = Modeler::instance();
            CSNode* cs = dynamic_cast<CSNode*>(node);
            if (cs) modeler->setCurrentCSNode(cs);

            // 선택된 항목이 1보다 클때 다중선택
            if (selectedIndexes.size() > 1) {
                onObjectPicked(node, true, view);
            }
            else {
                onObjectPicked(node, false, view);
            }
        }
    });

    connect(view, &bzMagTreeView::modelerQuery, main_window_, &MainWindow::onModelerQuery);
    connect(view, &bzMagTreeView::modelerMove, main_window_, &MainWindow::onModelerMove);
    connect(view, &bzMagTreeView::modelerRotate, main_window_, &MainWindow::onModelerRotate);
    connect(view, &bzMagTreeView::modelerSplit, main_window_, &MainWindow::onModelerSplit);
    connect(view, &bzMagTreeView::modelerClone, main_window_, &MainWindow::onModelerClone);
    connect(view, &bzMagTreeView::modelerBoolean, main_window_, &MainWindow::onModelerBoolean);
    connect(view, &bzMagTreeView::modelerUndoBoolean, main_window_, &MainWindow::onModelerUndoBoolean);
    connect(view, &bzMagTreeView::modelerCoil, main_window_, &MainWindow::onModelerCoil);
    connect(view, &bzMagTreeView::modelerTransient, main_window_, &MainWindow::onModelerTransient);
    connect(view, &bzMagTreeView::modelerDelete, main_window_, &MainWindow::onModelerDelete);
}

void SignalManager::registerPropertyView(PropertyView* view) {
    if (!view) return;

    property_view_ = view;
    connect(view, &PropertyView::propertyChanged, this, [=](Node* node) {
        onPropertyChanged(node);
    });
}

void SignalManager::registerExpressionTableModel(ExpressionTableModel* model)
{
    if (!model) return;

    expr_model_ = model;
    connect(model, &ExpressionTableModel::expressionUpdated, this, [=]() {
        Modeler* modeler = Modeler::instance();
        modeler->update();
        render_view_->requestUpdateAllObject(true);
    });
}

void SignalManager::registerTempleteManager(TemplateManager* manager)
{
    if (!manager) return;

    template_manager_.push_back(manager);
    connect(manager, &TemplateManager::templateUpdated, this, [=]() {
        Modeler* modeler = Modeler::instance();
        modeler->update();
        render_view_->requestUpdateAllObject(true);
    });
}

void SignalManager::registerPythonConsole(PythonConsole* console)
{
    if (!console) return;
    console_ = console;
    connect(console, &PythonConsole::executed, this, [=]() {
        if (tree_view_) {
            TreeModel* model = dynamic_cast<TreeModel*>(tree_view_->model());
            if (model) {
                model->layoutChanged();
            }
        }

        render_view_->updateLink();
        render_view_->update();
    });
}

void SignalManager::onObjectPicked(Node* node, bool multi_selection, QObject* source) {
    if (suppress_signal_) return;
    //if (!node) return;

    suppress_signal_ = true;

    // RenderView에서 온 이벤트라면 TreeView와 PropertyView를 업데이트
    if (source == render_view_) {
        if (tree_view_) {
            tree_view_->setCurrentIndex(QModelIndex());
            if (node) {
                TreeModel* model = dynamic_cast<TreeModel*>(tree_view_->model());
                QModelIndex foundIndex = model->findItemIndexById(QModelIndex(), node->getID());
                if (foundIndex.isValid()) {
                    tree_view_->setCurrentIndex(foundIndex);
                }
            }
        }

        if (property_view_) {
            property_view_->displayProperties(node);
        }
    }
    // TreeView에서 온 이벤트라면 RenderView와 PropertyView를 업데이트
    else if (source == tree_view_) {
        if (render_view_) {
            render_view_->selectObject(node, multi_selection);
            if (property_view_) {
                property_view_->displayProperties(node);
            }
        }

        TemplateNode* template_node = dynamic_cast<TemplateNode*>(node);
        if (template_node) {
            // 모델러에 현재 템플릿을 적용
            Modeler* modeler = Modeler::instance();
            modeler->setWorkingTemplate(template_node);
            modeler->setCurrentCSNode(template_node->getDefaultCSNode());

            // 트리모델 업데이트
            TreeModel* tree_model = dynamic_cast<TreeModel*>(tree_view_->model());
            if (tree_model) {
                tree_model->selectItem(template_node);
                tree_model->selectItem(template_node->getDefaultCSNode());
                tree_model->layoutChanged();
            }

            // 렌더뷰 업데이트
            render_view_->updateLink();
            render_view_->updatePostLink();
            render_view_->updateModelSize();
            render_view_->fitToWindow();

            // 메인윈도우 타이틀 적용
            main_window_->updateTitle(template_node->getProjectPath().c_str());

            // TemplatePropertyQDialog 팝업 표시
            if (modeler->getDefaultTemplate() != template_node) {
                TemplatePropertyDialog template_dialog(template_node, tree_view_->parentWidget());
                template_dialog.setReadOnly(modeler->isModelerLocked());
                template_dialog.exec();
            }
        }

        ExpressionServer* expr_server = dynamic_cast<ExpressionServer*>(node);
        if (expr_server) {
            Modeler* modeler = Modeler::instance();
            ExpressionDialog dialog(expr_model_, main_window_);
            dialog.setReadOnly(modeler->isModelerLocked());
            dialog.exec();
        }

        DataSetNode* dataset_node = dynamic_cast<DataSetNode*>(node);
        DataSet dataset;
        if (dataset_node) {
            // DataInputDialog 팝업 표시
            DataInputDialog* dataset_dialog = new DataInputDialog(dataset_node, tree_view_->parentWidget());
            dataset_dialog->resize(700, 400);
            dataset_dialog->exec();
        }

        PostPlotNode* post_plot = dynamic_cast<PostPlotNode*>(node);
        if (post_plot) {
            ChartDialog* dialog = new ChartDialog(post_plot, tree_view_->parentWidget());
            dialog->resize(700, 400);
            dialog->exec();
        }
    }

    suppress_signal_ = false;
}

void SignalManager::onPropertyChanged(Node* node) {
    if (suppress_signal_) return;
    if (!node) return;

    suppress_signal_ = true;

    // 트리뷰 업데이트
    TreeModel* model = dynamic_cast<TreeModel*>(tree_view_->model());
    emit model->layoutChanged();

    // 렌더뷰 업데이트
    if (dynamic_cast<GeomHeadNode*>(node)) {
        render_view_->requestUpdateHeadObject(node);
    }
    else if (dynamic_cast<GeomBaseNode*>(node)) {
        render_view_->requestUpdateAllObject(false);
    }
    else if (dynamic_cast<CSNode*>(node)) {
        render_view_->requestUpdateAllObject(false);
    }
    else if (dynamic_cast<Modeler*>(node)) {
        render_view_->requestUpdate();
    }
    else if (dynamic_cast<PostNode*>(node)) {
        render_view_->requestUpdate();
    }

    suppress_signal_ = false;
}

