#pragma once

#include <QWidget>
#include <QPainter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

class CircularProgressBar : public QWidget {
    Q_OBJECT

public:
    explicit CircularProgressBar(QWidget* parent = nullptr)
        : QWidget(parent), progress_(0) {}

    void setProgress(int value) {
        progress_ = qBound(0, value, 100); // 0 ~ 100 사이로 제한
        update(); // 화면 갱신
    }

protected:
    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        int side = qMin(width(), height());
        QRectF rect(5, 5, side - 10, side - 10);

        // 배경 원
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::lightGray);
        painter.drawEllipse(rect);

        // 진행 원호
        painter.setBrush(Qt::blue);
        QPen pen(Qt::blue, 8);
        painter.setPen(pen);

        int startAngle = 90 * 16; // 12시 방향 시작
        int spanAngle = -static_cast<int>(360.0 * progress_ / 100 * 16); // 진행 각도
        painter.drawArc(rect, startAngle, spanAngle);

        // 텍스트 표시
        painter.setPen(Qt::black);
        painter.setFont(QFont("Arial", side / 10, QFont::Bold)); // 텍스트 크기 자동 조정
        QString text = QString("%1%").arg(progress_);
        painter.drawText(rect, Qt::AlignCenter, text);
    }

private:
    int progress_; // 진행 상태 (0~100)
};