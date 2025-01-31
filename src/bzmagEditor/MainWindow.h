#pragma once

#include <QMainWindow>
#include <QToolBar>
#include <QSplitter>
#include <QTreeView>
#include <QPlainTextEdit>
#include <QDockWidget>
#include <QQueue>
#include <mutex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "SignalManager.h"
#include "RenderView.h"
#include "PropertyView.h"
#include "QtWidgetSink.h"
#include "PythonScriptExecutor.h"
#include "PythonConsole.h"
#include "GmshHelper.h"
#include "GetDPHelper.h"
#include "ProjectLoader.h"
#include "ProgressWidget.h"
#include "TreeModel.h"
#include "ExpressionTableModel.h"

#include "Modeler.h"
#include "core/String.h"
#include "core/module.h"

class bzMagTreeView;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void connectModeler();
    void updateTitle(const QString& project_path);

protected:
    void initWindow();
    void buildMenu();
    void buildToolBar();
    void buildTemplateNode();
    void buildTree(const QString& root);
    bool saveProject(bool saveas, bool without_template = false);
    void openProject(const QString& filename);
    void test1();
    void test2();
    void test3();

protected:
    void closeEvent(QCloseEvent* event) override;

    void updateRecentProjects(const QString& project_path);
    QStringList loadRecentProjects();
    void saveRecentProjects(const QStringList& projects);
    void processNextPost();
    void processGlobalQuantity(const QString& file_name);

public slots:
    void onNewTriggered();     // File - Open 동작과 연결된 슬롯
    void onOpenTriggered();     // File - Open 동작과 연결된 슬롯
    void onSaveTriggered();     // File - Save 동작과 연결된 슬롯
    void onSaveAsTriggered();   // File - SaveAs 동작과 연결된 슬롯
    void onSaveWithoutTemplateTriggered();
    void populateRecentProjectsMenu();
    
    void onMeshTriggered();     // Analysis - Mesh 동작과 연결된 슬롯
    void onGetDPTriggered();    // Analysis - Generate getdp input
    void onStopTriggered();
    void onTaskStartTriggered(const QString& name);

    void onLockModelerTriggered(bool checked);  // Modeler - LockModeler
    void onConfigExpressionsTriggered();        // Modeler - ConfigExpression

    void onAboutTriggered();

    void onModelerQuery(const QString& query);
    void onModelerLine();
    void onModelerArc();
    void onModelerCircle();
    void onModelerRectangle();
    void onModelerCS();
    void onModelerClone(bzmag::engine::GeomHeadNode* node);
    void onModelerMove(bzmag::engine::GeomHeadNode* node);
    void onModelerRotate(bzmag::engine::GeomHeadNode* node);
    void onModelerSplit(bzmag::engine::GeomHeadNode* node);
    void onModelerBoolean(bzmag::engine::GeomHeadNode* node, std::vector<bzmag::engine::GeomHeadNode*> tools, int type);
    void onModelerUndoBoolean(bzmag::engine::GeomBooleanNode* node);
    
    void onModelerTransient(bzmag::engine::SolutionSetup* node);
    void onModelerDelete(bzmag::Node* node);

    void onModelerMaterial();
    void onModelerWinding();
    void onModelerCoil(bzmag::engine::WindingNode* node);
    void onModelerFixedBC(const std::vector<bzmag::engine::GeomHeadNode*>& heads);
    void onModelerMasterBC(bzmag::engine::GeomHeadNode* node);
    void onModelerSlaveBC(bzmag::engine::GeomHeadNode* node);
    void onModelerMovingBand(bzmag::engine::GeomHeadNode* node);

protected slots:
    void onDrawLineTriggered();
    void onDrawArcTriggered();
    void onDrawCircleTriggered();
    void onDrawRectangleTriggered();
    void onCloneTriggered();

    void onMoveTriggered();
    void onRotateTriggered();
    void onDifferenceTriggered();
    void onUniteTriggered();
    void onIntersectTriggered();
    void onSplitTriggered();
    void onCreateCSTriggered();

protected:
    QToolBar* toolbar_ = nullptr;

    // 트리뷰, 렌더러, 프로퍼티뷰, 템플릿다이얼로그, 수식다이얼로그의 신호처리
    SignalManager signal_manager_;
    
    Modeler* modeler_ = nullptr;
    TreeModel* model_ = nullptr;
    ExpressionTableModel* expr_model_ = nullptr;

    bzMagTreeView* tree_view_ = nullptr;
    RenderView* render_view_ = nullptr;
    QPlainTextEdit* log_view_ = nullptr;
    PythonScriptExecutor* script_executor_ = nullptr;
    PythonConsole* python_console_ = nullptr;
    ProgressWidget* progress_view_ = nullptr;
    

    GmshHelper* gmsh_helper_ = nullptr;
    GetDPHelper* getdp_helper_ = nullptr;
    ProjectLoader* project_loader_ = nullptr;
    QThread* thread_ = nullptr;     // 메쉬작업을 위해 필요한 쓰레드
    std::mutex mutex_;

    PropertyView* property_view_ = nullptr;
    QDockWidget* log_dock_ = nullptr;
    QDockWidget* console_dock_ = nullptr;
    QDockWidget* progress_dock_ = nullptr;
    QDockWidget* property_dock_ = nullptr;

    QAction* toggle_log_action_ = nullptr;
    QAction* toggle_progress_action_ = nullptr;
    QAction* toggle_property_action_ = nullptr;
    QAction* lock_modeler_action_ = nullptr;

    QStringList recent_projects_;
    QMenu* recent_projects_menu_;

    QQueue<QString> post_processing_queue_;
    bool is_post_processing_ = false;
    bool is_postplot_processing_ = false;
    QThreadPool* thread_pool_ = nullptr;
    bool is_closing_ = false;
};
