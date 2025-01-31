#include "GetdpObject.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <QtMath> // qAtan2, qRadiansToDegrees
#include "core/tokenizer.h"

GetdpObject::GetdpObject(Flat3DShader* shader) : DrawingObject(shader)
{

}

GetdpObject::~GetdpObject()
{

}

GetdpObject& GetdpObject::operator=(const GetdpObject& other)
{
    if (this != &other) {
        this->snapshot_data_ = other.snapshot_data_;
        this->rendering_data_ = other.rendering_data_;
    }
    return *this;
}

void GetdpObject::setSelected(bool selected)
{
    selected_ = selected;
}

bool GetdpObject::isSelected() const
{
    return selected_;
}

bool GetdpObject::loadData(const QString& filename)
{
    bzmag::String temp(filename.toStdWString().c_str());
    std::string name(temp.c_str());
    std::ifstream file(name);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << name << std::endl;
        return false;
    }

    rendering_data_.type_scalar_ = true;
    rendering_data_.nodes_.clear();
    rendering_data_.scalar_field_.clear();
    rendering_data_.vector_field_.clear();

    set_minmax_ = false;

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("View ") == 0) {
            parseViewName(line);
        }
        else if (line.find("ST(") == 0) {
            parseScalar(line);
        }
        else if (line.find("VT(") == 0) {
            parseVector(line);
            rendering_data_.type_scalar_ = false;
        }
    }
    file.close();

    {
        QMutexLocker locker(&lock_);
        snapshot_data_.type_scalar_ = rendering_data_.type_scalar_;
        snapshot_data_.nodes_ = rendering_data_.nodes_;
        snapshot_data_.scalar_field_ = rendering_data_.scalar_field_;
        snapshot_data_.vector_field_ = rendering_data_.vector_field_;
        snapshot_data_.value_min_ = rendering_data_.value_min_;
        snapshot_data_.value_max_ = rendering_data_.value_max_;
    }

    if (rendering_data_.scalar_field_.size() > 0 && rendering_data_.name_ == "Az") {
        makeContour(15);
    }
    else if (rendering_data_.vector_field_.size() > 0) {
        makeVector(15);
    }

    return true;
}

std::string GetdpObject::getName()
{
    QMutexLocker locker(&lock_);
    std::string name = snapshot_data_.name_;
    locker.unlock();

    return name;
}

void GetdpObject::makeContour(int level_count, float min, float max)
{
    if (rendering_data_.scalar_field_.empty() || rendering_data_.nodes_.empty()) {
        std::cerr << "Scalar field or nodes are not set." << std::endl;
        return;
    }

    if (rendering_data_.scalar_field_.size() != rendering_data_.nodes_.size()) {
        std::cerr << "Data size of Scalar field and Nodes does not match." << std::endl;
        return;
    }

    rendering_data_.contour_lines_.clear();
    rendering_data_.contour_levels_.clear();
    rendering_data_.contour_colors_.clear();

    float level_min = rendering_data_.value_min_;
    float level_max = rendering_data_.value_max_;
    if (min != 0 || max != 0) {
        level_min = min;
        level_max = max;
    }
    if (level_count < 2) level_count = 2;
    float delta_level = (level_max - level_min) / ((float)(level_count-1));

    for (int i = 0; i < level_count; ++i) {
        float level = level_min + i * delta_level;
        rendering_data_.contour_levels_.push_back(level);

        QColor color = getColorForLevel(level_count, i);
        rendering_data_.contour_colors_.push_back(color);
    }

    for (size_t k = 0; k < rendering_data_.nodes_.size(); k = k + 3) {
        for (int lv = 0; lv < rendering_data_.contour_levels_.size(); ++lv) {
            float level = rendering_data_.contour_levels_[lv];
            std::vector<Vertex> intersections;
            for (int i = 0; i < 3; ++i) {
                int j = (i + 1) % 3;
                float v1 = rendering_data_.scalar_field_[k + i];
                float v2 = rendering_data_.scalar_field_[k + j];
                if ((v1 - level) * (v2 - level) <= 0) {
                    float t = (level - v1) / (v2 - v1);
                    Vertex interpolated{ rendering_data_.nodes_[k + i][0] + t * (rendering_data_.nodes_[k + j][0] - rendering_data_.nodes_[k + i][0]),
                                         rendering_data_.nodes_[k + i][1] + t * (rendering_data_.nodes_[k + j][1] - rendering_data_.nodes_[k + i][1]) };
                    intersections.push_back(interpolated);
                }
            }

            if (intersections.size() == 2) {
                rendering_data_.contour_lines_.push_back({ { intersections[0], intersections[1] }, lv });
            }
        }
    }

    {
        QMutexLocker locker(&lock_);
        snapshot_data_.contour_lines_ = rendering_data_.contour_lines_;
        snapshot_data_.contour_levels_ = rendering_data_.contour_levels_;
        snapshot_data_.contour_colors_ = rendering_data_.contour_colors_;
    }
}

void GetdpObject::makeVector(int level_count, float min, float max)
{
    if (rendering_data_.vector_field_.empty() || rendering_data_.nodes_.empty()) {
        std::cerr << "Vector field or nodes are not set." << std::endl;
        return;
    }

    if (rendering_data_.vector_field_.size() != rendering_data_.nodes_.size()) {
        std::cerr << "Data size of Vector field and Nodes does not match." << std::endl;
        return;
    }

    rendering_data_.arrows_.clear();
    rendering_data_.contour_levels_.clear();
    rendering_data_.contour_colors_.clear();

    float level_min = rendering_data_.value_min_;
    float level_max = rendering_data_.value_max_;
    if (min != 0 || max != 0) {
        level_min = min;
        level_max = max;
    }
    if (level_count < 2) level_count = 2;
    float delta_level = (level_max - level_min) / ((float)(level_count - 1));

    for (int i = 0; i < level_count; ++i) {
        float level = level_min + i * delta_level;
        rendering_data_.contour_levels_.push_back(level);

        QColor color = getColorForLevel(level_count, i);
        rendering_data_.contour_colors_.push_back(color);
    }
    
    for (size_t k = 0; k < rendering_data_.nodes_.size(); k = k + 3) {
        Vertex center{ 0,0,0 };
        QVector3D vector;

        center[0] += rendering_data_.nodes_[k + 0][0];
        center[0] += rendering_data_.nodes_[k + 1][0];
        center[0] += rendering_data_.nodes_[k + 2][0];

        center[1] += rendering_data_.nodes_[k + 0][1];
        center[1] += rendering_data_.nodes_[k + 1][1];
        center[1] += rendering_data_.nodes_[k + 2][1];

        center[0] /= 3.0;
        center[1] /= 3.0;

        vector = (rendering_data_.vector_field_[k + 0] + rendering_data_.vector_field_[k + 1] + rendering_data_.vector_field_[k + 2])/3.0;
        Arrow arrow = generateArrow(center, vector, level_min, level_max);

        float v_length = vector.length();
        int level = int((v_length - level_min) / (level_max - level_min) * (level_count));
        if (level >= level_count) level = level_count - 1;
        if (level < 0) level = 0;

        rendering_data_.arrows_.push_back({ arrow, level });
    }
    {
        QMutexLocker locker(&lock_);
        snapshot_data_.arrows_ = rendering_data_.arrows_;
        snapshot_data_.contour_levels_ = rendering_data_.contour_levels_;
        snapshot_data_.contour_colors_ = rendering_data_.contour_colors_;
    }
}

bool GetdpObject::legendPick(const QPoint& pos) {
    QMutexLocker locker(&lock_);
    bool empty_contour = snapshot_data_.contour_levels_.empty();
    locker.unlock();

    QRect legend_rect(x_start_, y_start_, legend_width_, legend_height_); // 범례 영역

    if (!empty_contour && legend_rect.contains(pos)) {
        setSelected(true);
        drag_start_position_ = pos - QPoint(x_start_, y_start_);
        return true;
    }

    return false;
}

void GetdpObject::legendMove(const QPoint& pos) {
    QPoint new_position = pos - drag_start_position_;
    x_start_ = new_position.x();
    y_start_ = new_position.y();
}

void GetdpObject::legendRelease() {
    setSelected(false);
}

void GetdpObject::parseViewName(const std::string& line) {
    size_t start = line.find('"');
    size_t end = line.find('"', start + 1);
    if (start != std::string::npos && end != std::string::npos) {
        rendering_data_.name_ = line.substr(start + 1, end - start - 1);
    }

    {
        QMutexLocker locker(&lock_);
        snapshot_data_.name_ = rendering_data_.name_;
    }
}

void GetdpObject::parseScalar(const std::string& line) {
    // Find the start and end positions of the parentheses
    size_t start_parentheses = line.find('(');
    size_t end_parentheses = line.find(')', start_parentheses);

    // Extract content inside parentheses
    if (start_parentheses != std::string::npos && end_parentheses != std::string::npos) {
        std::string parentheses_content = line.substr(start_parentheses + 1, end_parentheses - start_parentheses - 1);

        bzmag::Tokenizer token(parentheses_content.c_str(), ',');
        if (token.size() != 9) return;
        Vertex v1{ float(token[0].toDouble() * 1000),float(token[1].toDouble() * 1000), float(token[2].toDouble() * 1000) };
        Vertex v2{ float(token[3].toDouble() * 1000),float(token[4].toDouble() * 1000), float(token[5].toDouble() * 1000) };
        Vertex v3{ float(token[6].toDouble() * 1000),float(token[7].toDouble() * 1000), float(token[8].toDouble() * 1000) };

        // Store parsed values
        rendering_data_.nodes_.push_back(v1);
        rendering_data_.nodes_.push_back(v2);
        rendering_data_.nodes_.push_back(v3);
    }

    // Find the start and end positions of the braces
    size_t start_braces = line.find('{');
    size_t end_braces = line.find('}', start_braces);

    // Extract content inside braces
    if (start_braces != std::string::npos && end_braces != std::string::npos) {
        std::string braces_content = line.substr(start_braces + 1, end_braces - start_braces - 1);

        bzmag::Tokenizer token(braces_content.c_str(), ',');
        if (token.size() != 3) return;

        float value1 = float(token[0].toDouble());
        float value2 = float(token[1].toDouble());
        float value3 = float(token[2].toDouble());

        rendering_data_.scalar_field_.push_back(value1);
        rendering_data_.scalar_field_.push_back(value2);
        rendering_data_.scalar_field_.push_back(value3);

        if (!set_minmax_) {
            rendering_data_.value_min_ = min(value1, value2, value3);
            rendering_data_.value_max_ = max(value1, value2, value3);
            set_minmax_ = true;
        }
        else {
            rendering_data_.value_min_ = min(rendering_data_.value_min_, value1, value2, value3);
            rendering_data_.value_max_ = max(rendering_data_.value_max_, value1, value2, value3);
        }
    }
}

void GetdpObject::parseVector(const std::string& line)
{
    // Find the start and end positions of the parentheses
    size_t start_parentheses = line.find('(');
    size_t end_parentheses = line.find(')', start_parentheses);

    // Extract content inside parentheses
    if (start_parentheses != std::string::npos && end_parentheses != std::string::npos) {
        std::string parentheses_content = line.substr(start_parentheses + 1, end_parentheses - start_parentheses - 1);

        bzmag::Tokenizer token(parentheses_content.c_str(), ',');
        if (token.size() != 9) return;
        Vertex v1{ float(token[0].toDouble() * 1000),float(token[1].toDouble() * 1000), float(token[2].toDouble() * 1000) };
        Vertex v2{ float(token[3].toDouble() * 1000),float(token[4].toDouble() * 1000), float(token[5].toDouble() * 1000) };
        Vertex v3{ float(token[6].toDouble() * 1000),float(token[7].toDouble() * 1000), float(token[8].toDouble() * 1000) };

        // Store parsed values
        rendering_data_.nodes_.push_back(v1);
        rendering_data_.nodes_.push_back(v2);
        rendering_data_.nodes_.push_back(v3);
    }

    // Find the start and end positions of the braces
    size_t start_braces = line.find('{');
    size_t end_braces = line.find('}', start_braces);

    // Extract content inside braces
    if (start_braces != std::string::npos && end_braces != std::string::npos) {
        std::string braces_content = line.substr(start_braces + 1, end_braces - start_braces - 1);

        bzmag::Tokenizer token(braces_content.c_str(), ',');
        if (token.size() != 9) return;
        QVector3D v1{ float(token[0].toDouble()),float(token[1].toDouble()), float(token[2].toDouble()) };
        QVector3D v2{ float(token[3].toDouble()),float(token[4].toDouble()), float(token[5].toDouble()) };
        QVector3D v3{ float(token[6].toDouble()),float(token[7].toDouble()), float(token[8].toDouble()) };

        // Store parsed values
        rendering_data_.vector_field_.push_back(v1);
        rendering_data_.vector_field_.push_back(v2);
        rendering_data_.vector_field_.push_back(v3);

        if (!set_minmax_) {
            rendering_data_.value_min_ = min(v1.length(), v2.length(), v3.length());
            rendering_data_.value_max_ = max(v1.length(), v2.length(), v3.length());
            set_minmax_ = true;
        }
        else {
            rendering_data_.value_min_ = min(rendering_data_.value_min_, v1.length(), v2.length(), v3.length());
            rendering_data_.value_max_ = max(rendering_data_.value_max_, v1.length(), v2.length(), v3.length());
        }
    }
}

void GetdpObject::drawPlot(QMatrix4x4& transform_matrix, float scale_factor)
{
    QMutexLocker locker(&lock_);
    bool type_scalar = snapshot_data_.type_scalar_;
    locker.unlock();

    if (type_scalar) {
        drawContourPlot(transform_matrix);
    }
    else {
        drawVectorPlot(transform_matrix, scale_factor);
    }
}

void GetdpObject::drawLegend(QPainter& painter) {
    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<float> contour_levels = snapshot_data_.contour_levels_;
    std::string name = snapshot_data_.name_;
    locker.unlock();

    if (contour_levels.empty()) {
        return;
    }

    float segment_height = legend_height_ / static_cast<float>(contour_levels.size());
    painter.setRenderHint(QPainter::Antialiasing);

    // 범례 테두리 추가 (선택된 경우)
    if (isSelected()) {
        painter.setPen(QPen(Qt::blue, 2)); // 선택된 상태를 파란 테두리로 표시
        painter.drawRect(QRectF(x_start_ - 2, y_start_ - 2, legend_width_ + 4, legend_height_ + 4));
    }

    // 범례 이름 그리기
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 12, QFont::Bold));
    painter.drawText(QRectF(x_start_ - 40.0f, y_start_ - 25.0f, legend_width_ + 80.0f, 20.0f), Qt::AlignCenter, QString(name.c_str()));

    // 컬러맵 그리기
    for (size_t i = 0; i < contour_levels.size(); ++i) {
        QColor color = getColorForLevel(int(contour_levels.size()),int(contour_levels.size() - 1 - i));
        painter.setBrush(QBrush(color));
        painter.setPen(Qt::NoPen);
        float y_pos = y_start_ + i * segment_height;
        painter.drawRect(QRectF(x_start_, y_pos, legend_width_, segment_height));
    }

    // 텍스트 추가
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", 10));
    painter.drawText(QPointF(x_start_ + legend_width_ + 10.0f, y_start_ + 10.0f), QString::number(contour_levels.back(), 'g', 4));
    painter.drawText(QPointF(x_start_ + legend_width_ + 10.0f, y_start_ + legend_height_ / 2.0f), QString::number(contour_levels[contour_levels.size() / 2], 'g', 4));
    painter.drawText(QPointF(x_start_ + legend_width_ + 10.0f, y_start_ + legend_height_ - 5.0f), QString::number(contour_levels.front(), 'g', 4));
}

void GetdpObject::setLegendPosition(float x, float y, float width, float height)
{
    x_start_ = x;
    y_start_ = y;
    legend_width_ = width;
    legend_height_ = height;
}

void GetdpObject::drawContourPlot(QMatrix4x4& transform_matrix) {
    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<std::pair<std::array<Vertex, 2>, int>> contour_lines = snapshot_data_.contour_lines_;
    std::vector<QColor> contour_colors = snapshot_data_.contour_colors_;
    locker.unlock();

    if (contour_lines.size() == 0) return;

    //shader_->setUseContour(true);
    shader_->use();
    shader_->setTransformation(transform_matrix);
    for (const auto& contour : contour_lines) {
        const Vertex& source = contour.first[0];
        const Vertex& target = contour.first[1];
        QColor color = contour_colors[contour.second];
        shader_->setColor(color);
        shader_->drawLine(source, target);
    }
    //shader_->setUseContour(false);
}

void GetdpObject::drawVectorPlot(QMatrix4x4& transform_matrix, float scale_factor) {
    // 스냅샷 복사
    QMutexLocker locker(&lock_);
    std::vector<std::pair<Arrow, int>> arrows = snapshot_data_.arrows_;
    std::vector<QColor> contour_colors = snapshot_data_.contour_colors_;
    locker.unlock();

    shader_->use();
    
    // Draw arrows representing vectors
    for (const auto& arrow : arrows ) {

        auto origin = arrow.first.origin;
        QMatrix4x4 trans;
        trans.translate(origin[0], origin[1]);
        trans.scale(vector_length_ / scale_factor);
        shader_->setTransformation(transform_matrix, trans);

        QColor color = contour_colors[arrow.second];
        shader_->setColor(color);

        auto nodes = arrow.first.nodes;
        shader_->draw(nodes, arrow.first.elements);
    }
}

QColor GetdpObject::getColorForLevel(int level_count, int current_level) {
    if (level_count < 2) {
        // 레벨이 하나만 있다면 단일 색상 반환
        return QColor(0, 0, 255); // 파랑
    }

    // 현재 레벨의 비율 계산 (0.0 ~ 1.0)
    float ratio = static_cast<float>(current_level) / (level_count - 1);

    // 구간별 색상 보간 (파랑 → 청록 → 초록 → 노랑 → 빨강)
    int red = 0, green = 0, blue = 0;

    if (ratio < 0.25f) {
        // 파랑 → 청록 (0.0 ~ 0.25)
        float segment_ratio = ratio / 0.25f;
        red = 0;
        green = static_cast<int>(255 * segment_ratio);
        blue = 255;
    }
    else if (ratio < 0.5f) {
        // 청록 → 초록 (0.25 ~ 0.5)
        float segment_ratio = (ratio - 0.25f) / 0.25f;
        red = 0;
        green = 255;
        blue = static_cast<int>(255 * (1.0f - segment_ratio));
    }
    else if (ratio < 0.75f) {
        // 초록 → 노랑 (0.5 ~ 0.75)
        float segment_ratio = (ratio - 0.5f) / 0.25f;
        red = static_cast<int>(255 * segment_ratio);
        green = 255;
        blue = 0;
    }
    else {
        // 노랑 → 빨강 (0.75 ~ 1.0)
        float segment_ratio = (ratio - 0.75f) / 0.25f;
        red = 255;
        green = static_cast<int>(255 * (1.0f - segment_ratio));
        blue = 0;
    }

    return QColor(red, green, blue);
}

GetdpObject::Arrow GetdpObject::generateArrow(const Vertex& origin, const QVector3D& vector, float min, float max)
{
    float current_size = vector.length();
    float size_ratio = (current_size - min) / (max - min);
    if (size_ratio < 0) size_ratio = 0;

    // y축기준 사이즈 1 화살표를 만든다
    Arrow arrow;
    arrow.origin = origin;
    if (size_ratio < 0.01) return arrow;

    std::pair<std::vector<Vertex>, std::vector<Triangle>> cone = generateCone(0.1f, 0.4f, 0.6f, 6);
    std::pair<std::vector<Vertex>, std::vector<Triangle>> cylinder = generateCylinder(0.015f, 0.6f, 0.0f, 6);

    // 1. 벡터 사이즈 설정
    QMatrix4x4 trans;
    trans.scale(size_ratio*2);

    // 2. 벡터 방향 설정
    QVector3D normalized_dir = vector.normalized(); // 방향 벡터 정규화
    float angle = qRadiansToDegrees(qAtan2(normalized_dir.y(), normalized_dir.x()) - CGAL_PI/2);
    trans.rotate(angle, 0, 0, 1); // z축 기준으로 회전 (z축 고정)


    for (auto& vert1 : cone.first) {
        QVector4D trans_vert = trans * (QVector4D{ vert1[0], vert1[1], vert1[2], 1.0f });
        Vertex new_node{ trans_vert.x(), trans_vert.y(), trans_vert.z() };
        arrow.nodes.push_back(new_node);
    }
    for (auto& tri1 : cone.second) {
        arrow.elements.push_back(tri1);
    }

    for (auto& vert2 : cylinder.first) {
        QVector4D trans_vert = trans * (QVector4D{ vert2[0], vert2[1], vert2[2], 1.0f });
        Vertex new_node{ trans_vert.x(), trans_vert.y(), trans_vert.z() };
        arrow.nodes.push_back(new_node);
    }
    unsigned int offset = (unsigned int)cone.first.size();
    for (auto& tri2 : cylinder.second) {
        tri2[0] += offset;
        tri2[1] += offset;
        tri2[2] += offset;
        arrow.elements.push_back(tri2);
    }

    return arrow;
}
