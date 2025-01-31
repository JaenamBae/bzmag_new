#pragma once

#include <QWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QTextBlock>

class PromptTextEdit : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit PromptTextEdit(QWidget* parent = nullptr)
        : QPlainTextEdit(parent), prompt_length_(4), history_index_(-1) {
        appendPlainText(">>> "); // 초기 프롬프트 설정
        moveCursor(QTextCursor::End);
    }

protected:
    void keyPressEvent(QKeyEvent* event) override {
        QTextCursor cursor = textCursor();
        int cursor_position = cursor.position();

        // 현재 줄의 시작 위치 계산
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        int block_start_position = cursor.selectionStart();

        if (event->key() == Qt::Key_Backspace && cursor_position <= block_start_position + prompt_length_) {
            return; // ">>> " 이후만 삭제 가능
        }

        // 화살표 키 처리
        if (event->key() == Qt::Key_Up) {
            navigateHistory(-1);
            return;
        }
        else if (event->key() == Qt::Key_Down) {
            navigateHistory(1);
            return;
        }

        // 엔터 키 동작
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            // 현재 줄의 명령어 가져오기
            QString current_line = cursor.block().text();
            QString command = current_line.mid(prompt_length_).trimmed(); // ">>> " 이후만 가져옴

            if (!command.isEmpty()) {
                history_.append(command); // 히스토리에 추가
                history_index_ = -1;     // 히스토리 인덱스 초기화
                emit commandExecuted(command); // 명령어 실행 신호 방출
            }

            appendPlainText(">>> "); // 새 프롬프트 추가
            moveCursor(QTextCursor::End);
        }
        else {
            QPlainTextEdit::keyPressEvent(event); // 기본 동작 유지
        }
    }

signals:
    void commandExecuted(const QString& command);

private:
    void navigateHistory(int direction) {
        if (history_.isEmpty()) {
            return;
        }

        // 히스토리 인덱스 업데이트
        history_index_ += direction;
        if (history_index_ < 0) {
            history_index_ = 0;
        }
        else if (history_index_ >= history_.size()) {
            history_index_ = history_.size() - 1;
        }

        // 명령어 업데이트
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.insertText(">>> " + history_.at(history_index_));
        moveCursor(QTextCursor::End);
    }

    const int prompt_length_;
    QStringList history_;       // 명령어 히스토리
    int history_index_;         // 현재 히스토리 인덱스
};


class PythonConsole : public QWidget {
    Q_OBJECT

public:
    explicit PythonConsole(QWidget* parent = nullptr);
    ~PythonConsole() override;

private slots:
    void executeCommand(const QString& command);
    void openAndRunFile();

signals:
    void executed();

private:
    PromptTextEdit* console_output_;
    QPushButton* file_icon_button_;
    QString current_command_;
};
