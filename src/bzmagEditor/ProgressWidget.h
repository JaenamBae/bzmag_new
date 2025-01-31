#pragma once

#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QString>
#include <QPushButton>

class ProgressWidget : public QWidget {
    Q_OBJECT

public:
    explicit ProgressWidget(QWidget* parent = nullptr) : QWidget(parent) {
        auto* main_layout = new QVBoxLayout(this);

        // 작업 설명 레이블
        task_label_ = new QLabel("No task in progress...", this);
        task_label_->setAlignment(Qt::AlignCenter);
        main_layout->addWidget(task_label_, 0, Qt::AlignBottom);

        // 프로그래스바
        progress_bar_ = new QProgressBar(this);
        progress_bar_->setRange(0, 100);
        progress_bar_->setFixedHeight(20);
        progress_bar_->setValue(0);
        main_layout->addWidget(progress_bar_, 0, Qt::AlignTop);

        progress_bar_->setStyleSheet(
            "QProgressBar {"
            "    text-align: center;"   // 텍스트를 중앙 정렬
            "    font-weight: bold;"   // 텍스트 굵게
            "    font-size: 14px;"     // 텍스트 크기
            "}");

        // 버튼 레이아웃
        auto* button_layout = new QHBoxLayout();
        button_layout->addStretch(); // 왼쪽 여백
        mesh_button_ = new QPushButton("Mesh", this);
        analysis_button_ = new QPushButton("Analysis", this);

        button_layout->addWidget(mesh_button_);
        button_layout->addWidget(analysis_button_);
        main_layout->addLayout(button_layout);

        // 버튼 클릭 시그널 연결
        connect(mesh_button_, &QPushButton::clicked, this, &ProgressWidget::onMeshButtonClicked);
        connect(analysis_button_, &QPushButton::clicked, this, &ProgressWidget::onAnalysisButtonClicked);
    }

public slots:
    void startTask(const QString& message) {
        progress_bar_->setValue(0);
        task_label_->setText(message);
    }

    void updateLabel(const QString& message) {
        task_label_->setText(message);
    }

    void updateProgress(int progress) {
        progress_bar_->setValue(progress);
    }

    void clearTask() {
        progress_bar_->setValue(0);
        task_label_->setText("No task in progress...");
        resetButtons();
    }

    void startedMesh() {
        mesh_button_->setText("Stop");
    }

    void startedAnalysis() {
        analysis_button_->setText("Stop");
    }

private slots:
    void onMeshButtonClicked() {
        if (mesh_button_->text() == "Mesh") {
            mesh_button_->setText("Stop");
            analysis_button_->setDisabled(true);
            emit taskStarted("Mesh");
        }
        else {
            mesh_button_->setText("Mesh");
            analysis_button_->setDisabled(false);
            emit taskStopped();
        }
    }

    void onAnalysisButtonClicked() {
        if (analysis_button_->text() == "Analysis") {
            analysis_button_->setText("Stop");
            mesh_button_->setEnabled(false);
            emit taskStarted("Analysis");
        }
        else {
            analysis_button_->setText("Analysis");
            mesh_button_->setEnabled(true);
            emit taskStopped();
        }
    }

private:
    void resetButtons() {
        mesh_button_->setText("Mesh");
        analysis_button_->setText("Analysis");
        mesh_button_->setEnabled(true);
        analysis_button_->setDisabled(false);
    }

signals:
    void taskStarted(const QString& task); // 작업 시작 신호
    void taskStopped(); // 작업 중단 신호

private:
    QLabel* task_label_;
    QProgressBar* progress_bar_;
    QPushButton* mesh_button_;
    QPushButton* analysis_button_;
};