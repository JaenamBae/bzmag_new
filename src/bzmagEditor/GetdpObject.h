#pragma once

#include <QPainter>
#include <QPoint>
#include <QMouseEvent>
#include <QMutex>
#include <memory>
#include "DrawingObject.h"
#include <string>
#include <vector>
#include <algorithm>

class GetdpObject : public DrawingObject
{
public:
    using ScalarField = std::vector<float>;
    using VectorField = std::vector<QVector3D>;
    struct Arrow {
        Vertex origin;
        std::vector<Vertex> nodes;
        std::vector<Triangle> elements;
    };

public:
    GetdpObject(Flat3DShader* shader);
    virtual ~GetdpObject();

    // = 연산자 오버로드
    GetdpObject& operator=(const GetdpObject& other);

    void setSelected(bool selected);
    bool isSelected() const;

    // Load mesh-based data from a file
    bool loadData(const QString& filename);

    // Get plot-name
    std::string getName();

    // Render contour plot
    void drawPlot(QMatrix4x4& transform_matrix, float scale_factor = 1);
    void drawLegend(QPainter& painter);
    void setLegendPosition(float x, float y, float width, float height);

    // Make contour line
    void makeContour(int level_count, float min = 0, float max = 0);

    // Make vector
    void makeVector(int level_count, float min = 0, float max = 0);

    bool legendPick(const QPoint& pos); // 범레 선택
    void legendMove(const QPoint& pos); // 범례 이동
    void legendRelease();               // 범례 선택해제

protected:
    template <typename T>
    T min(T a) {
        return a;
    }

    template <typename T, typename... Args>
    T min(T a, Args... args) {
        return std::min(a, min(args...));
    }

    template <typename T>
    T max(T a) {
        return a;
    }

    template <typename T, typename... Args>
    T max(T a, Args... args) {
        return std::max(a, max(args...));
    }

private:
    void parseViewName(const std::string& line);
    void parseScalar(const std::string& line);
    void parseVector(const std::string& line);

    // Render contour plot
    void drawContourPlot(QMatrix4x4& transform_matrix);

    // Render vector plot
    void drawVectorPlot(QMatrix4x4& transform_matrix, float scale_factor);

    QColor getColorForLevel(int level_count, int current_level);

    Arrow generateArrow(const Vertex& origin, const QVector3D& vector, float min = 0, float max = 0);

private:
    struct Data {
        bool type_scalar_ = true;
        std::string name_;
        std::vector<Vertex> nodes_;

        ScalarField scalar_field_;
        std::vector<std::pair<std::array<Vertex, 2>, int>> contour_lines_;

        VectorField vector_field_;
        std::vector<std::pair<Arrow, int>> arrows_;

        std::vector<float> contour_levels_;
        std::vector<QColor> contour_colors_;

        float value_min_ = 0;
        float value_max_ = 0;
    };

    QMutex lock_;
    Data snapshot_data_;
    Data rendering_data_; // 최신 데이터

    bool selected_;
    bool set_minmax_ = false;
    float vector_length_ = 30;
    float legend_width_ = 20.0f;   // 범례 너비 (픽셀)
    float legend_height_ = 150.0f; // 범례 높이 (픽셀)
    float x_start_ = 10.0f;        // 범례 시작 X 위치
    float y_start_ = 35.0f;        // 범례 시작 Y 위치

    QPoint drag_start_position_;  // 드래그 시작 위치
};
