#ifndef PYTHONSCRIPTEXECUTOR_H
#define PYTHONSCRIPTEXECUTOR_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPlainTextEdit>

class PythonScriptExecutor : public QWidget {
    Q_OBJECT

public:
    explicit PythonScriptExecutor(QWidget* parent = nullptr);
    QPlainTextEdit* getLogWindows();

private slots:
    void executeScript();
    void loadScriptFromFile();
    void showOutputContextMenu(const QPoint& pos);
    void clearOutputDisplay();

signals:
    void executed();

private:
    QLineEdit* script_input_;   // 한 줄 입력창
    QPushButton* execute_button_; // 실행 버튼
    QPushButton* load_button_; // 파일 로드 버튼
    QPlainTextEdit* output_display_; // 출력 창

    void runPythonScript(const QString& script);
};

#endif // PYTHONSCRIPTEXECUTOR_H