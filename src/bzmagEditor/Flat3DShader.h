#pragma once

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <spdlog/spdlog.h>

class Flat3DShader
{
public:
    Flat3DShader(QOpenGLFunctions_3_3_Core* functions);
    ~Flat3DShader();

public:
    void use();
    void setTransformation(const QMatrix4x4& transform, const QMatrix4x4& model_transform = QMatrix4x4());
    void setColor(const QColor& color);


    void draw(
        const std::vector<std::array<float, 3>>& nodes,
        const std::vector<std::array<unsigned int, 3>>& elements,
        bool draw_outline = false);
    void draw(
        const std::vector<std::array<float, 3>>& nodes,
        const std::vector<std::array<unsigned int, 2>>& segments);
    void drawLine(const std::array<float, 3>& source, const std::array<float, 3>& target);

private:
    void compileShader(const char* source, GLenum type, GLuint& shader);
    void linkProgram();
    void initializeBuffers();

private:
    QOpenGLFunctions_3_3_Core* functions_ = nullptr;

    GLuint shader_program_ = 0;
    GLuint vertex_shader_ = 0;
    GLuint fragment_shader_ = 0;

    GLint trans_location_ = -1;  // 'transform' uniform의 위치 저장
    GLint model_trans_location_ = -1; // useTransformation 유니폼 위치
    GLint base_color_location_ = -1;      // baseColor 유니폼 위치

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
};
