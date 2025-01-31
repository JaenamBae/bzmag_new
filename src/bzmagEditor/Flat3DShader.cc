#include "Flat3DShader.h"
#include <QColor>
#include <array>

Flat3DShader::Flat3DShader(QOpenGLFunctions_3_3_Core* functions)
    : functions_(functions)
{
    // Vertex Shader 소스 코드
    const char* vertex_shader_source = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        uniform mat4 transformation;
        uniform mat4 modelTransformation;

        void main() {
            vec4 worldPosition = modelTransformation * vec4(position, 1.0);
            gl_Position = transformation * worldPosition;

            // 강제 사용을 보장
            if (modelTransformation[0][0] == 0.0) {
                gl_Position = vec4(0.0); // 의미 없는 코드, 최적화 방지용
            }
        }
        )";

    // Fragment Shader 소스 코드
    const char* fragment_shader_source = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec4 baseColor;

        void main() {
            FragColor = baseColor;
        }
    )";

    // 셰이더 컴파일 및 링크
    compileShader(vertex_shader_source, GL_VERTEX_SHADER, vertex_shader_);
    compileShader(fragment_shader_source, GL_FRAGMENT_SHADER, fragment_shader_);
    linkProgram();

    // VAO 및 VBO 초기화
    initializeBuffers();

    // Uniform 위치 저장
    trans_location_ = functions_->glGetUniformLocation(shader_program_, "transformation");
    if (trans_location_ == -1) {
        spdlog::warn("Uniform 'transformation' not found or optimized out.");
    }
    model_trans_location_ = functions_->glGetUniformLocation(shader_program_, "modelTransformation");
    if (model_trans_location_ == -1) {
        spdlog::warn("Uniform 'modelTransformation' not found or optimized out.");
    }
    base_color_location_ = functions_->glGetUniformLocation(shader_program_, "baseColor");
    if (base_color_location_ == -1) {
        spdlog::warn("Uniform 'baseColor' not found or optimized out.");
    }
}

Flat3DShader::~Flat3DShader() 
{
    functions_->glDeleteProgram(shader_program_);
}

void Flat3DShader::use() 
{
    functions_->glUseProgram(shader_program_);
}

void Flat3DShader::setTransformation(const QMatrix4x4& transform, const QMatrix4x4& model_transform)
{
    if (trans_location_ != -1) {
        functions_->glUniformMatrix4fv(trans_location_, 1, GL_FALSE, transform.constData());
    }
    else {
        spdlog::warn("Uniform 'transformation' not found or optimized out.");
    }
    if (model_trans_location_ != -1) {
        functions_->glUniformMatrix4fv(model_trans_location_, 1, GL_FALSE, model_transform.constData());
    }
    else {
        spdlog::warn("Uniform 'modelTransformation' not found or optimized out.");
    }
}

void Flat3DShader::setColor(const QColor& color)
{
    if (base_color_location_ != -1) {
        functions_->glUniform4f(base_color_location_, color.redF(), color.greenF(), color.blueF(), color.alphaF());
    }
    else {
        spdlog::warn("Uniform 'baseColor' not found or optimized out.");
    }
}

void Flat3DShader::draw(const std::vector<std::array<float, 3>>& nodes,
                        const std::vector<std::array<unsigned int, 2>>& segments)
{
    use(); // 셰이더 프로그램 사용
    functions_->glBindVertexArray(vao_); // VAO 바인딩

    // 세그먼트 그리기
    if (!segments.empty()) {
        std::vector<float> segment_vertices;
        for (const auto& seg : segments) {
            segment_vertices.insert(segment_vertices.end(), {
                nodes[seg[0]][0], nodes[seg[0]][1], nodes[seg[0]][2],
                nodes[seg[1]][0], nodes[seg[1]][1], nodes[seg[1]][2]
                });
        }

        // VBO에 데이터 전송
        functions_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        functions_->glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(segment_vertices.size() * sizeof(float)),
            segment_vertices.data(), GL_STATIC_DRAW);

        // 선 그리기
        functions_->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(segment_vertices.size() / 3));
    }

    // VAO 바인딩 해제
    functions_->glBindVertexArray(0);

    GLenum error = functions_->glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("OpenGL draw error: {}", error);
    }
}

void Flat3DShader::drawLine(const std::array<float, 3>& source, const std::array<float, 3>& target)
{
    use(); // 셰이더 프로그램 사용
    functions_->glBindVertexArray(vao_); // VAO 바인딩

    // 두 점의 데이터를 VBO에 전송
    float line_vertices[] = {
        source[0], source[1], source[2],
        target[0], target[1], target[2]
    };

    functions_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    functions_->glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices, GL_STATIC_DRAW);

    // 선 그리기
    functions_->glDrawArrays(GL_LINES, 0, 2);

    // VAO 바인딩 해제
    functions_->glBindVertexArray(0);

    GLenum error = functions_->glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("OpenGL draw error: {}", error);
    }
}

void Flat3DShader::draw(const std::vector<std::array<float, 3>>& nodes,
    const std::vector<std::array<unsigned int, 3>>& elements,
    bool draw_outline)
{
    use(); // 셰이더 프로그램 사용
    functions_->glBindVertexArray(vao_); // VAO 바인딩

    if (draw_outline) {
        // 외곽선(라인) 그리기
        std::vector<float> line_vertices;
        for (const auto& elem : elements) {
            for (int i = 0; i < 3; ++i) {
                line_vertices.insert(line_vertices.end(), {
                    nodes[elem[i]][0], nodes[elem[i]][1], nodes[elem[i]][2],
                    nodes[elem[(i + 1) % 3]][0], nodes[elem[(i + 1) % 3]][1], nodes[elem[(i + 1) % 3]][2]
                    });
            }
        }

        functions_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        functions_->glBufferData(GL_ARRAY_BUFFER, line_vertices.size() * sizeof(float), line_vertices.data(), GL_STATIC_DRAW);

        // GL_LINES 모드로 외곽선을 그립니다
        functions_->glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(line_vertices.size() / 3));
    }
    else {
        // 삼각형 메쉬 그리기
        std::vector<float> element_vertices;
        for (const auto& elem : elements) {
            for (int i = 0; i < 3; ++i) {
                element_vertices.insert(element_vertices.end(), {
                    nodes[elem[i]][0], nodes[elem[i]][1], nodes[elem[i]][2]
                    });
            }
        }

        functions_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        functions_->glBufferData(GL_ARRAY_BUFFER, element_vertices.size() * sizeof(float), element_vertices.data(), GL_STATIC_DRAW);

        // GL_TRIANGLES 모드로 면을 그립니다
        functions_->glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(element_vertices.size() / 3));
    }

    functions_->glBindVertexArray(0);

    GLenum error = functions_->glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("OpenGL error: {}", error);
    }
}

void Flat3DShader::compileShader(const char* source, GLenum type, GLuint& shader)
{
    shader = functions_->glCreateShader(type);
    functions_->glShaderSource(shader, 1, &source, nullptr);
    functions_->glCompileShader(shader);

    // 컴파일 오류 체크
    GLint success;
    functions_->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        functions_->glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        spdlog::error("Shader compilation failed:\n{}", infoLog);
    }
}

void Flat3DShader::linkProgram()
{
    shader_program_ = functions_->glCreateProgram();
    functions_->glAttachShader(shader_program_, vertex_shader_);
    functions_->glAttachShader(shader_program_, fragment_shader_);
    functions_->glLinkProgram(shader_program_);

    GLint success;
    functions_->glGetProgramiv(shader_program_, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        functions_->glGetProgramInfoLog(shader_program_, 512, nullptr, infoLog);
        spdlog::error("Shader linking failed: {}", infoLog);
    }

    functions_->glDeleteShader(vertex_shader_);
    functions_->glDeleteShader(fragment_shader_);
}

void Flat3DShader::initializeBuffers()
{
    functions_->glGenVertexArrays(1, &vao_);
    functions_->glGenBuffers(1, &vbo_);

    if (!vao_ || !vbo_) {
        spdlog::error("Failed to generate VAO or VBO. VAO: {}, VBO: {}", vao_, vbo_);
    }

    functions_->glBindVertexArray(vao_);
    functions_->glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    functions_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    functions_->glEnableVertexAttribArray(0);

    functions_->glBindBuffer(GL_ARRAY_BUFFER, 0);
    functions_->glBindVertexArray(0);

    GLenum error = functions_->glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("Error during VAO/VBO initialization: {}", error);
    }
}