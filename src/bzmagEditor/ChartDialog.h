#pragma once

#include <QDialog>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "PostPlotNode.h"

QT_CHARTS_USE_NAMESPACE

class ChartDialog : public QDialog {
    Q_OBJECT

public:
    explicit ChartDialog(PostPlotNode* node, QWidget* parent = nullptr);
    ~ChartDialog() {};

private slots:
    void resetZoom();
    void exportToCSV();
    void showTooltip(const QPointF& point, bool state);

private:
    double calculateNiceInterval(double range, bool upper = true); // 간격 계산

    QChart* chart_;
    QChartView* chart_view_;
    PostPlotNode* node_;

    double initial_x_min_; // 초기 X축 최소값
    double initial_x_max_; // 초기 X축 최대값
    double initial_y_min_; // 초기 Y축 최소값
    double initial_y_max_; // 초기 Y축 최대값
};
