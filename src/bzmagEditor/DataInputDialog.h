#ifndef DATAINPUTDIALOG_H
#define DATAINPUTDIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include "engine/DataSetNode.h"

QT_CHARTS_USE_NAMESPACE

class DataInputDialog : public QDialog {
public:
    DataInputDialog(bzmag::engine::DataSetNode* dataset_node, QWidget* parent = nullptr);
    bzmag::DataSet getDataSet() const;

private slots:
    void onCellChanged(int row, int column);
    void onImportButtonClicked();
    void onApplyButtonClicked();
    void updateChart();

private:
    void initializeDataFromNode();
    double calculateNiceInterval(double range);
    void setTableDataBlockSignals(bool block);


    // UI Elements
    QTableWidget* table_widget_;
    QChartView* chart_view_;

    // Chart Elements
    QChart* chart_;
    QScatterSeries* scatter_series_;
    QLineSeries* line_series_;
    QValueAxis* axis_x_;
    QValueAxis* axis_y_;

    // Dataset
    bzmag::DataSet dataset_;
    bzmag::engine::DataSetNode* dataset_node_;
};

#endif // DATAINPUTDIALOG_H
