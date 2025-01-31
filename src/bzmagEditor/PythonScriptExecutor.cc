#include "bzmagpy/pythonscriptserver.h"
#include "PythonScriptExecutor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include "Modeler.h"

using namespace bzmag;

PythonScriptExecutor::PythonScriptExecutor(QWidget* parent)
    : QWidget(parent) // QWidget으로 변경
{
    // UI 요소 초기화
    QVBoxLayout* main_layout = new QVBoxLayout(this);
    QHBoxLayout* input_layout = new QHBoxLayout();

    script_input_ = new QLineEdit(this);
    execute_button_ = new QPushButton("Execute", this);
    load_button_ = new QPushButton("Load Script", this);
    output_display_ = new QPlainTextEdit(this);
    output_display_->setReadOnly(true);

    // 버튼 크기 작게 조정
    execute_button_->setFixedSize(80, 25);
    load_button_->setFixedSize(80, 25);

    // 입력창과 버튼 배치
    input_layout->addWidget(script_input_);
    input_layout->addWidget(execute_button_);
    input_layout->addWidget(load_button_);

    // 전체 레이아웃 구성
    main_layout->addLayout(input_layout);
    main_layout->addWidget(output_display_);

    setLayout(main_layout); // setWidget 대신 setLayout 사용

    // 연결
    connect(execute_button_, &QPushButton::clicked, this, &PythonScriptExecutor::executeScript);
    connect(load_button_, &QPushButton::clicked, this, &PythonScriptExecutor::loadScriptFromFile);
    connect(script_input_, &QLineEdit::returnPressed, this, &PythonScriptExecutor::executeScript);

    // 생성자 내에 추가
    output_display_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(output_display_, &QPlainTextEdit::customContextMenuRequested,
        this, &PythonScriptExecutor::showOutputContextMenu);


    // Python 초기화
    bzmagPyScriptServer* pyServer = bzmagPyScriptServer::instance();
    String test_code =
R"(
import sys, os
import bzmagPy as bzmag
sys.path.append('python')
modeler = bzmag.get('/sys/modeler')
)";
    String res;
    pyServer->run(test_code, &res);
    //printf("%s", res.c_str());
}

QPlainTextEdit* PythonScriptExecutor::getLogWindows()
{
    return output_display_;
}

void PythonScriptExecutor::executeScript() {
    QString script = script_input_->text();
    if (script.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Script cannot be empty!");
        return;
    }

    // 명령어 출력
    output_display_->appendPlainText(script);

    runPythonScript(script);
}

void PythonScriptExecutor::loadScriptFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Python Script", "", "Python Files (*.py)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open the file.");
        return;
    }

    QString script = file.readAll();
    file.close();
    runPythonScript(script);
    //script_input_->setText(script);
}

void PythonScriptExecutor::runPythonScript(const QString& script)
{
    Modeler* modeler = Modeler::instance();
    if (modeler->getWorkingTemplate() != modeler->getDefaultTemplate()) {
        output_display_->appendPlainText("Script can only work on default template!");
        return;
    }

    bzmagPyScriptServer* py_server = bzmagPyScriptServer::instance();

    // 명령어 실행
    String res;
    py_server->run(script.toStdString(), &res);

    // 실행 결과 출력
    output_display_->appendPlainText(QString::fromStdString(res.c_str()));

    // 명령줄 클리어
    script_input_->setText("");

    emit executed();
}

// 컨텍스트 메뉴 표시
void PythonScriptExecutor::showOutputContextMenu(const QPoint& pos)
{
    // 기본 컨텍스트 메뉴 가져오기
    QMenu* context_menu = output_display_->createStandardContextMenu();

    // Clear 항목 추가
    context_menu->addSeparator(); // 메뉴 구분선 추가
    QAction* clear_action = context_menu->addAction("Clear");
    connect(clear_action, &QAction::triggered, this, &PythonScriptExecutor::clearOutputDisplay);

    // 메뉴 표시
    context_menu->exec(output_display_->mapToGlobal(pos));
    delete context_menu; // 메뉴 삭제
}

// 출력 창 클리어
void PythonScriptExecutor::clearOutputDisplay()
{
    output_display_->clear();
}