#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QObject>  // 시그널을 위해 필요
#include <QPointer>
#include <QFuture>
#include <QFutureWatcher>
#include <QMutex>
#include "Flat3DShader.h"
#include "Camera.h"
#include "GeomObject.h"
#include "AxisObject.h"
#include "GmshObject.h"
#include "GetdpObject.h"
#include "engine/GeomToTriangle.h"
#include "core/String.h"
#include "core/ref.h"

class TemplateNode;
class PostNode;
class RenderView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    RenderView(QWidget* parent = nullptr);
    ~RenderView();
    void fitToWindow();
    void updateModelSize();
    void setGeometryCSPath(const bzmag::String& geom_path, const bzmag::String& cs_path, const bzmag::String& post_path);

    template <typename T>
    T* makeDrawingObject() {
        static_assert(std::is_base_of<DrawingObject, T>::value, "T must inherit from DrawingObject");

        try {
            return new T(shader_);
        }
        catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to create makeZoomObject: ") + e.what());
        }
    }
    void updateAllObject();
    void updateLink();
    void updatePostLink();

signals:
    void objectPicked(bzmag::Node* node, bool multi_selection);
    void progressUpdated(int value); // 진행률 신호
    void labelUpdated(const QString& message); // 상태 메시지 신호

public slots:
    void selectObject(bzmag::Node* node, bool multi_selection);
    void requestUpdateAllObject(bool now);
    void requestUpdateHeadObject(bzmag::Node* node);
    void requestUpdate();
    
protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

    void prepairPickingBuffer();

    void generateGeomObejct(bzmag::Node* root);
    void clearGeomObjects();

    void generateAxesObejct(bzmag::Node* root);
    void clearAxesObject();

    std::vector<bzmag::engine::GeomHeadNode*> getSelectedHeads() const;

signals:
    void modelerQuery(const QString& query);
    void modelerMove(bzmag::engine::GeomHeadNode* node);
    void modelerRotate(bzmag::engine::GeomHeadNode* node);
    void modelerSplit(bzmag::engine::GeomHeadNode* node);
    void modelerClone(bzmag::engine::GeomHeadNode* node);
    void modelerBoolean(bzmag::engine::GeomHeadNode* node, std::vector<bzmag::engine::GeomHeadNode*> tools, int type);
    void modelerDelete(bzmag::Node* node);

private:
    unsigned int unique_id_;

    Flat3DShader* shader_;
    Camera camera_;
    QVector3D model_min_;
    QVector3D model_max_;
    QPoint last_mouse_position_, press_mouse_position_;
    
    bzmag::String geom_path_;
    bzmag::String cs_path_;
    bzmag::String post_path_;

    std::map<int, GeomObject*> objects_;
    QMatrix4x4 coord_;
    unsigned int default_cs_id_ = 0;
    unsigned int current_cs_id_ = 0;
    unsigned int object_cs_id_ = 0;
    std::map<int, AxisObject*> axes_;

    bool pending_update_request_ = false;
    //bool pending_update_ = false;  // "마지막 호출"을 처리해야 하는지 표시
    QMutex update_mutex_;
    std::atomic<bool> stop_requested_{ false }; // 종료 요청 플래그
    QFuture<void> future_;
    QPointer<QFutureWatcher<void>> watcher_;
    bool is_updating_ = false;  // 업데이트 중인지 여부

    //GeomToTriangleObject* geom_to_triangle_ = nullptr;
    GmshObject* gmsh_obj_ = nullptr;
    std::vector<bzmag::Ref<PostNode>> post_nodes_;
    GetdpObject* selected_getdp_obj_ = nullptr;

    // Picking 관련 변수 선언
    GLuint picking_FBO_        = 0; // 피킹용 프레임버퍼 객체
    GLuint picking_texture_    = 0; // 피킹용 텍스처
    GLuint depth_render_buffer_= 0; // 깊이 렌더버퍼
};
