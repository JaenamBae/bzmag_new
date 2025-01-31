#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QProcess>
#include <QRegularExpression>
#include "QtWidgetSink.h"

class ProcessWidget : public QWidget {
    Q_OBJECT

public:
    ProcessWidget(QWidget* parent = nullptr) : QWidget(parent) {
        // 메인 레이아웃
        auto* main_layout = new QVBoxLayout(this);

        // 로그 창 및 프로그래스바 레이아웃
        auto* log_layout = new QVBoxLayout();

        // 프로그래스바 추가
        progress_bar_ = new QProgressBar(this);
        progress_bar_->setRange(0, 100);
        log_layout->addWidget(progress_bar_);

        // 로그 뷰어 추가
        log_viewer_ = new QPlainTextEdit(this);
        log_viewer_->setReadOnly(true);
        log_layout->addWidget(log_viewer_);

        // 버튼 레이아웃 (세로 배치)
        auto* button_layout = new QVBoxLayout();

        // 시작 버튼
        start_button_ = new QPushButton(tr("Start"), this);
        button_layout->addWidget(start_button_);

        // 중단 버튼
        stop_button_ = new QPushButton(tr("Stop"), this);
        stop_button_->setEnabled(false); // 초기에는 비활성화
        button_layout->addWidget(stop_button_);

        // 버튼 레이아웃을 우측에 추가
        auto* content_layout = new QHBoxLayout();
        content_layout->addLayout(log_layout);    // 로그 창
        content_layout->addLayout(button_layout); // 버튼 레이아웃
        main_layout->addLayout(content_layout);

        // QProcess 초기화
        process_ = new QProcess(this);
        connect(process_, &QProcess::readyReadStandardOutput, this, &ProcessWidget::readStdout);
        connect(process_, &QProcess::readyReadStandardError, this, &ProcessWidget::readStderr);
        connect(process_, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &ProcessWidget::processFinished);

        // 버튼 클릭 시 슬롯 연결
        connect(start_button_, &QPushButton::clicked, this, &ProcessWidget::processStart);
        connect(stop_button_, &QPushButton::clicked, this, &ProcessWidget::processStop);

        // spdlog를 Qt 로그 뷰어와 연결
        auto sink = std::make_shared<QtWidgetSink<std::mutex>>(log_viewer_);
        auto logger = std::make_shared<spdlog::logger>("qt_logger", sink);
        spdlog::set_default_logger(logger);
    }

private slots:
    void processStart() {
        if (process_->state() != QProcess::NotRunning) {
            spdlog::warn("A process is already running.");
            return;
        }

        progress_bar_->setValue(0); // 진행률 초기화
        spdlog::info("Starting process...");
        process_->start("getdp", { "problem.pro", "-mesh", "problem.msh", "-solve", "Solve" });

        start_button_->setEnabled(false); // 실행 중에는 시작 버튼 비활성화
        stop_button_->setEnabled(true);  // 중단 버튼 활성화
    }

    void processStop() {
        if (process_->state() == QProcess::Running) {
            process_->kill();
            spdlog::info("Process terminated by user.");
        }

        start_button_->setEnabled(true);  // 시작 버튼 활성화
        stop_button_->setEnabled(false); // 중단 버튼 비활성화
    }

    void readStdout() {
        QString output = process_->readAllStandardOutput();
        spdlog::info(output.toStdString());

        QRegularExpression progress_regex(R"(\bProgress:\s*(\d+)%\b)");
        QRegularExpressionMatch match = progress_regex.match(output);
        if (match.hasMatch()) {
            int progress = match.captured(1).toInt();
            progress_bar_->setValue(progress);
        }
    }

    void readStderr() {
        QString error_output = process_->readAllStandardError();
        spdlog::error(error_output.toStdString());
    }

    void processFinished(int exit_code, QProcess::ExitStatus exit_status) {
        if (exit_status == QProcess::NormalExit) {
            spdlog::info("Process finished successfully.");
            progress_bar_->setValue(100);
        }
        else {
            spdlog::error("Process failed or terminated unexpectedly.");
        }

        start_button_->setEnabled(true);  // 시작 버튼 활성화
        stop_button_->setEnabled(false); // 중단 버튼 비활성화
    }

private:
    QProgressBar* progress_bar_;
    QPlainTextEdit* log_viewer_;
    QPushButton* start_button_;
    QPushButton* stop_button_;
    QProcess* process_;
};
