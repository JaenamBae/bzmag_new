#include "PythonConsole.h"
#include "bzmagPy/pythonscriptserver.h"
#include <QKeyEvent>
#include <QScrollBar>
#include <QFile>
#include <QMessageBox>
#include <QIcon>
#include <QTextStream>
#include <QTextBlock>
#include "Modeler.h"

using namespace bzmag;

PythonConsole::PythonConsole(QWidget* parent)
    : QWidget(parent), current_command_("") {
    setWindowTitle("Python Console");

    auto* main_layout = new QHBoxLayout(this);
    main_layout->setContentsMargins(0, 0, 0, 0); // 전체 레이아웃 여백 제거
    main_layout->setSpacing(0);                  // 레이아웃 간 간격 제거

    // 좌측 아이콘 영역을 위한 위젯 생성
    auto* icon_container = new QWidget(this);
    icon_container->setStyleSheet(
        "background-color: #4A4E69;" // 아이콘 배경색
        "border: none;"             // 경계 제거
    );
    auto* icon_layout = new QVBoxLayout(icon_container);
    icon_layout->setContentsMargins(0, 0, 0, 0); // 여백 제거
    icon_layout->setSpacing(0);                  // 위젯 간 간격 제거
    icon_layout->setAlignment(Qt::AlignTop);

    // 아이콘 버튼 추가
    file_icon_button_ = new QPushButton(icon_container);
    file_icon_button_->setText("📂");
    file_icon_button_->setStyleSheet(
        "QPushButton {"
        "    background-color: #4A4E69;" // 아이콘 배경색
        "    color: #F8F8F2;"            // 텍스트 색상
        "    border: none;"
        "    font-size: 14px;"
        "    padding: 5px;"
        "    border-radius: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #6C7093;" // 호버 시 밝은 배경색
        "}"
        "QPushButton:pressed {"
        "    background-color: #3A3D4D;" // 클릭 시 어두운 배경색
        "}"
    );
    file_icon_button_->setFixedSize(25, 25); // 아이콘 크기 조정
    icon_layout->addWidget(file_icon_button_);

    // 전체 레이아웃에 아이콘 컨테이너 추가
    main_layout->addWidget(icon_container);

    // 콘솔 출력 및 입력 영역
    console_output_ = new PromptTextEdit(this);
    console_output_->setReadOnly(false); // 입력 가능
    console_output_->setTextInteractionFlags(Qt::TextEditorInteraction);
    console_output_->setStyleSheet(
        "QPlainTextEdit {"
        "    background-color: #2B2B2B;" // 콘솔 배경색
        "    color: #A9B7C6;"           // 텍스트 색상
        "    font-family: Consolas;"    // 폰트 스타일
        "    font-size: 12px;"          // 폰트 크기
        "    border: none;"             // 외곽선 제거
        "    padding: 5px;"             // 내부 여백 추가
        "}"
        "QScrollBar:vertical {"
        "    border: none;"             // 스크롤바 테두리 제거
        "    background: #313335;"      // 스크롤바 배경색
        "    width: 12px;"              // 스크롤바 폭
        "    margin: 0px;"              // 여백 제거
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #5D5F61;"      // 스크롤바 핸들 색상
        "    min-height: 20px;"         // 핸들의 최소 높이
        "    border-radius: 4px;"       // 둥근 모서리
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"              // 위아래 버튼 숨김
        "    background: none;"
        "}"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"         // 스크롤바 배경 채우기 제거
        "}"
    );

    // 파일 실행 신호 연결
    connect(file_icon_button_, &QPushButton::clicked, this, &PythonConsole::openAndRunFile);

    // 명령어 실행 신호 연결
    connect(console_output_, &PromptTextEdit::commandExecuted, this, &PythonConsole::executeCommand);

    // 전체 레이아웃에 콘솔 영역 추가
    main_layout->addWidget(console_output_);

    // 초기화
    console_output_->moveCursor(QTextCursor::End);
    console_output_->setFocus(); // 커서 활성화
}


PythonConsole::~PythonConsole() {}

void PythonConsole::executeCommand(const QString& command) {
    Modeler* modeler = Modeler::instance();
    if (modeler->getWorkingTemplate() != modeler->getDefaultTemplate()) {
        console_output_->appendPlainText("Script can only work on default template!");
        return;
    }

    bzmagPyScriptServer* py_server = bzmagPyScriptServer::instance();

    // 명령어 실행
    String res;
    py_server->run(command.toStdString(), &res);

    //console_output_->appendPlainText(">>> " + command);
    console_output_->appendPlainText(res.c_str());
    console_output_->moveCursor(QTextCursor::End);

    emit executed();
}

void PythonConsole::openAndRunFile() {
    Modeler* modeler = Modeler::instance();
    if (modeler->getWorkingTemplate() != modeler->getDefaultTemplate()) {
        console_output_->appendPlainText("Script can only work on default template!");
        return;
    }

    QString file_path = QFileDialog::getOpenFileName(this, "Open Python Script", "", "Python Files (*.py)");
    if (file_path.isEmpty()) {
        return;
    }

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Failed to open the file.");
        return;
    }

    QTextStream in(&file);
    QString script_content = in.readAll();
    file.close();

    bzmagPyScriptServer* py_server = bzmagPyScriptServer::instance();

    // 명령어 실행
    String res;
    py_server->run(script_content.toStdString(), &res);

    console_output_->appendPlainText(res.c_str());
    console_output_->moveCursor(QTextCursor::End);

    emit executed();
}