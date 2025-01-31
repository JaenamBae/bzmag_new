#include "MainWindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QCoreApplication>
#include <QFileInfo>
#include <Qthread>
#include <QtConcurrent>
#include <regex>
#include "TemplateNode.h"
#include "bzMagTreeView.h"
#include "ExpressionDialog.h"
#include "PlatformDepandent.h"
#include "GmshObject.h"
#include "GetdpObject.h"
#include "LineInputDialog.h"
#include "ArcInputDialog.h"
#include "CircleInputDialog.h"
#include "RectInputDialog.h"
#include "MoveInputDialog.h"
#include "RotateInputDialog.h"
#include "BooleanInputDialog.h"
#include "SplitInputDialog.h"
#include "CSInputDialog.h"
#include "MaterialInputDialog.h"
#include "WindingInputDialog.h"
#include "FixedBCInputDialog.h"
#include "MasterBCInputDialog.h"
#include "SlaveBCInputDialog.h"
#include "MovingBandInputDialog.h"
#include "CoilInputDialog.h"
#include "TransientInputDialog.h"
#include "AboutDialog.h"

#include "core/kernel.h"
#include "engine/ExpressionServer.h"

using namespace bzmag;
using namespace bzmag::engine;

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    QString title = QString("%1 - bzMagEditor").arg("Untitled");
    setWindowTitle(title);

    initWindow();
    buildMenu();
    buildToolBar();
    buildTemplateNode();    // 트리 빌드보다 먼저 실행되어야 함
    buildTree("/");

    thread_pool_ = QThreadPool::globalInstance();
}

MainWindow::~MainWindow()
{
    if (model_) delete model_;
}

void MainWindow::connectModeler()
{
    // 렌더뷰 초기화
    // render_view_->setGeometryCSPath() 에서 render_view_->updateLink()가 이루어짐
    String geom_path = "/" + Modeler::getGeometryName();
    String cs_path = "/" + Modeler::getCoordinateSystemName();
    String post_path = "/" + Modeler::getPostName();
    render_view_->setGeometryCSPath(geom_path, cs_path, post_path);
    render_view_->updateModelSize();
    render_view_->fitToWindow();

    // 모델러 초기화
    modeler_ = Modeler::instance();
    modeler_->setCurrentCSNode(modeler_->getWorkingDefaultCSNode());

    // 트리모델 초기화
    model_->selectItem(modeler_->getWorkingTemplate());
    model_->selectItem(modeler_->getWorkingDefaultCSNode());
    model_->layoutChanged();

    // 트리뷰 선택상태 초기화
    tree_view_->setCurrentIndex(QModelIndex());

    // 프로퍼티뷰 초기화
    property_view_->displayProperties(nullptr);

    //test2();
}

void MainWindow::initWindow() {
    resize(800, 600);

    // 좌우 Splitter 구성
    QSplitter* splitter = new QSplitter(Qt::Horizontal, this);

    // 좌측 트리 뷰
    tree_view_ = new bzMagTreeView(splitter);
    splitter->addWidget(tree_view_);

    // 우측에 Renderer (Magnum OpenGL 뷰)
    render_view_ = new RenderView(splitter);
    splitter->addWidget(render_view_);

    // 초기 비율 설정
    splitter->setSizes(QList<int> { int(width() * 0.25), int(width() * 0.75) });
    splitter->setStretchFactor(0, 1); // 트리뷰는 비율 유지
    splitter->setStretchFactor(1, 3); // 렌더뷰는 더 넓게

    // Central Widget 설정
    setCentralWidget(splitter);

    // 하단에 로그 뷰
    log_dock_ = new QDockWidget(tr("Log"), this);
    log_view_ = new QPlainTextEdit(log_dock_);
    log_dock_->setWidget(log_view_);
    addDockWidget(Qt::BottomDockWidgetArea, log_dock_);

    // 하단에 파이썬 콘솔
    console_dock_ = new QDockWidget(tr("Python Console"), this);
    python_console_ = new PythonConsole(console_dock_);
    console_dock_->setWidget(python_console_);
    addDockWidget(Qt::BottomDockWidgetArea, console_dock_);

    // 하단에 프로그래스 뷰
    progress_dock_ = new QDockWidget(tr("Progress"), this);
    progress_view_ = new ProgressWidget(progress_dock_);
    progress_dock_->setWidget(progress_view_);
    addDockWidget(Qt::BottomDockWidgetArea, progress_dock_); 

    // 프로그래스 뷰를 로그+콘솔 탭 오른쪽에 배치
    splitDockWidget(log_dock_, progress_dock_, Qt::Horizontal);

    // 로그 뷰와 콘솔 뷰를 탭으로 묶기
    tabifyDockWidget(log_dock_, console_dock_);

    // 디폴트 탭을 로그 뷰로 설정
    log_dock_->raise();


    // 우측에 프로퍼티 뷰
    property_dock_ = new QDockWidget(tr("Properties"), this);
    property_view_ = new PropertyView(property_dock_);
    property_dock_->setWidget(property_view_);
    property_dock_->setBaseSize(200, height());
    property_dock_->setAllowedAreas(Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, property_dock_);

    // spdlog 초기화
    auto qt_sink = std::make_shared<QtWidgetSink<std::mutex>>(log_view_);
    auto logger = std::make_shared<spdlog::logger>("qt_logger", qt_sink);
    spdlog::set_default_logger(logger);
    spdlog::set_level(spdlog::level::info);

    // 로거 포맷 설정
    logger->set_pattern("[%H:%M:%S.%e] [%l] %v");

    // ExpressionTableModel 생성
    expr_model_ = new ExpressionTableModel(this);

    // ---------------------------------
    // 시그널 메니저에 렌더뷰, 프로퍼티뷰, 수식모델 등록
    signal_manager_.registerMainWindow(this);
    signal_manager_.registerProgressWidget(progress_view_);
    signal_manager_.registerRenderView(render_view_);
    signal_manager_.registerPropertyView(property_view_);
    signal_manager_.registerPythonConsole(python_console_);
    signal_manager_.registerExpressionTableModel(expr_model_);
}

void MainWindow::buildMenu()
{
    // 메뉴 바 생성
    QMenuBar* menu_bar = new QMenuBar(this);
    setMenuBar(menu_bar);

    // ---------------------------------
    // "File" 메뉴 추가
    QMenu* file_menu = menu_bar->addMenu(tr("File"));

    // "New" 액션 추가
    QAction* new_action = new QAction(tr("New"), this);
    file_menu->addAction(new_action);
    connect(new_action, &QAction::triggered, this, &MainWindow::onNewTriggered);

    // "Open" 액션 추가
    file_menu->addSeparator();
    QAction* open_action = new QAction(tr("Open"), this);
    file_menu->addAction(open_action);
    connect(open_action, &QAction::triggered, this, &MainWindow::onOpenTriggered);

    // "Open Recent" 액션 추가
    recent_projects_menu_ = file_menu->addMenu(tr("Recent Projects"));
    populateRecentProjectsMenu();

    // "Save" 액션 추가
    file_menu->addSeparator();
    QAction* save_action = new QAction(tr("Save"), this);
    file_menu->addAction(save_action);
    connect(save_action, &QAction::triggered, this, &MainWindow::onSaveTriggered);

    QAction* saveas_action = new QAction(tr("SaveAs"), this);
    file_menu->addAction(saveas_action);
    connect(saveas_action, &QAction::triggered, this, &MainWindow::onSaveAsTriggered);

    QAction* save_without_template_action = new QAction(tr("SaveWithoutTemplate"), this);
    file_menu->addAction(save_without_template_action);
    connect(save_without_template_action, &QAction::triggered, this, &MainWindow::onSaveWithoutTemplateTriggered);

    // 종료 액션 추가
    file_menu->addSeparator();
    file_menu->addAction(tr("Exit"), this, &MainWindow::close);

    // ---------------------------------
    // "View" 메뉴 추가
    QMenu* view_menu = menu_bar->addMenu(tr("View"));

    // 로그 뷰 토글 액션
    toggle_log_action_ = log_dock_->toggleViewAction();
    toggle_log_action_->setText(tr("Show Log"));
    view_menu->addAction(toggle_log_action_);

    // 프로그래스 뷰 토글 액션
    toggle_progress_action_ = progress_dock_->toggleViewAction();
    toggle_progress_action_->setText(tr("Show Progress"));
    view_menu->addAction(toggle_progress_action_);

    // 프로퍼티 뷰 토글 액션
    toggle_property_action_ = property_dock_->toggleViewAction();
    toggle_property_action_->setText(tr("Show Properties"));
    view_menu->addAction(toggle_property_action_);

    // ---------------------------------
    // "Modeler" 메뉴 추가
    QMenu* modeler_menu = menu_bar->addMenu(tr("Modeler"));

    // "Lock Modeler" 액션 추가
    lock_modeler_action_ = new QAction(tr("Lock Modeler"), this);
    lock_modeler_action_->setCheckable(true); // 체크 가능하게 설정
    modeler_menu->addAction(lock_modeler_action_);
    connect(lock_modeler_action_, &QAction::triggered, this, &MainWindow::onLockModelerTriggered);

    // "Config Expressions" 액션 추가
    QAction* config_expressions_action = new QAction(tr("Config Expressions"), this);
    modeler_menu->addAction(config_expressions_action);
    connect(config_expressions_action, &QAction::triggered, this, &MainWindow::onConfigExpressionsTriggered);


    // ---------------------------------
    // "Analysis" 메뉴 추가
    QMenu* analysis_menu = menu_bar->addMenu(tr("Analysis"));

    // "Mesh" 액션 추가
    QAction* mesh_action = new QAction(tr("Mesh Generation"), this);
    analysis_menu->addAction(mesh_action);
    connect(mesh_action, &QAction::triggered, this, &MainWindow::onMeshTriggered);

    // "Generate getdp input" 액션 추가
    QAction* getdp_action = new QAction(tr("Analysis"), this);
    analysis_menu->addAction(getdp_action);
    connect(getdp_action, &QAction::triggered, this, &MainWindow::onGetDPTriggered);

    // ---------------------------------
    // "Help" 메뉴 추가
    QMenu* help_menu = menu_bar->addMenu(tr("Help"));
    // "Mesh" 액션 추가
    QAction* about_action = new QAction(tr("About"), this);
    help_menu->addAction(about_action);
    connect(about_action, &QAction::triggered, this, &MainWindow::onAboutTriggered);
}

void MainWindow::buildToolBar()
{
    // 툴바 추가
    toolbar_ = addToolBar(tr("Drawing Tools"));
    toolbar_->setMovable(false); // 툴바 고정
    toolbar_->setIconSize(QSize(24, 24)); // 아이콘 크기 설정

    QAction* lineAction = new QAction(QIcon(":/icons/DrawLine.png"), tr("Draw Line"), this);
    QAction* arcAction = new QAction(QIcon(":/icons/DrawArc.png"), tr("Draw Arc"), this);
    QAction* circleAction = new QAction(QIcon(":/icons/DrawCircle.png"), tr("Draw Circle"), this);
    QAction* rectangleAction = new QAction(QIcon(":/icons/DrawRectangle.png"), tr("Draw Rectangle"), this);
    QAction* cloneAction = new QAction(QIcon(":/icons/Clone.png"), tr("Clone"), this);
    
    QAction* moveAction = new QAction(QIcon(":/icons/Move.png"), tr("Move Object"), this);
    QAction* rotateAction = new QAction(QIcon(":/icons/Rotate.png"), tr("Rotate Object"), this);
    QAction* splitAction = new QAction(QIcon(":/icons/Spilt.png"), tr("Split"), this);

    QAction* differenceAction = new QAction(QIcon(":/icons/Difference.png"), tr("Subtract"), this);
    QAction* uniteAction = new QAction(QIcon(":/icons/Unite.png"), tr("Unite"), this);
    QAction* intersectAction = new QAction(QIcon(":/icons/Intersect.png"), tr("Intersect"), this);
    
    QAction* CSAction = new QAction(QIcon(":/icons/CS.png"), tr("Coordinate System"), this);

    connect(lineAction, &QAction::triggered, this, &MainWindow::onDrawLineTriggered);
    connect(arcAction, &QAction::triggered, this, &MainWindow::onDrawArcTriggered);
    connect(circleAction, &QAction::triggered, this, &MainWindow::onDrawCircleTriggered);
    connect(rectangleAction, &QAction::triggered, this, &MainWindow::onDrawRectangleTriggered);
    connect(cloneAction, &QAction::triggered, this, &MainWindow::onCloneTriggered);

    connect(moveAction, &QAction::triggered, this, &MainWindow::onMoveTriggered);
    connect(rotateAction, &QAction::triggered, this, &MainWindow::onRotateTriggered);
    connect(splitAction, &QAction::triggered, this, &MainWindow::onSplitTriggered);

    connect(differenceAction, &QAction::triggered, this, &MainWindow::onDifferenceTriggered);
    connect(uniteAction, &QAction::triggered, this, &MainWindow::onUniteTriggered);
    connect(intersectAction, &QAction::triggered, this, &MainWindow::onIntersectTriggered);
    
    connect(CSAction, &QAction::triggered, this, &MainWindow::onCreateCSTriggered);

    toolbar_->addAction(lineAction);
    toolbar_->addAction(arcAction);
    toolbar_->addAction(circleAction);
    toolbar_->addAction(rectangleAction);
    toolbar_->addAction(cloneAction);
    toolbar_->addSeparator();

    toolbar_->addAction(moveAction);
    toolbar_->addAction(rotateAction);
    toolbar_->addAction(splitAction);
    toolbar_->addSeparator();

    toolbar_->addAction(differenceAction);
    toolbar_->addAction(uniteAction);
    toolbar_->addAction(intersectAction);
    toolbar_->addSeparator();

    toolbar_->addAction(CSAction);
}

void MainWindow::buildTemplateNode()
{
    Modeler* modeler = Modeler::instance();

    // PMSM 템플릿 노드 생성
    TemplateNode* template_node = new TemplateNode;
    template_node->setName("pmsm");
    modeler->attach(template_node);

    std::string template_path = getExecPath() + "/template/template_pmsm.json";

    // 템플릿을 로드할 때는 반드시 working Template를 자신으로 설정하고 로드 해야함
    TemplateNode* current_template = modeler->getWorkingTemplate();
    modeler->setWorkingTemplate(template_node);
    if (!template_node->loadTemplate(template_path)) {
        spdlog::error("Fail to load template!");
    }
    modeler->setWorkingTemplate(current_template);

    // loadTemplate() 함수 호출 후에 메니져가 생성된다
    signal_manager_.registerTempleteManager(template_node->getManager());
}


void MainWindow::buildTree(const QString& root_path)
{
    Kernel* kernel = Kernel::instance();
    Node* root_node = kernel->lookup(root_path.toStdString().c_str());
    if (root_node == nullptr)
        return;

    // Tree 모델 생성
    model_ = new TreeModel(root_node);

    // QTreeView에 모델 설정
    tree_view_->setModel(model_);
    signal_manager_.registerTreeView(tree_view_);
}

bool MainWindow::saveProject(bool saveas, bool without_template)
{
    QString filename(modeler_->getWorkingTemplate()->getProjectPath());

    // project_path_에 경로가 있는 경우 그대로 사용
    if (filename.isEmpty() || saveas){
        // 사용자 홈 디렉토리를 기본 경로로 설정
        QString default_path = QDir::homePath();

        // 저장 파일 대화 상자 열기
        filename = QFileDialog::getSaveFileName(this,
            tr("Save File"),
            default_path,
            tr("JSON Files (*.json);;All Files (*)"));

        if (filename.isEmpty()) {
            return false; // 사용자가 취소한 경우
        }

        // 확장자 확인 및 추가
        if (!filename.endsWith(".json", Qt::CaseInsensitive)) {
            filename += ".json";
        }
    }

    try {
        Kernel* kernel = Kernel::instance();
        ExpressionServer* expr_server = ExpressionServer::instance();

        BzmagFileManager result;

        if (!modeler_->saveCurrentData(result, without_template)) {
            spdlog::error("Fail to prepare current data");
            QMessageBox::critical(this, tr("Save Failed"), tr("Fail to prepare current data"));
            return false;
        }

        result.saveToFile(filename);
        spdlog::info("The file has been saved successfully. Path: {}", filename.toStdString());
        //QMessageBox::information(this, "Save Successful", "The file has been saved successfully.");

        // 템플릿의 project_path_에 새 경로 설정
        modeler_->getWorkingTemplate()->setProjectPath(filename.toStdString());

        // 윈도우 타이틀 업데이트
        updateTitle(filename);

        return true;
    }
    catch (const std::exception& ex) {
        // 오류 메시지 출력
        spdlog::error("Failed to save the file. Path: {}. Error: {}", filename.toStdString(), ex.what());
        QMessageBox::critical(this, tr("Save Failed"), tr("An error occurred while saving the file:\n%1").arg(ex.what()));

        return false;
    }
}

void MainWindow::openProject(const QString& filename)
{
    // 동기화
    std::lock_guard<std::mutex> lock(mutex_);

    // 실행 중인 작업이 있을 경우 바로 반환
    if (thread_ && thread_->isRunning()) {
        spdlog::warn("File is loading...");
        return;
    }

    // 기존 thread_와 project_loader_ 정리
    if (thread_) {
        thread_->quit();
        thread_->wait();
        delete thread_;
        thread_ = nullptr;
    }

    if (project_loader_) {
        delete project_loader_;
        project_loader_ = nullptr;
    }

    thread_ = new QThread(this);
    project_loader_ = new ProjectLoader(filename);
    project_loader_->moveToThread(thread_);

    // 쓰레드 시작 시 로드 작업 수행
    connect(thread_, &QThread::started, project_loader_, &ProjectLoader::load);

    // 진행 상태 업데이트
    connect(project_loader_, &ProjectLoader::taskStarted, progress_view_, &ProgressWidget::startTask);
    connect(project_loader_, &ProjectLoader::progressUpdated, progress_view_, &ProgressWidget::updateProgress);

    // 예외 발생 시 처리
    connect(project_loader_, &ProjectLoader::errorOccurred, this, [this](const QString& error) {
        QMessageBox::critical(this, tr("Open Failed"), tr("An error occurred: %1").arg(error));
    });

    // 로드 완료 시 후속 작업 수행
    connect(project_loader_, &ProjectLoader::taskCompleted, this, [this, filename](bool success) {
        if (success) {
            // 파일 로드 후 UI 업데이트
            progress_view_->updateLabel(tr("Loading completed!"));

            modeler_->setCurrentCSNode(modeler_->getWorkingDefaultCSNode());

            // 모델을 다시 로드함
            model_->reloadData();

            // 모델 초기화
            model_->selectItem(modeler_->getWorkingTemplate());
            model_->selectItem(modeler_->getWorkingDefaultCSNode());
            model_->layoutChanged();

            // 트리뷰 선택상태 초기화
            tree_view_->setCurrentIndex(QModelIndex());

            // 렌더뷰 초기화
            render_view_->updateLink();
            render_view_->updatePostLink();

            // 프로퍼티뷰 초기화
            property_view_->displayProperties(nullptr);

            // 파일 로드 성공 메시지
            spdlog::info("The file has been successfully loaded. Path: {}", filename.toStdString());
            QMessageBox::information(this, tr("Open Successful"), tr("The file has been successfully loaded."));

            // 템플릿의 project_path_에 새 경로 설정
            modeler_->getWorkingTemplate()->setProjectPath(filename.toStdString());
            updateTitle(filename);
        }
        else {
            QMessageBox::warning(this, tr("Open Failed"), tr("The file could not be loaded."));
        }
        thread_->quit();
    }, Qt::QueuedConnection);

    // 스레드 종료 및 메모리 정리
    connect(thread_, &QThread::finished, project_loader_, [=]() {
        project_loader_->deleteLater();
        project_loader_ = nullptr;
    });
    connect(thread_, &QThread::finished, thread_, [=]() {
        thread_->deleteLater();
        thread_ = nullptr;
    });

    thread_->start();
}

void MainWindow::updateTitle(const QString& project_path)
{
    if (project_path.isEmpty()) {
        QString title = QString("Untitled - bzMagEditor");
        setWindowTitle(title);
    }
    else {
        QFileInfo file_info(project_path);
        QString title = QString("%1 - bzMagEditor").arg(file_info.fileName());
        setWindowTitle(title);
    }
}

void MainWindow::onNewTriggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Save File", "Do you want to save the file?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Yes) {
        // 파일 저장 로직
        saveProject(true);
    }
    else if (reply == QMessageBox::Cancel) {
        // 취소: 아무 작업도 하지 않음
        return;
    }

    updateTitle("");
    modeler_->setWorkingTemplateAsDefault();
    modeler_->getDefaultTemplate()->initialize();
    model_->selectItem(modeler_->getWorkingTemplate());
    model_->selectItem(modeler_->getWorkingDefaultCSNode());
    model_->layoutChanged();
    render_view_->updateLink();
    render_view_->updatePostLink();
}

void MainWindow::onOpenTriggered()
{
    // 파일 선택 대화 상자 열기
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        QDir::homePath(),
        tr("JSON Files (*.json);;All Files (*)"));

    if (!filename.isEmpty()) {
        updateRecentProjects(filename);
        populateRecentProjectsMenu();

        tree_view_->selectionModel()->clearSelection();
        openProject(filename);
    }
}

void MainWindow::onSaveTriggered()
{
    saveProject(false);
}

void MainWindow::onSaveAsTriggered()
{
    saveProject(true);
}

void MainWindow::onSaveWithoutTemplateTriggered()
{
    saveProject(true, true);
}

void MainWindow::populateRecentProjectsMenu()
{
    recent_projects_menu_->clear(); // 기존 메뉴 초기화

    QStringList recent_projects = loadRecentProjects();
    for (const QString& project : recent_projects) {
        QAction* action = recent_projects_menu_->addAction(project);
        connect(action, &QAction::triggered, [this, project]() {
            tree_view_->selectionModel()->clearSelection();
            openProject(project);
        });
    }

    // 최근 프로젝트가 없을 경우 메뉴 비활성화
    recent_projects_menu_->setEnabled(!recent_projects.isEmpty());
}

void MainWindow::updateRecentProjects(const QString& project_path) {
    recent_projects_.removeAll(project_path); // 중복 제거
    recent_projects_.prepend(project_path);  // 맨 앞에 추가

    const int max_recent_projects = 10;    // 최대 10개 유지
    while (recent_projects_.size() > max_recent_projects) {
        recent_projects_.removeLast();
    }

    saveRecentProjects(recent_projects_);
}

QStringList MainWindow::loadRecentProjects()
{
    QSettings settings("DongyangMirae University", "bzMagEditor");
    return settings.value("recentProjects").toStringList();
}

void MainWindow::saveRecentProjects(const QStringList& projects)
{
    QSettings settings("DongyangMirae University", "bzMagEditor");
    settings.setValue("recentProjects", projects);
}

void MainWindow::processNextPost() {
    if (is_closing_ || is_post_processing_ || post_processing_queue_.isEmpty())
        return;

    is_post_processing_ = true;

    QString post_file;
    {
        std::lock_guard<std::mutex> lock(mutex_); // 동시 접근 방지
        while (post_processing_queue_.size() > 2) {
            post_processing_queue_.dequeue(); // 2개빼고 큐의 나머지 제거
        }
        post_file = post_processing_queue_.dequeue();
    }
    bzmag::String post_name(post_file.toStdWString().c_str());

    std::filesystem::path file_path(post_name.c_str());
    std::string file_name = file_path.stem().string();

    QtConcurrent::run(thread_pool_, [=]() {
        if (is_closing_) return;

        PostNode* post = modeler_->createPostObject("Post - " + file_name);
        if (post) {
            GetdpObject* getdp_obj = dynamic_cast<GetdpObject*>(post->getDrawingObject());
            if (!getdp_obj) {
                getdp_obj = render_view_->makeDrawingObject<GetdpObject>();
                post->setDrawingObject(getdp_obj);
            }
            getdp_obj->loadData(post_file);
        }

        QMetaObject::invokeMethod(this, [=]() {
            if (is_closing_) {
                return;
            }
            if (post) {
                model_->layoutChanged();
                render_view_->updatePostLink();
            }
            else {
                spdlog::error("Failed to create PostObject");
            }

            is_post_processing_ = false;
            if (!post_processing_queue_.isEmpty()) {
                processNextPost();
            }
        });
    });
}

void MainWindow::processGlobalQuantity(const QString& file_name)
{
    if (is_closing_ || is_postplot_processing_)
        return;

    is_postplot_processing_ = true;

    DataSet dataset;
    String temp(file_name.toStdWString().c_str());
    std::string filename(temp.c_str());
    std::ifstream file(filename, std::ios::in);    // 읽기전용으로 파일을 연다
    if (!file.is_open()) {
        spdlog::error("Unable to open file: " + filename);
    }
    // 버퍼 크기 설정
    const size_t buffer_size = 1 << 20; // 1MB
    std::vector<char> buffer(buffer_size);
    file.rdbuf()->pubsetbuf(buffer.data(), buffer_size);

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        DataPair data;
        if (iss >> data.x_ >> data.y_) {
            dataset.push_back(data);
        }
        else {
            spdlog::error("Error parsing line: " + line);
        }
    }


    std::filesystem::path file_path(filename);
    std::string name = file_path.stem().string();
    std::string group_name;
    std::string plot_name;
    size_t underscore_pos = name.find('_');

    if (underscore_pos != std::string::npos && underscore_pos > 0) {
        group_name = name.substr(0, underscore_pos);
        plot_name = name.substr(underscore_pos + 1);

        PostPlotNode* postplot = modeler_->createPostPlotObject("Global Quantity - " + group_name);
        if (postplot) {
            postplot->setPlottingData(plot_name, dataset);
        }
        else {
            spdlog::error("Failed to create PostObject");
        }
    }

    is_postplot_processing_ = false;
}

void MainWindow::onMeshTriggered()
{
    // 동기화
    std::lock_guard<std::mutex> lock(mutex_);

    // 실행 중인 작업이 있을 경우 바로 반환
    if (thread_ && thread_->isRunning()) {
        spdlog::warn("Mesh generation is already running.");
        return; 
    }

    // 기존 thread_와 gmsh_helper_ 정리
    if (thread_) {
        thread_->quit();
        thread_->wait();
        delete thread_;
        thread_ = nullptr;
    }

    if (gmsh_helper_) {
        gmsh_helper_->stop();
        delete gmsh_helper_;
        gmsh_helper_ = nullptr;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Save Project"), tr("Do you want to save to continue?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;
    if (!saveProject(false)) return;

    QFileInfo project(modeler_->getWorkingTemplate()->getProjectPath().c_str());
    QString result_path = project.absolutePath() + "/" + project.baseName() + ".results/";
    if (!QDir(result_path).exists() && !QDir().mkpath(result_path)) {
        spdlog::error("Failed to create directory!");
        return;
    }

    QString geo_file = result_path + project.baseName() + ".geo";
    QString msh_file = result_path + project.baseName() + ".msh";

    // 기존 파일 삭제
    if (QFile::exists(geo_file)) {
        QFile::remove(geo_file);
    }
    if (QFile::exists(msh_file)) {
        QFile::remove(msh_file);
    }

    // 새 스레드와 작업 생성
    thread_ = new QThread(this);
    gmsh_helper_ = new GmshHelper(geo_file, msh_file);
    gmsh_helper_->moveToThread(thread_);

    // 스레드 시작과 작업 실행
    connect(thread_, &QThread::started, gmsh_helper_, &GmshHelper::run);

    // 진행률과 완료 시그널 연결
    connect(gmsh_helper_, &GmshHelper::progressUpdated, progress_view_, &ProgressWidget::updateProgress);
    connect(gmsh_helper_, &GmshHelper::messageUpdated, progress_view_, &ProgressWidget::updateLabel);
    connect(gmsh_helper_, &GmshHelper::taskStarted, progress_view_, &ProgressWidget::startTask);
    connect(gmsh_helper_, &GmshHelper::taskCompleted, this, [=](bool success) {
        if (success) {
            spdlog::info("Mesh generation completed successfully!");
            progress_view_->clearTask();
            progress_view_->updateLabel("Mesh generation completed successfully!");
            
            // 후속 작업 시작
            PostNode* post = modeler_->createPostObject("Mesh - " + project.baseName().toStdString());
            if (post) {
                GmshObject* gmsh_obj = dynamic_cast<GmshObject*>(post->getDrawingObject());
                if (!gmsh_obj) {
                    gmsh_obj = render_view_->makeDrawingObject<GmshObject>();
                    post->setDrawingObject(gmsh_obj);
                }
                success = gmsh_obj->loadMesh(msh_file);
                if (success) {
                    model_->layoutChanged();
                    render_view_->updatePostLink();
                }
                else {
                    spdlog::error("Failed to load post file: {}", msh_file.toStdString());
                }
            }
            else {
                spdlog::error("Failed to create PostObject");
            }
        }
        else {
            progress_view_->clearTask();
            progress_view_->updateLabel("Mesh generation failed.");
            spdlog::error("Mesh generation failed.");
        }
        if (thread_) thread_->quit();
    }, Qt::QueuedConnection);

    // 스레드 종료 및 메모리 정리
    connect(thread_, &QThread::finished, gmsh_helper_, [=]() {
        gmsh_helper_->deleteLater();
        gmsh_helper_ = nullptr;
    });
    connect(thread_, &QThread::finished, thread_, [=]() {
        thread_->deleteLater();
        thread_ = nullptr;
    });


    progress_view_->startedMesh();
    thread_->start();
}

void MainWindow::onGetDPTriggered()
{
    // 동기화
    std::lock_guard<std::mutex> lock(mutex_);

    // 실행 중인 작업이 있을 경우 바로 반환
    if (thread_ && thread_->isRunning()) {
        spdlog::warn("Analysis is already running.");
        return;
    }

    // 기존 thread_와 getdp_helper_ 정리
    if (thread_) {
        thread_->quit();
        thread_->wait();
        delete thread_;
        thread_ = nullptr;
    }

    if (getdp_helper_) {
        getdp_helper_->stop();
        delete getdp_helper_;
        getdp_helper_ = nullptr;
    }

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Save Project"), tr("Do you want to save to continue?"),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) return;
    if (!saveProject(false)) return;

    QFileInfo project(modeler_->getWorkingTemplate()->getProjectPath().c_str());
    QString result_path = project.absolutePath() + "/" + project.baseName() + ".results/";
    if (!QDir(result_path).exists() && !QDir().mkpath(result_path)) {
        spdlog::error("Failed to create directory!");
        return;
    }

    // 모델러를 잠금
    lock_modeler_action_->setChecked(true);

    // 이벤트를 강제로 트리거
    emit lock_modeler_action_->toggled(true);

    QString pro_file = result_path + project.baseName() + ".pro";

    // 기존 파일 삭제
    if (QFile::exists(pro_file)) {
        QFile::remove(pro_file);
    }

    // 새 스레드와 작업 생성
    thread_ = new QThread(this);
    getdp_helper_ = new GetDPHelper(pro_file);
    getdp_helper_->moveToThread(thread_);

    // 스레드 시작과 작업 실행
    connect(thread_, &QThread::started, getdp_helper_, &GetDPHelper::run);

    // 진행률과 완료 시그널 연결
    connect(getdp_helper_, &GetDPHelper::progressUpdated, progress_view_, &ProgressWidget::updateProgress);
    connect(getdp_helper_, &GetDPHelper::messageUpdated, progress_view_, &ProgressWidget::updateLabel);
    connect(getdp_helper_, &GetDPHelper::taskStarted, progress_view_, &ProgressWidget::startTask);
    connect(getdp_helper_, &GetDPHelper::postLocalField, this, [=](QString post_file) {
        post_processing_queue_.enqueue(post_file);
        processNextPost();
    });
    connect(getdp_helper_, &GetDPHelper::postGlobalQuantity, this, [=](QString post_file) {
        processGlobalQuantity(post_file);
    });

    connect(getdp_helper_, &GetDPHelper::taskCompleted, this, [=](bool success) {
        if (success) {
            spdlog::info("Analysis completed successfully!");
            progress_view_->clearTask();
            progress_view_->updateLabel("Analysis completed successfully!");
        }
        else {
            progress_view_->clearTask();
            //progress_view_->updateLabel("Analysis failed.");
            spdlog::error("Analysis failed.");
        }
        if (thread_) thread_->quit();
    }, Qt::QueuedConnection);

    // 스레드 종료 및 메모리 정리
    connect(thread_, &QThread::finished, getdp_helper_, [=]() {
        getdp_helper_->deleteLater();
        getdp_helper_ = nullptr;
    });

    connect(thread_, &QThread::finished, thread_, [=]() {
        thread_->deleteLater();
        thread_ = nullptr;
    });

    progress_view_->startedAnalysis();
    thread_->start();
}

void MainWindow::onStopTriggered()
{
    if (getdp_helper_) {
        getdp_helper_->stop();
    }
    if (gmsh_helper_) {
        gmsh_helper_->stop();
    }
}

void MainWindow::onTaskStartTriggered(const QString& name)
{
    if (name == "Mesh") {
        onMeshTriggered();
    }
    else if (name == "Analysis") {
        onGetDPTriggered();
    }
}

void MainWindow::onLockModelerTriggered(bool checked)
{
    modeler_->lockModeler(checked);
    property_view_->setReadOnly(checked);
}

// ---------------------------------
// "Config Expressions" 클릭 시 호출되는 슬롯 구현
void MainWindow::onConfigExpressionsTriggered() {
    ExpressionDialog dialog(expr_model_, this);
    dialog.setReadOnly(modeler_->isModelerLocked());
    dialog.exec();
}

void MainWindow::onAboutTriggered()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onDrawLineTriggered()
{
    emit onModelerQuery("Line");
}

void MainWindow::onDrawArcTriggered()
{
    emit onModelerQuery("Arc");
}

void MainWindow::onDrawCircleTriggered()
{
    emit onModelerQuery("Circle");
}

void MainWindow::onDrawRectangleTriggered()
{
    emit onModelerQuery("Rectangle");
}

void MainWindow::onCloneTriggered()
{
    emit onModelerQuery("Clone");
}

void MainWindow::onMoveTriggered()
{
    emit onModelerQuery("Move");
}

void MainWindow::onRotateTriggered()
{
    emit onModelerQuery("Rotate");
}

void MainWindow::onDifferenceTriggered()
{
    emit onModelerQuery("Difference");
}

void MainWindow::onUniteTriggered()
{
    emit onModelerQuery("Unite");
}

void MainWindow::onIntersectTriggered()
{
    emit onModelerQuery("Intersect");
}

void MainWindow::onSplitTriggered()
{
    emit onModelerQuery("Split");
}

void MainWindow::onCreateCSTriggered()
{
    emit onModelerQuery("CS");
}

void MainWindow::onModelerQuery(const QString& query)
{
    QModelIndexList selected_indexes = tree_view_->selectionModel()->selectedIndexes();
    std::vector<GeomHeadNode*> heads;
    GeomHeadNode* from = nullptr;
    std::vector<GeomHeadNode*> tools;
    for (auto& index : selected_indexes) {
        Node* node = static_cast<Node*>(index.internalPointer());
        GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(node);
        if (!head) continue;

        heads.push_back(head);

        if (!from) from = head;
        else {
            tools.push_back(head);
        }
    }

    if (query == "Line") {
        onModelerLine();
    }
    else if (query == "Arc") {
        onModelerArc();
    }
    else if (query == "Circle") {
        onModelerCircle();
    }
    else if (query == "Rectangle") {
        onModelerRectangle();
    }
    else if (query == "Clone") {
        if (heads.size() == 1) {
            onModelerClone(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "One target node is required.");
        }
    }
    else if (query == "Move") {
        if (heads.size() == 1) {
            onModelerMove(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "One target node is required.");
        }
    }
    else if (query == "Rotate") {
        if (heads.size() == 1) {
            onModelerRotate(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "One target node is required.");
        }
    }
    else if (query == "Difference") {
        onModelerBoolean(from, tools, 1);
    }
    else if (query == "Unite") {
        onModelerBoolean(from, tools, 2);
    }
    else if (query == "Intersect") {
        onModelerBoolean(from, tools, 3);
    }
    else if (query == "Split") {
        if (heads.size() == 1) {
            onModelerSplit(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "One target node is required.");
        }
    }
    else if (query == "CS") {
        onModelerCS();
    }
    else if (query == "Material") {
        onModelerMaterial();
    }
    else if (query == "Winding") {
        onModelerWinding();
    }
    else if (query == "FixedBC") {
        if (heads.size() > 0) {
            onModelerFixedBC(heads);
        }
        else {
            QMessageBox::warning(nullptr, "Error", "Target node is required.");
        }
    }
    else if (query == "MasterBC") {
        if (heads.size() > 0) {
            onModelerMasterBC(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "Target node is required.");
        }
    }
    else if (query == "SlaveBC") {
        if (heads.size() > 0) {
            onModelerSlaveBC(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "Target node is required.");
        }
    }
    else if (query == "MovingBand") {
        if (heads.size() == 1) {
            onModelerMovingBand(heads.front());
        }
        else {
            QMessageBox::warning(nullptr, "Error", "Target node is required.");
        }
    }
}

void MainWindow::onModelerLine() {
    // 라인 그리기 로직 추가
    LineInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString start = dialog.getStartPoint();
        QString end = dialog.getEndPoint();

        modeler_->createLine(start.toStdString(), end.toStdString(), name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerArc() {
    // 라인 그리기 로직 추가
    ArcInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString center = dialog.getCenter();
        QString start = dialog.getStartPoint();
        QString end = dialog.getEndPoint();
        QString radius = dialog.getRadius();

        modeler_->createCurve(start.toStdString(), end.toStdString(), center.toStdString(), radius.toStdString(), name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerCircle() {
    // 원 그리기 로직 추가
    CircleInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString center = dialog.getCenter();
        QString radius = dialog.getRadius();

        modeler_->createCircle(center.toStdString(), radius.toStdString(), "", name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerRectangle() {
    // 사각형 그리기 로직 추가
    RectInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString start = dialog.getStartPoint();
        QString dx = dialog.getDx();
        QString dy = dialog.getDy();

        modeler_->createRectangle(start.toStdString(), dx.toStdString(), dy.toStdString(), name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerClone(GeomHeadNode* head)
{
    if (!head) {
        QMessageBox::warning(nullptr, "Error", "Target node is required.");
        return;
    }

    else {
        modeler_->clone(head->getLastNode(), head->getName());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerMove(GeomHeadNode* head)
{
    if (!head) {
        QMessageBox::warning(nullptr, "Error", "Target node is required.");
        return;
    }

    else {
        MoveInputDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString dx = dialog.getDx();
            QString dy = dialog.getDy();

            modeler_->move(head->getLastNode(), dx.toStdString(), dy.toStdString());
            model_->layoutChanged();
            render_view_->updateLink();
        }
    }
}

void MainWindow::onModelerRotate(GeomHeadNode* head)
{
    if (!head) {
        QMessageBox::warning(nullptr, "Error", "Target node is required.");
        return;
    }

    else {
        RotateInputDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString angle = dialog.getAngle();

            modeler_->rotate(head->getLastNode(), angle.toStdString());
            model_->layoutChanged();
            render_view_->updateLink();
        }
    }
}

void MainWindow::onModelerCS()
{
    CSInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString center = dialog.getCenter();
        QString angle = dialog.getAngle();
        modeler_->createCS(center.toStdString(), angle.toStdString(), name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}


void MainWindow::onModelerSplit(GeomHeadNode* head)
{
    if (!head) {
        QMessageBox::warning(nullptr, "Error", "Target node is required.");
        return;
    }

    else {
        SplitInputDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString splane = dialog.getSelectedPlane();
            QString sdir = dialog.getSelectedDirection();

            GeomSplitNode::SPLIT_PLANE plane;
            if (splane == "YZ-Plane") plane = GeomSplitNode::SPLIT_YZ;
            else if (splane == "ZX-Plane") plane = GeomSplitNode::SPLIT_ZX;

            bool dir;
            if (sdir == "Positive") dir = true;
            else dir = false;

            modeler_->split(head->getLastNode(), plane, dir, modeler_->getCurrentCSNode());
            model_->layoutChanged();
            render_view_->updateLink();
        }
    }
}

void MainWindow::onModelerBoolean(GeomHeadNode* head, std::vector<GeomHeadNode*> tools, int type)
{
    if (!head) {
        QMessageBox::warning(nullptr, "Error", "Target node is required.");
        return;
    }

    if (tools.size() == 0) {
        QMessageBox::warning(nullptr, "Error", "At least one operation node is required.");
        return;
    }

    BooleanInputDialog dialog(head, tools, type, this);
    if (dialog.exec() == QDialog::Accepted) {
        // 선택된 항목을 해제
        QItemSelectionModel* selection_model = tree_view_->selectionModel();
        if (selection_model) {
            selection_model->clearSelection();
        }

        GeomHeadNode* from = dynamic_cast<GeomHeadNode*>(dialog.getLeft());
        std::vector<GeomBaseNode*> tools = dialog.getRightList();

        for (auto& tool : tools) {
            //tool->detach();
            model_->onNodeRemoved(tool);
        }

        if (type == 1) {
            modeler_->booleanSubtract(from->getLastNode(), tools);
        }
        else if (type == 2) {
            modeler_->booleanUnite(from->getLastNode(), tools);
        }
        else if (type == 3) {
            modeler_->booleanIntersect(from->getLastNode(), tools);
        }

        model_->layoutChanged();
        
        render_view_->updateLink();
    }
}

void MainWindow::onModelerUndoBoolean(GeomBooleanNode* node)
{
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    while (node->getNumChildren() > 0) {
        auto child = node->firstChildNode();
        GeomHeadNode* head = child->get<GeomHeadNode*>();
        if (head) {
            geom_root->attach(head);
        }
        else {
            (*child)->releaseMe();
        }
    }
    node->detach();
    onModelerDelete(node);
}

void MainWindow::onModelerMaterial()
{
    MaterialInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();

        auto material = modeler_->createMaterial(name.toStdString(), dialog.isNonLinear());
        if (dialog.isNonLinear()) {
            DataSetNode* dataset_node = material->getDataSetNode();
            dataset_node->setDataset(dialog.getBHCurve());
            material->setPermeability("1");
        }
        else {
            QString permeability = dialog.getPermeability();
            material->setPermeability(permeability.toStdString());
        }

        QString conductivity = dialog.getConductivity();
        QString magnetization = dialog.getMagnetization();
        QString mag_dir = dialog.getMagnetizationDirection();
        material->setConductivity(conductivity.toStdString());
        material->setMagnetization(magnetization.toStdString());
        material->setDirectionOfMagnetization(mag_dir.toStdString());

        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerWinding()
{
    WindingInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString current = dialog.getCurrent();
        QString a = dialog.getParallelBranch();
        modeler_->createWinding(current.toStdString(), a.toStdString(), name.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerCoil(WindingNode* winding)
{
    if (!winding) {
        QMessageBox::warning(nullptr, "Error", "Target node(WindingNode) is required.");
        return;
    }

    std::vector<GeomHeadNode*> heads;
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = it->get<GeomHeadNode*>();
        if (head) {
            heads.push_back(head);
        }
    }
    
    Node* winding_root = modeler_->getWorkingExcitationRootNode();
    for (auto it = winding_root->firstChildNode(); it != winding_root->lastChildNode(); ++it) {
        Node* node = *it;
        for (auto koil = node->firstChildNode(); koil != node->lastChildNode(); ++koil) {
            CoilNode* coil = koil->get<CoilNode*>();
            if (coil) {
                GeomHeadNode* head = coil->getReferenceNode();
                heads.erase(std::remove(heads.begin(), heads.end(), head), heads.end());
            }
        }
    }

    CoilInputDialog dialog(winding, heads, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString direction = dialog.getDirection();
        QString turns = dialog.getTurns();
        std::vector<bzmag::engine::GeomHeadNode*> nodes = dialog.getSelectedNodes();
        int i = 0;

        for (auto node : nodes) {
            std::string coil_name = name.toStdString();
            if (nodes.size() > 1) {
                coil_name = name.toStdString() + "_" + std::to_string(i);
            }
            modeler_->createCoil(turns.toStdString(), (direction == "Positive"), coil_name, winding);
            i++;
        }
        
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerFixedBC(const std::vector<GeomHeadNode*>& heads)
{
    QModelIndexList selected_indexes = tree_view_->selectionModel()->selectedIndexes();

    FixedBCInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString value = dialog.getValue();
        auto fix_bc = modeler_->createDirichletBC(value.toStdString(), name.toStdString());
        int i = 1;
        for (auto head : heads) {
            if (head) {
                std::string name("Dirichlet");
                if (selected_indexes.size() > 1) {
                    name = name + "_" + std::to_string(i);
                }
                modeler_->createBCObject(head, name, fix_bc);
                i++;
            }
        }
        if (i == 1) {
            QMessageBox::warning(nullptr, "Error", "Head node shoud be selected.");
            return;
        }
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerMasterBC(GeomHeadNode* head)
{
    MasterBCInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        bool dir = dialog.getDirection();
        auto master_bc = modeler_->createMasterBC(dir, name.toStdString());
        modeler_->createBCObject(head, "Master", master_bc);
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerSlaveBC(GeomHeadNode* head)
{
    SlaveBCInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        bool dir = dialog.getDirection();
        bool even = dialog.getDirection();
        MasterPeriodicBCNode* master = dialog.getMaster();
        if (master) {
            auto slave_bc = modeler_->createSlaveBC(master, dir, even, name.toStdString());
            modeler_->createBCObject(head, "Slave", slave_bc);
            model_->layoutChanged();
            render_view_->updateLink();
        }
    }
}

void MainWindow::onModelerMovingBand(GeomHeadNode* head)
{
    MovingBandInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString name = dialog.getName();
        QString initial_pos = dialog.getInitialPosition();
        QString speed = dialog.getSpeed();

        auto mb = modeler_->createMovingBand(speed.toStdString(), initial_pos.toStdString(), name.toStdString());
        modeler_->createBCObject(head, "Band", mb);

        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerTransient(bzmag::engine::SolutionSetup* setup)
{
    if (!setup) {
        QMessageBox::warning(nullptr, "Error", "Target node(SolutionSetup) is required.");
        return;
    }

    TransientInputDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString stop_time = dialog.getStopTime();
        QString time_step = dialog.getTimeStep();

        auto transient = modeler_->createTransientSetup("Transient", setup);
        transient->setStopTime(stop_time.toStdString());
        transient->setTimeStep(time_step.toStdString());
        model_->layoutChanged();
        render_view_->updateLink();
    }
}

void MainWindow::onModelerDelete(Node* node)
{
    if (node) {
        uint32 ref_base = 0;
        if (node->getType()->getName() == "GeomHeadNode") {
            ref_base++;
        }
        if (node->getRef() <= (2 + ref_base)) {
            // 선택된 항목을 해제
            QItemSelectionModel* selection_model = tree_view_->selectionModel();
            if (selection_model) {
                selection_model->clearSelection();
            }

            // 모델에서 삭제 (트리에서도 Detach() 됨)
            model_->onNodeRemoved(node);

            // 트리모델 업데이트
            model_->layoutChanged();

            // 렌더뷰 업데이트
            render_view_->updateLink();

            // 노드의 유품 정리
            node->releaseMe();
        }
        else {
            QMessageBox::warning(nullptr, "Error", "The node is being referenced by another node.");
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    is_closing_ = true; // 종료 상태 설정

    if (render_view_) {
        render_view_->close(); // 자식의 closeEvent 호출
    }

    if (thread_ && thread_->isRunning()) {
        if (gmsh_helper_) gmsh_helper_->stop();
        if (getdp_helper_) getdp_helper_->stop();
        thread_->quit();
        thread_->wait(); // 스레드 종료 대기
    }

    thread_pool_->clear();  // 남은 작업 취소
    thread_pool_->waitForDone();  // 모든 작업 종료 대기

    event->accept();
}

void MainWindow::test1()
{
    typedef Ref<GeomHeadNode> RefHeadNode;
    typedef Ref<GeomCurveNode> RefCurveNode;
    typedef Ref<GeomCircleNode> RefCircleNode;
    typedef Ref<GeomRectNode> RefRectNode;
    typedef Ref<GeomCloneFromNode> RefCloneFromNode;
    typedef Ref<GeomCoverLineNode> RefCoverLineNode;
    typedef Ref<GeomRotateNode> RefRotateNode;
    typedef Ref<GeomMoveNode> RefMoveNode;
    typedef Ref<GeomSplitNode> RefSplitNode;
    typedef Ref<GeomSubtractNode> RefSubtractNode;
    typedef Ref<GeomUniteNode> RefUniteNode;
    typedef Ref<GeomCloneToNode> RefCloneToNode;
    typedef Ref<Expression> RefExpression;
    typedef Ref<CSNode> RefCSNode;
    typedef Ref<BCNode> RefBCNode;

    Kernel* kernel = Kernel::instance();

    // Circle / CloneTo / Subtract
    RefHeadNode head1 = kernel->create("GeomHeadNode", "/usr1/geom1");
    RefCircleNode circle1 = kernel->create("GeomCircleNode", "/usr1/geom1/CreateCircle");
    RefCoverLineNode cover1 = kernel->create("GeomCoverLineNode", "/usr1/geom1/CreateCircle/CoverLine");
    RefSubtractNode sub1 = kernel->create("GeomSubtractNode", "/usr1/geom1/CreateCircle/CoverLine/Subtract");
    RefRotateNode rot1 = kernel->create("GeomRotateNode", "/usr1/geom1/CreateCircle/CoverLine/Subtract/Rotate");

    // Cirle
    RefHeadNode head2 = kernel->create("GeomHeadNode", "/usr1/geom2");
    RefCircleNode circle2 = kernel->create("GeomCircleNode", "/usr1/geom2/CreateCircle");
    RefCoverLineNode cover2 = kernel->create("GeomCoverLineNode", "/usr1/geom2/CreateCircle/CoverLine");

    // Curve1
    RefHeadNode head3 = kernel->create("GeomHeadNode", "/usr1/geom3");
    RefCurveNode curve3 = kernel->create("GeomCurveNode", "/usr1/geom3/CreateCurve");

    // Rectangle 
    RefHeadNode head4 = kernel->create("GeomHeadNode", "/usr1/geom4");
    RefRectNode rect4 = kernel->create("GeomRectNode", "/usr1/geom4/CreateRectangle");
    RefCoverLineNode cover4 = kernel->create("GeomCoverLineNode", "/usr1/geom4/CreateRectangle/CoverLine");
    RefCoverLineNode clone4 = kernel->create("GeomCloneToNode", "/usr1/geom4/CreateRectangle/CoverLine/CloneTo");
    RefUniteNode unite4 = kernel->create("GeomUniteNode", "/usr1/geom4/CreateRectangle/CoverLine/CloneTo/Unite");
    unite4->attach(head1);

    // cloned rect
    //RefHeadNode head5       = kernel->create("GeomHeadNode",       "/usr1/geom5");
    //RefCloneFromNode clone5 = kernel->create("GeomCloneFromNode",  "/usr1/geom5/CloneFrom");
    //RefRotateNode rot5      = kernel->create("GeomRotateNode",     "/usr1/geom5/CloneFrom/Rotate");

    // Cirle
    RefHeadNode head6 = kernel->create("GeomHeadNode", "/usr1/geom6");
    RefCircleNode circle6 = kernel->create("GeomCircleNode", "/usr1/geom6/CreateCircle");
    RefCoverLineNode cover6 = kernel->create("GeomCoverLineNode", "/usr1/geom6/CreateCircle/CoverLine");

    // Coordinate system for circle1
    RefCSNode cs1 = kernel->create("CSNode", "/cs/cs1");
    RefCSNode cs2 = kernel->create("CSNode", "/cs/cs1/cs2");
    RefCSNode cs3 = kernel->create("CSNode", "/cs/cs3");

    cs1->setParameters("50, 0", "0");
    cs2->setParameters("0, 10", "_pi/3");


    circle1->setParameters("0, 0", "120");
    circle2->setParameters("0, 10.01", "109.99");
    sub1->attach(head2);
    rot1->setParameters("_pi/5");

    curve3->setParameters("10, 0", "0, 10", "0,0", "10");
    rect4->setParameters("-30, -30", "60", "60");

    //clone5->setReferenceNode(clone4);
    //rot5->setParameters("1.23");

    //bool covered = head5->isCovered();

    circle6->setParameters("0, 0", "130");

    model_->layoutChanged();
}

void MainWindow::test2()
{
    // 변수 등록
    modeler_->createExpression("Core_Width", "25", "Transformer Core Width (based on Outbounded)");
    modeler_->createExpression("Core_Height", "20", "Transformer Core Height (based on Outbounded)");
    modeler_->createExpression("Path_Width", "5", "Transformer Core Band Width");
    modeler_->createExpression("Window_Width", "Core_Width-2*Path_Width", "Window Width");
    modeler_->createExpression("Window_Height", "Core_Height-2*Path_Width", "Window Height");
    modeler_->createExpression("WiningWidth", "5", "Winding Width");
    modeler_->createExpression("AirgapWidth", "1", "Winding Width");
    modeler_->createExpression("AirModel_Width", "Core_Width+2*WiningWidth", "Airmodel Width");
    modeler_->createExpression("N_c1", "10", "Number of Turns for the Primary Winding");
    modeler_->createExpression("N_c2", "20", "Number of Turns for the Secondary Winding");
    modeler_->createExpression("I_a", "20", "Applied Current of Primary Winding");

    // 코어
    GeomBaseNode* core = modeler_->createRectangle("-Core_Width/2, -Core_Height/2", "Core_Width", "Core_Height", "Core");
    GeomBaseNode* core_in = modeler_->createRectangle("-Window_Width/2, -Window_Height/2", "Window_Width", "Window_Height", "Core_in");
    GeomBaseNode* airgap = modeler_->createRectangle("-AirgapWidth/2, 0", "AirgapWidth", "Core_Height", "Airgap");
    modeler_->booleanSubtract(core, { core_in, airgap });

    // 1차측 권선
    GeomHeadNode* winding11 = modeler_->createRectangle("-Core_Width/2, -Window_Height/2", "-WiningWidth", "Window_Height", "PrimaryWining(Left)")->getHeadNode();
    GeomHeadNode* winding12 = modeler_->createRectangle("-Window_Width/2, -Window_Height/2", "WiningWidth", "Window_Height", "PrimaryWining(Right)")->getHeadNode();
    winding11->setColor({ 255, 170, 0, 230 });
    winding12->setColor({ 255, 170, 127, 230 });

    // 2차측 권선
    GeomHeadNode* winding22 = modeler_->createRectangle("Core_Width/2, -Window_Height/2", "WiningWidth", "Window_Height", "SecondaryWining(Right)")->getHeadNode();
    GeomHeadNode* winding21 = modeler_->createRectangle("Window_Width/2, -Window_Height/2", "-WiningWidth", "Window_Height", "SecondaryWining(Left)")->getHeadNode();
    winding21->setColor({ 170, 170, 0, 230 });
    winding22->setColor({ 170, 170, 127, 230 });

    // AirModel
    GeomHeadNode* airmodel = modeler_->createRectangle("-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width * 1.2", "Core_Height * 1.2", "AirModel")->getHeadNode();
    airmodel->setColor({ 255, 255, 255, 0 });

    // 경계라인 (반시계 방향)
    GeomHeadNode* bc1 = modeler_->createLine("-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "Boundary_Bottom")->getHeadNode();
    GeomHeadNode* bc2 = modeler_->createLine("AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "Boundary_Right")->getHeadNode();
    GeomHeadNode* bc3 = modeler_->createLine("AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "-AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "Boundary_Top")->getHeadNode();
    GeomHeadNode* bc4 = modeler_->createLine("-AirModel_Width/2 * 1.2, Core_Height/2 * 1.2", "-AirModel_Width/2 * 1.2, -Core_Height/2 * 1.2", "Boundary_Left")->getHeadNode();

    // 경계조건 생성
    BCNode* bc = modeler_->createDirichletBC("0", "ZeroPotential");
    GeomHeadRefNode* ref_head1 = modeler_->createBCObject(bc1, "Fixed1", bc);
    GeomHeadRefNode* ref_head2 = modeler_->createBCObject(bc2, "Fixed2", bc);
    GeomHeadRefNode* ref_head3 = modeler_->createBCObject(bc3, "Fixed3", bc);
    GeomHeadRefNode* ref_head4 = modeler_->createBCObject(bc4, "Fixed4", bc);

    // 재질(코어)
    MaterialNode* mat_core = modeler_->createMaterial("35PN270", true);
    DataSet dataset{
        {0, 0},
        {6.15, 0.05},
        {12.3, 0.1},
        {18.4, 0.15},
        {24.6, 0.2},
        {30.7, 0.25},
        {36.9, 0.3},
        {43, 0.35},
        {49.2, 0.4},
        {55.3, 0.45},
        {61.5, 0.5},
        {67.7, 0.55},
        {73.8, 0.6},
        {80, 0.65},
        {86.2, 0.7},
        {92.5, 0.75},
        {98.7, 0.8},
        {105, 0.85},
        {112, 0.9},
        {118, 0.95},
        {125, 1},
        {132, 1.05},
        {140, 1.1},
        {148, 1.15},
        {159, 1.2},
        {171, 1.25},
        {189, 1.3},
        {214, 1.35},
        {252, 1.4},
        {315, 1.45},
        {422, 1.5},
        {609, 1.55},
        {947, 1.6},
        {1570, 1.65},
        {2740, 1.7},
        {4990, 1.75},
        {9360, 1.8},
        {18000, 1.85},
        {35500, 1.9},
        {71300, 1.95},
        {146000, 2},
        {304000, 2.05},
        {644000, 2.1},
        {1.39e+06, 2.15},
        {3.04e+06, 2.2},
        {6.78e+06, 2.25},
        {1.54e+07, 2.3},
        {3.55e+07, 2.35},
        {8.33e+07, 2.4} };
    DataSetNode* dataset_node = mat_core->getDataSetNode();
    dataset_node->setDataset(dataset);
    mat_core->setPermeability("1000");

    MaterialNode* mat_copper = modeler_->createMaterial("copper", false);
    mat_copper->setConductivity("5.08*1e7");
    mat_copper->setPermeability("0.95");

    // 코어재질 설정
    core->getHeadNode()->setMaterialNode(mat_core);

    // 권선재질 설정
    winding11->setMaterialNode(mat_copper);
    winding12->setMaterialNode(mat_copper);
    winding21->setMaterialNode(mat_copper);
    winding22->setMaterialNode(mat_copper);

    // 여자조건 생성
    WindingNode* primary = modeler_->createWinding("I_a", "1", "PrimaryWinding");
    WindingNode* secondary = modeler_->createWinding("0", "1", "SecondaryWinding");
    CoilNode* prim_coil = modeler_->createCoil("N_c1", true, "Primary+", primary);
    CoilNode* prim_recoil = modeler_->createCoil("N_c1", false, "Primary-", primary);
    CoilNode* second_coil = modeler_->createCoil("N_c2", true, "Secondary+", secondary);
    CoilNode* second_recoil = modeler_->createCoil("N_c2", false, "Secondary-", secondary);

    prim_coil->setReferenceNode(winding11);
    prim_recoil->setReferenceNode(winding12);
    second_coil->setReferenceNode(winding21);
    second_recoil->setReferenceNode(winding22);

    render_view_->updateLink();
}

void MainWindow::test3()
{
    // 변수 등록
    modeler_->createExpression("N_poles", "8", "Number of Poles");
    modeler_->createExpression("N_slots", "12", "Number of Slot");

    modeler_->createExpression("R_so", "100/2", "Stator Outer Diameter");
    modeler_->createExpression("R_si", "55/2", "Stator Inner Diameter");
    modeler_->createExpression("R_ro", "54/2", "Rotor Outer Diameter");
    modeler_->createExpression("R_shaft", "30/2", "Shaft Diameter");
    modeler_->createExpression("g", "(R_si - R_ro)", "Airgap");

    modeler_->createExpression("d1", "1", "Shoe Tip1");
    modeler_->createExpression("d2", "1", "Shoe Tip2");
    modeler_->createExpression("d3", "15", "Active Slot Depth");
    modeler_->createExpression("ds", "d1 + d2 + d3", "Slot Depth");
    modeler_->createExpression("W_t", "8", "Teeth Width");
    modeler_->createExpression("W_sy", "(R_so-R_si) - ds", "Stator Back Yoke Width");
    modeler_->createExpression("W_so", "2", "Slot Open Width");

    modeler_->createExpression("T_m", "2", "Magnet Thickness");
    modeler_->createExpression("Alpha_m", "0.7", "Pole Arc Ratio (0~1)");
    modeler_->createExpression("W_ry", "(R_ro-R_shaft) - T_m", "Rotor Back Yoke Width");

    modeler_->createExpression("R_si1", "R_si + d1", "Stator Inner Radius + Shoe1");
    modeler_->createExpression("R_si2", "R_si + d1 + d2", "Stator Inner Radius + Shoe1+2");
    modeler_->createExpression("Theta_so1", "asin((W_so)/(2*R_si1))", "Angle for Half Slot Open @ R_si1");
    modeler_->createExpression("Theta_shoe1", "_pi/N_slots - Theta_so1", "Angle for Half Shoe @ R_si1");
    modeler_->createExpression("Theta_tooth2", "asin((W_t)/(2*R_si2))", "Angle for Half Tooth Width @ R_si2");

    modeler_->createExpression("N_c", "10", "Number of Turns per Coil");
    modeler_->createExpression("a", "1", "Number of Parallel Branch of the Winding");
    modeler_->createExpression("RotatingSpeed", "3600", "Rotating Speed in RPM");
    modeler_->createExpression("f_e", "RotatingSpeed/120*N_poles", "Electric Frequency");
    modeler_->createExpression("omega", "2*_pi*f_e", "Electric Angular Speed");
    modeler_->createExpression("I_a", "0", "Amplitude of Phase Current [A_peak]");
    modeler_->createExpression("beta", "0", "beta Angle [degE]");

    // 재질(코어)
    MaterialNode* mat_core = modeler_->createMaterial("35PN270", true);
    DataSet dataset{
        {0, 0},
        {6.15, 0.05},
        {12.3, 0.1},
        {18.4, 0.15},
        {24.6, 0.2},
        {30.7, 0.25},
        {36.9, 0.3},
        {43, 0.35},
        {49.2, 0.4},
        {55.3, 0.45},
        {61.5, 0.5},
        {67.7, 0.55},
        {73.8, 0.6},
        {80, 0.65},
        {86.2, 0.7},
        {92.5, 0.75},
        {98.7, 0.8},
        {105, 0.85},
        {112, 0.9},
        {118, 0.95},
        {125, 1},
        {132, 1.05},
        {140, 1.1},
        {148, 1.15},
        {159, 1.2},
        {171, 1.25},
        {189, 1.3},
        {214, 1.35},
        {252, 1.4},
        {315, 1.45},
        {422, 1.5},
        {609, 1.55},
        {947, 1.6},
        {1570, 1.65},
        {2740, 1.7},
        {4990, 1.75},
        {9360, 1.8},
        {18000, 1.85},
        {35500, 1.9},
        {71300, 1.95},
        {146000, 2},
        {304000, 2.05},
        {644000, 2.1},
        {1.39e+06, 2.15},
        {3.04e+06, 2.2},
        {6.78e+06, 2.25},
        {1.54e+07, 2.3},
        {3.55e+07, 2.35},
        {8.33e+07, 2.4} };
    DataSetNode* dataset_node = mat_core->getDataSetNode();
    dataset_node->setDataset(dataset);
    mat_core->setPermeability("1000");

    MaterialNode* mat_copper = modeler_->createMaterial("copper", false);
    mat_copper->setConductivity("5.08 * 1e7");
    mat_copper->setPermeability("0.95");

    MaterialNode* mat_magnet_n = modeler_->createMaterial("MagnetN", false);
    mat_magnet_n->setDirectionOfMagnetization("cos(atan2($Y,$X)), sin(atan2($Y,$X))");
    mat_magnet_n->setPermeability("1.05");
    mat_magnet_n->setMagnetization("1.25");

    MaterialNode* mat_magnet_s = modeler_->createMaterial("MagnetS", false);
    mat_magnet_s->setDirectionOfMagnetization("cos(atan2($Y,$X)), sin(atan2($Y,$X))");
    mat_magnet_s->setPermeability("1.05");
    mat_magnet_s->setMagnetization("-1.25");

    // 고정자
    GeomBaseNode* stator_ext = modeler_->createCircle("0,0", "R_so * 1.25", "0", "StatorExtend");
    GeomBaseNode* stator = modeler_->createCircle("0,0", "R_so", "0", "Stator");
    GeomBaseNode* air_stator = modeler_->clone(stator, "AirStator");
    GeomBaseNode* stator_ext_in = modeler_->clone(stator, "StatorExtend_in");
    stator_ext = modeler_->booleanSubtract(stator_ext, { stator_ext_in });
    
    // 고정자 재질설정
    stator->getHeadNode()->setMaterialNode(mat_core);
    
    GeomBaseNode* stator_inner = modeler_->createCircle("0,0", "R_si", "0", "StatorInner");
    GeomBaseNode* shoe_inner = modeler_->clone(stator_inner, "ShoeInner");
    GeomBaseNode* airstator_in = modeler_->clone(stator_inner, "AirStatorInner");
    air_stator = modeler_->booleanSubtract(air_stator, { airstator_in });

    GeomBaseNode* stator_bore = modeler_->createCircle("0,0", "R_so - W_sy", "0", "Stator_bore");
    stator = modeler_->booleanSubtract(stator, { stator_bore });

    // 고정자 치
    GeomBaseNode* tooth = modeler_->createRectangle("0, -W_t/2", "R_so-W_sy/2", "W_t", "Tooth");

    // 고정자 슈
    CSNode* cs_shoew_cut = modeler_->createCS("R_si1 * cos(Theta_shoe1), R_si1 * sin(Theta_shoe1)", "0", "CS_ShoeCut");

    GeomBaseNode* shoe2 = modeler_->createCircle("0,0", "R_si+d1+d2", "0", "ShoeTop");
    GeomBaseNode* shoe1 = modeler_->createCircle("0,0", "R_si+d1", "0", "ShoeMiddle");
    GeomBaseNode* clone_shoe1 = modeler_->clone(shoe1, "ShoeMiddle_1");
    shoe2 = modeler_->booleanSubtract(shoe2, { clone_shoe1 });
    shoe1 = modeler_->booleanSubtract(shoe1, { shoe_inner });
    tooth = modeler_->booleanSubtract(tooth, { stator_inner });

    CSNode* cs_half_slotpitch_p = modeler_->createCS("0,0", "_pi/N_slots", "CS_HalfSlotPitch_Positive");
    CSNode* cs_cut_shoe_p = modeler_->createCS("0,-W_so/2", "0", "CS_CutShoeP", cs_half_slotpitch_p);

    CSNode* cs_half_slotpitch_n = modeler_->createCS("0,0", "-_pi/N_slots", "CS_HalfSlotPitch_Negative");
    CSNode* cs_cut_shoe_n = modeler_->createCS("0,W_so/2", "0", "CS_CutShoeN", cs_half_slotpitch_n);

    shoe1 = modeler_->split(shoe1, GeomSplitNode::SPLIT_ZX, true, cs_cut_shoe_n);
    shoe1 = modeler_->split(shoe1, GeomSplitNode::SPLIT_ZX, false, cs_cut_shoe_p);

    shoe2 = modeler_->split(shoe2, GeomSplitNode::SPLIT_ZX, true, cs_cut_shoe_n);
    shoe2 = modeler_->split(shoe2, GeomSplitNode::SPLIT_ZX, false, cs_cut_shoe_p);

    tooth = modeler_->booleanUnite(tooth, { shoe1, shoe2 });


    for (int i = 1; i < 4; i++) {
        String name;
        name.format("Tooth_%d", i);
        GeomBaseNode* clone_tooth = modeler_->clone(tooth, name);

        String angle;
        angle.format("2*_pi/N_slots * %d", i);
        clone_tooth = modeler_->rotate(clone_tooth, angle);
        modeler_->booleanUnite(stator, { clone_tooth });
    }
    stator = modeler_->booleanUnite(stator, { tooth });

    //render_view_->updateLink();
    //return;

    CSNode* cs_cut_slotL_base = modeler_->createCS("0,0", "2*_pi/N_slots", "CS_CutSlotLBase");
    CSNode* cs_cut_slotL = modeler_->createCS("0,-W_t/2", "0", "CS_CutSlotL", cs_cut_slotL_base);
    CSNode* cs_cut_slotR = modeler_->createCS("0,W_t/2", "0", "CS_CutSlotR");
    GeomBaseNode* slotL = modeler_->createCircle("0,0", "R_si+d1+d2+d3", "0", "SlotL");
    GeomBaseNode* slotL_in = modeler_->createCircle("0,0", "R_si+d1+d2+2", "0", "Slot_in");
    slotL = modeler_->booleanSubtract(slotL, { slotL_in });
    slotL = modeler_->split(slotL, GeomSplitNode::SPLIT_ZX, true, cs_cut_slotR);
    slotL = modeler_->split(slotL, GeomSplitNode::SPLIT_ZX, false, cs_cut_slotL);
    GeomBaseNode* slotR = modeler_->clone(slotL, "SlotR");
    slotL = modeler_->split(slotL, GeomSplitNode::SPLIT_ZX, true, cs_half_slotpitch_p);
    slotR = modeler_->split(slotR, GeomSplitNode::SPLIT_ZX, false, cs_half_slotpitch_p);

    std::vector<GeomBaseNode*> slotLs;
    std::vector<GeomBaseNode*> slotRs;
    slotL->getHeadNode()->setColor(Color(255, 170, 0, 255));
    slotR->getHeadNode()->setColor(Color(255, 170, 0, 255));
    slotL->getHeadNode()->setMaterialNode(mat_copper);
    slotR->getHeadNode()->setMaterialNode(mat_copper);
    slotLs.push_back(slotL);
    slotRs.push_back(slotR);
    for (int i = 1; i < 3; i++) {
        String name1, name2;
        name1.format("SlotL_%d", i);
        name2.format("SlotR_%d", i);
        GeomBaseNode* clone_slotL = modeler_->clone(slotL, name1);
        GeomBaseNode* clone_slotR = modeler_->clone(slotR, name2);

        String angle;
        angle.format("2*_pi/N_slots * %d", i);
        clone_slotL = modeler_->rotate(clone_slotL, angle);
        clone_slotR = modeler_->rotate(clone_slotR, angle);

        clone_slotL->getHeadNode()->setColor(Color(255, 170, 0, 255));
        clone_slotR->getHeadNode()->setColor(Color(255, 170, 0, 255));
        clone_slotL->getHeadNode()->setMaterialNode(mat_copper);
        clone_slotR->getHeadNode()->setMaterialNode(mat_copper);

        slotLs.push_back(clone_slotL);
        slotRs.push_back(clone_slotR);
    }
    // 공극
    GeomBaseNode* airgap = modeler_->createCircle("0,0", "R_si", "0", "Airgap");
    GeomBaseNode* airgap_in = modeler_->createCircle("0,0", "R_ro", "0", "AirgapIn");
    airgap = modeler_->booleanSubtract(airgap, { airgap_in });

    //--------------------
    // 회전자
    CSNode* cs_cut_magnet = modeler_->createCS("0,0", "2*_pi/N_poles * Alpha_m", "CS_CutMagnet");
    GeomBaseNode* magnet = modeler_->createCircle("0,0", "R_ro", "0", "Magnet_N");
    GeomBaseNode* air_rotor = modeler_->clone(magnet, "AirRotor");

    GeomBaseNode* rotor = modeler_->createCircle("0,0", "R_ro-T_m", "0", "Rotor");
    // 회전자 재질설정
    rotor->getHeadNode()->setMaterialNode(mat_core);

    GeomBaseNode* clone_magnet_in = modeler_->clone(rotor, "Magnet_in_1");
    magnet = modeler_->booleanSubtract(magnet, { clone_magnet_in });
    magnet = modeler_->split(magnet, GeomSplitNode::SPLIT_ZX, true, modeler_->getWorkingDefaultCSNode());
    magnet = modeler_->split(magnet, GeomSplitNode::SPLIT_ZX, false, cs_cut_magnet);
    magnet = modeler_->rotate(magnet, "_pi/N_poles * (1-Alpha_m)");

    GeomBaseNode* magnet_2 = modeler_->clone(magnet, "Magnet_S");
    magnet_2 = modeler_->rotate(magnet_2, "2*_pi/N_poles");

    magnet->getHeadNode()->setColor(Color{ 255, 0, 0, 255 });
    magnet_2->getHeadNode()->setColor(Color{ 0, 0, 255, 255 });

    magnet->getHeadNode()->setMaterialNode(mat_magnet_n);
    magnet_2->getHeadNode()->setMaterialNode(mat_magnet_s);

    GeomBaseNode* shaft = modeler_->createCircle("0,0", "R_shaft", "0", "Shaft");

    std::vector<GeomBaseNode*> spilt_objs{ stator, air_stator, air_rotor, rotor, stator_ext, airgap, shaft };
    CSNode* cs_cut_model = modeler_->createCS("0,0", "2*_pi/4", "CS_CutModel");
    for (auto& obj : spilt_objs) {
        GeomBaseNode* temp = modeler_->split(obj, GeomSplitNode::SPLIT_ZX, true, modeler_->getWorkingDefaultCSNode());
        modeler_->split(temp, GeomSplitNode::SPLIT_ZX, false, cs_cut_model);
    }

    // 경계
    GeomBaseNode* ext_boundary = modeler_->createCurve("R_so * 1.25, 0", "0, R_so * 1.25", "0,0", "R_so * 1.25", "StatorExt");
    GeomBaseNode* master = modeler_->createLine("0,0", "R_so * 1.25,0", "Master");
    GeomBaseNode* slave = modeler_->clone(master, "Slave");
    slave = modeler_->rotate(slave, "2*_pi/4");

    // 경계조건 생성
    BCNode* bc = modeler_->createDirichletBC("0", "ZeroPotential");
    modeler_->createBCObject(ext_boundary->getHeadNode(), "Fixed1", bc);

    MasterPeriodicBCNode* master_bc = modeler_->createMasterBC(true, "Master");
    modeler_->createBCObject(master->getHeadNode(), "Master", master_bc);

    MasterPeriodicBCNode* slave_bc = modeler_->createSlaveBC(master_bc, true, true, "Slave");
    modeler_->createBCObject(slave->getHeadNode(), "Slave", slave_bc);

    MovingBandNode* mb = modeler_->createMovingBand("RotatingSpeed", "0", "MovingBand");
    modeler_->createBCObject(airgap->getHeadNode(), "Band", mb);


    // 여자조건 생성
    WindingNode* phaseA = modeler_->createWinding("I_a*cos(omega*$TIME + beta)", "1", "PhaseA");
    WindingNode* phaseB = modeler_->createWinding("I_a*cos(omega*$TIME + beta - 2*_pi/3)", "1", "PhaseB");
    WindingNode* phaseC = modeler_->createWinding("I_a*cos(omega*$TIME + beta + 2*_pi/3)", "1", "PhaseC");
    CoilNode* phaseA_coil = modeler_->createCoil("N_c", true, "PhaseA+", phaseA);
    CoilNode* phaseA_recoil = modeler_->createCoil("N_c", false, "PhaseA-", phaseA);
    CoilNode* phaseB_coil = modeler_->createCoil("N_c", true, "PhaseB+", phaseB);
    CoilNode* phaseB_recoil = modeler_->createCoil("N_c", false, "PhaseB-", phaseB);
    CoilNode* phaseC_coil = modeler_->createCoil("N_c", true, "PhaseC+", phaseC);
    CoilNode* phaseC_recoil = modeler_->createCoil("N_c", false, "PhaseC-", phaseC);

    phaseA_coil->setReferenceNode(slotLs[2]->getHeadNode());
    phaseA_recoil->setReferenceNode(slotRs[0]->getHeadNode());
    phaseB_coil->setReferenceNode(slotLs[0]->getHeadNode());
    phaseB_recoil->setReferenceNode(slotRs[1]->getHeadNode());
    phaseC_coil->setReferenceNode(slotLs[1]->getHeadNode());
    phaseC_recoil->setReferenceNode(slotRs[2]->getHeadNode());

    // 트랜지언지 해석 조건 생성
    Transient* transient = modeler_->createTransientSetup("Transient", modeler_->getWorkingDefaultSetupNode());
    transient->setStopTime("1/f_e");
    transient->setTimeStep("1/f_e/90");

    render_view_->updateLink();
}



