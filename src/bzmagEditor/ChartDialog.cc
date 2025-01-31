#include "ChartDialog.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QPushButton>
#include <QHBoxLayout>
#include <QtCharts/QValueAxis>
#include <QScatterSeries>
#include <QToolTip>
#include <QMouseEvent>

ChartDialog::ChartDialog(PostPlotNode* node, QWidget* parent)
    : QDialog(parent),
    chart_(new QChart()),
    chart_view_(new QChartView(chart_, this)),
    node_(node) {
    chart_->setTitle(node->getName().c_str());

    chart_view_->setRenderHint(QPainter::Antialiasing);
    chart_view_->setRubberBand(QChartView::HorizontalRubberBand);

    auto* x_axis = new QValueAxis(this);
    x_axis->setTitleText("X Axis");

    auto* y_axis = new QValueAxis(this);
    y_axis->setTitleText("Y Axis");

    chart_->addAxis(x_axis, Qt::AlignBottom);
    chart_->addAxis(y_axis, Qt::AlignLeft);

    auto& plot_data = node->getPlottingData();

    double x_min = std::numeric_limits<double>::max();
    double x_max = std::numeric_limits<double>::lowest();
    double y_min = std::numeric_limits<double>::max();
    double y_max = std::numeric_limits<double>::lowest();

    for (const auto& [name, dataset] : plot_data) {
        if (dataset.empty()) continue;

        for (const auto& data : dataset) {
            if (data.x() < x_min) x_min = data.x();
            if (data.x() > x_max) x_max = data.x();
            if (data.y() < y_min) y_min = data.y();
            if (data.y() > y_max) y_max = data.y();
        }

        if (dataset.size() == 1) {
            QScatterSeries* scatter_series = new QScatterSeries(this);
            scatter_series->setName(name.c_str());
            scatter_series->append(dataset[0].x(), dataset[0].y());
            chart_->addSeries(scatter_series);
            scatter_series->attachAxis(x_axis);
            scatter_series->attachAxis(y_axis);
        }
        else {
            QLineSeries* line_series = new QLineSeries(this);
            line_series->setName(name.c_str());
            for (const auto& data : dataset) {
                line_series->append(data.x(), data.y());
            }
            line_series->setPointsVisible(true);
            chart_->addSeries(line_series);
            line_series->attachAxis(x_axis);
            line_series->attachAxis(y_axis);
        }
    }

    if (x_min == std::numeric_limits<double>::max() || x_max == std::numeric_limits<double>::lowest()) {
        x_min = 0.0;
        x_max = 1.0;
    }
    if (y_min == std::numeric_limits<double>::max() || y_max == std::numeric_limits<double>::lowest()) {
        y_min = 0.0;
        y_max = 1.0;
    }

    if (x_min == x_max) {
        x_min -= 0.5;
        x_max += 0.5;
    }
    if (y_min == y_max) {
        y_min -= 0.5;
        y_max += 0.5;
    }

    //double y_range = y_max - y_min;
    //double y_interval = calculateNiceInterval(y_range);
    //y_min = std::floor(y_min / y_interval) * y_interval;
    //y_max = std::ceil(y_max / y_interval) * y_interval;

    double y_min_re = 0;
    if (y_min < 0) {
        y_min_re = calculateNiceInterval(abs(y_min), true);
        y_min_re *= -1;
    }
    else {
        y_min_re = calculateNiceInterval(abs(y_min), false);
    }

    double y_max_re = 0;
    if (y_max < 0) {
        y_max_re = calculateNiceInterval(abs(y_max), false);
        y_max_re *= -1;
    }
    else {
        y_max_re = calculateNiceInterval(abs(y_max), true);
    }

    x_axis->setRange(x_min, x_max);
    y_axis->setRange(y_min_re, y_max_re);

    initial_x_min_ = x_min;
    initial_x_max_ = x_max;
    initial_y_min_ = y_min_re;
    initial_y_max_ = y_max_re;

    QVBoxLayout* main_layout = new QVBoxLayout(this);
    main_layout->addWidget(chart_view_);

    QHBoxLayout* button_layout = new QHBoxLayout;

    QPushButton* reset_button = new QPushButton("Reset", this);
    connect(reset_button, &QPushButton::clicked, this, &ChartDialog::resetZoom);
    button_layout->addWidget(reset_button);

    QPushButton* export_button = new QPushButton("Export CSV", this);
    connect(export_button, &QPushButton::clicked, this, &ChartDialog::exportToCSV);
    button_layout->addWidget(export_button);

    main_layout->addLayout(button_layout);
    setLayout(main_layout);

    for (auto* series : chart_->series()) {
        auto* line_series = qobject_cast<QLineSeries*>(series);
        if (line_series) {
            connect(line_series, &QLineSeries::hovered, this, &ChartDialog::showTooltip);
        }
        auto* scatter_series = qobject_cast<QScatterSeries*>(series);
        if (scatter_series) {
            connect(scatter_series, &QScatterSeries::hovered, this, &ChartDialog::showTooltip);
        }
    }
}

void ChartDialog::resetZoom() {
    auto* x_axis = qobject_cast<QValueAxis*>(chart_->axes(Qt::Horizontal).first());
    auto* y_axis = qobject_cast<QValueAxis*>(chart_->axes(Qt::Vertical).first());

    if (x_axis) {
        x_axis->setRange(initial_x_min_, initial_x_max_);
    }
    if (y_axis) {
        y_axis->setRange(initial_y_min_, initial_y_max_);
    }
}

void ChartDialog::exportToCSV() {
    QString file_name = QFileDialog::getSaveFileName(this, "Save Chart Data", "chart_data.csv", "CSV Files (*.csv)");
    if (file_name.isEmpty()) return;

    QFile file(file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    for (auto* series : chart_->series()) {
        auto* line_series = qobject_cast<QLineSeries*>(series);
        if (!line_series) continue;

        out << line_series->name() << ",X,Y\n";
        for (const QPointF& point : line_series->points()) {
            out << "," << point.x() << "," << point.y() << "\n";
        }
    }

    file.close();
}

void ChartDialog::showTooltip(const QPointF& point, bool state) {
    if (state) {
        QToolTip::showText(QCursor::pos(), QString("X: %1\nY: %2").arg(point.x()).arg(point.y()));
    }
    else {
        QToolTip::hideText();
    }
}

double ChartDialog::calculateNiceInterval(double range, bool upper)
{
    static const double nice_steps[] = { 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10 }; // 보기 좋은 간격
    double exponent = std::floor(std::log10(range));
    double fraction = range / std::pow(10, exponent);

    double nice_step = 1;
    double pre_step = 1;
    for (double step : nice_steps) {
        if (fraction < step) {
            if (upper) {
                nice_step = step;
            }
            else {
                nice_step = pre_step;
            }
            break;
        }
        pre_step = step;
    }

    return nice_step * std::pow(10, exponent);
}
