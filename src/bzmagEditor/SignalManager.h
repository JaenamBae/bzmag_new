#pragma once

#include <QObject>
#include <QModelIndex>
#include <QStandardItem>
#include <vector>
#include "core/node.h"

class RenderView;
class bzMagTreeView;
class PropertyView;
class ExpressionTableModel;
class TemplateManager;
class ProgressWidget;
class MainWindow;
class PythonConsole;

class SignalManager : public QObject {
    Q_OBJECT

public:
    void registerMainWindow(MainWindow* window);
    void registerProgressWidget(ProgressWidget* widget);
    void registerRenderView(RenderView* view);
    void registerTreeView(bzMagTreeView* view);
    void registerPropertyView(PropertyView* view);
    void registerExpressionTableModel(ExpressionTableModel* model);
    void registerTempleteManager(TemplateManager* model);
    void registerPythonConsole(PythonConsole* console);

    void onObjectPicked(bzmag::Node* node, bool multi_selection, QObject* source);
    void onPropertyChanged(bzmag::Node* node);

private:
    MainWindow* main_window_ = nullptr;
    ProgressWidget* progress_widget_ = nullptr;
    RenderView* render_view_ = nullptr;
    bzMagTreeView* tree_view_ = nullptr;
    PropertyView* property_view_ = nullptr;
    PythonConsole* console_ = nullptr;
    ExpressionTableModel* expr_model_ = nullptr;
    std::vector<TemplateManager*> template_manager_;

    QModelIndex previous_index_ = QModelIndex(); // 초기화
    bool suppress_signal_ = false;  // 신호 순환 방지 플래그
};
