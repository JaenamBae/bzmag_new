#include "DataInputDialog.h"
#include <cmath>                // std::floor, std::log10, std::pow
#include <limits>               // std::numeric_limits
#include <QString>              // QString
#include <QStringList>          // QStringList
#include <QVBoxLayout>          // QVBoxLayout
#include <QHBoxLayout>          // QHBoxLayout
#include <QHeaderView>          // QHeaderView
#include <QMessageBox>          // QMessageBox
#include <QFile>                // QFile
#include <QFileDialog>          // QFileDialog
#include <QTextStream>          // QTextStream

DataInputDialog::DataInputDialog(bzmag::engine::DataSetNode* dataset_node, QWidget* parent)
    : QDialog(parent), dataset_node_(dataset_node) {
    setWindowTitle("Data Input and Visualization");

    table_widget_ = new QTableWidget(1, 2, this);
    table_widget_->setHorizontalHeaderLabels({ "x", "y" });
    table_widget_->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: lightblue; }");
    table_widget_->setMinimumWidth(200);
    table_widget_->setMaximumWidth(300);
    connect(table_widget_, &QTableWidget::cellChanged, this, &DataInputDialog::onCellChanged);

    QPushButton* import_button = new QPushButton("Import from CSV", this);
    QPushButton* apply_button = new QPushButton("Apply", this);
    connect(import_button, &QPushButton::clicked, this, &DataInputDialog::onImportButtonClicked);
    connect(apply_button, &QPushButton::clicked, this, &DataInputDialog::onApplyButtonClicked);

    QHBoxLayout* button_layout = new QHBoxLayout();
    button_layout->addWidget(import_button);
    button_layout->addWidget(apply_button);

    QVBoxLayout* table_layout = new QVBoxLayout();
    table_layout->addWidget(table_widget_);
    table_layout->addLayout(button_layout);

    chart_ = new QChart();
    chart_view_ = new QChartView(chart_);
    chart_view_->setRenderHint(QPainter::Antialiasing);

    scatter_series_ = new QScatterSeries();
    scatter_series_->setMarkerSize(2.0);
    line_series_ = new QLineSeries();
    chart_->addSeries(scatter_series_);
    chart_->addSeries(line_series_);
    chart_->legend()->hide();

    axis_x_ = new QValueAxis();
    axis_y_ = new QValueAxis();
    chart_->addAxis(axis_x_, Qt::AlignBottom);
    chart_->addAxis(axis_y_, Qt::AlignLeft);
    scatter_series_->attachAxis(axis_x_);
    scatter_series_->attachAxis(axis_y_);
    line_series_->attachAxis(axis_x_);
    line_series_->attachAxis(axis_y_);

    QHBoxLayout* main_layout = new QHBoxLayout(this);
    main_layout->addLayout(table_layout);
    main_layout->addWidget(chart_view_, 1);

    initializeDataFromNode();
}

void DataInputDialog::initializeDataFromNode() {
    if (!dataset_node_) return;

    const auto& dataset = dataset_node_->getDataset();
    setTableDataBlockSignals(true);
    table_widget_->setRowCount(0);

    for (const auto& data_pair : dataset) {
        int row = table_widget_->rowCount();
        table_widget_->insertRow(row);
        table_widget_->setItem(row, 0, new QTableWidgetItem(QString::number(data_pair[0])));
        table_widget_->setItem(row, 1, new QTableWidgetItem(QString::number(data_pair[1])));
    }

    table_widget_->insertRow(table_widget_->rowCount());
    setTableDataBlockSignals(false);
    updateChart();
}

void DataInputDialog::onCellChanged(int row, int column) {
    if (row == table_widget_->rowCount() - 1 && !table_widget_->item(row, column)->text().isEmpty()) {
        table_widget_->insertRow(table_widget_->rowCount());
    }
    updateChart();
}

void DataInputDialog::onImportButtonClicked() {
    QString file_name = QFileDialog::getOpenFileName(this, "Open CSV File", "", "CSV Files (*.csv);;All Files (*)");
    if (file_name.isEmpty()) return;

    QFile file(file_name);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Unable to open the CSV file.");
        return;
    }

    setTableDataBlockSignals(true);
    table_widget_->setRowCount(0);
    QTextStream in(&file);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(",");
        if (fields.size() >= 2) {
            int row = table_widget_->rowCount();
            table_widget_->insertRow(row);
            table_widget_->setItem(row, 0, new QTableWidgetItem(fields[0].trimmed()));
            table_widget_->setItem(row, 1, new QTableWidgetItem(fields[1].trimmed()));
        }
    }

    file.close();
    table_widget_->insertRow(table_widget_->rowCount());
    setTableDataBlockSignals(false);
    updateChart();
}

void DataInputDialog::onApplyButtonClicked() {
    dataset_.clear();
    for (int row = 0; row < table_widget_->rowCount(); ++row) {
        QTableWidgetItem* item_x = table_widget_->item(row, 0);
        QTableWidgetItem* item_y = table_widget_->item(row, 1);
        if (!item_x || !item_y) continue;

        bool ok_x, ok_y;
        double x = item_x->text().toDouble(&ok_x);
        double y = item_y->text().toDouble(&ok_y);

        if (ok_x && ok_y) {
            dataset_.push_back({ static_cast<bzmag::float64>(x), static_cast<bzmag::float64>(y) });
        }
    }

    if (dataset_node_) {
        dataset_node_->setDataset(dataset_);
        QMessageBox::information(this, "Success", "Dataset has been successfully updated!");
    }

    accept();
}

double DataInputDialog::calculateNiceInterval(double range) {
    static const double niceSteps[] = { 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10 };
    double exponent = std::floor(std::log10(range));
    double fraction = range / std::pow(10, exponent);

    double niceStep = 1;
    for (double step : niceSteps) {
        if (fraction <= step) {
            niceStep = step;
            break;
        }
    }

    return niceStep * std::pow(10, exponent);
}

void DataInputDialog::setTableDataBlockSignals(bool block) {
    table_widget_->blockSignals(block);
}

void DataInputDialog::updateChart() {
    scatter_series_->clear();
    line_series_->clear();

    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();

    for (int row = 0; row < table_widget_->rowCount(); ++row) {
        QTableWidgetItem* item_x = table_widget_->item(row, 0);
        QTableWidgetItem* item_y = table_widget_->item(row, 1);
        if (!item_x || !item_y) continue;

        bool ok_x, ok_y;
        double x = item_x->text().toDouble(&ok_x);
        double y = item_y->text().toDouble(&ok_y);

        if (ok_x && ok_y) {
            scatter_series_->append(x, y);
            line_series_->append(x, y);

            min_x = std::min(min_x, x);
            max_x = std::max(max_x, x);
            min_y = std::min(min_y, y);
            max_y = std::max(max_y, y);
        }
    }

    if (min_x == std::numeric_limits<double>::max() || min_y == std::numeric_limits<double>::max()) {
        min_x = min_y = 0;
        max_x = max_y = 10;
    }

    double range_x = max_x - min_x;
    double range_y = max_y - min_y;

    double niceIntervalX = calculateNiceInterval(range_x);
    double niceIntervalY = calculateNiceInterval(range_y);

    double adjustedMinX = std::floor(min_x / niceIntervalX) * niceIntervalX;
    double adjustedMaxX = std::ceil(max_x / niceIntervalX) * niceIntervalX;
    double adjustedMinY = std::floor(min_y / niceIntervalY) * niceIntervalY;
    double adjustedMaxY = std::ceil(max_y / niceIntervalY) * niceIntervalY;

    axis_x_->setRange(adjustedMinX, adjustedMaxX);
    axis_y_->setRange(adjustedMinY, adjustedMaxY);

    axis_x_->setTickInterval(niceIntervalX);
    axis_y_->setTickInterval(niceIntervalY);
}

bzmag::DataSet DataInputDialog::getDataSet() const
{
    return dataset_;
}
