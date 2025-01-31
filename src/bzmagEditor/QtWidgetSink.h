#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/base_sink.h>
#include <QPlainTextEdit>
#include <QTextCharFormat>
#include <memory>
#include <mutex>

template<typename Mutex>
class QtWidgetSink : public spdlog::sinks::base_sink<Mutex> {
public:
    explicit QtWidgetSink(QPlainTextEdit* widget) : widget_(widget) {}

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override {
        spdlog::memory_buf_t formatted;
        this->formatter_->format(msg, formatted);

        QString log_message = QString::fromUtf8(formatted.data(), static_cast<int>(formatted.size()));

        QMetaObject::invokeMethod(widget_, [this, log_message, level = msg.level]() {
            QTextCharFormat format;
            switch (level) {
            case spdlog::level::warn:
                format.setForeground(QColor(255, 165, 0));
                break;
            case spdlog::level::err:
                format.setForeground(Qt::red);
                break;
            default:
                format.setForeground(Qt::black);
                break;
            }

            QTextCursor cursor = widget_->textCursor();
            cursor.movePosition(QTextCursor::End);
            cursor.insertText(log_message, format);
            widget_->setTextCursor(cursor);
        });
    }

    void flush_() override {}

private:
    QPlainTextEdit* widget_;
};
