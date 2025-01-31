#include "RenderView.h"
#include <QMenu>
#include <QMessageBox>
#include <QtConcurrent>
#include <QFutureWatcher>
#include "Modeler.h"
#include "TemplateNode.h"
#include "core/kernel.h"
#include "engine/GeomBaseNode.h"
#include "engine/GeomHeadNode.h"
#include "engine/GeomToSurfaceMesh.h"

using namespace bzmag;
using namespace bzmag::engine;

RenderView::RenderView(QWidget* parent) : 
    QOpenGLWidget(parent), unique_id_(1), camera_()
{
    setFocusPolicy(Qt::StrongFocus);
}

RenderView::~RenderView()
{
    delete shader_; // 동적으로 할당한 셰이더 삭제

    // Objects 삭제
    clearGeomObjects();

    // 좌표계 삭제
    clearAxesObject();

    // OpenGL 리소스 해제
    glDeleteFramebuffers(1, &picking_FBO_);
    glDeleteTextures(1, &picking_texture_);
    glDeleteRenderbuffers(1, &depth_render_buffer_);
}

void RenderView::fitToWindow() 
{ 
    updateModelSize();
    camera_.fitWindow(coord_); 
    for (auto& [key, obj] : objects_) {
        if (obj) {
            obj->generateScaleFactorDenpendantData(camera_.getZoomScale());
        }
    }
}

void RenderView::updateModelSize()
{
    model_min_ = model_max_ = QVector3D{ 0.0f, 0.0f, 0.0f };
    for (auto it = objects_.begin(); it != objects_.end(); ++it)
    {
        GeomObject* obj = it->second;
        GeomHeadNode* head = obj->getHead();
        if (head->isHide()) continue;

        // Update the overall bounding box of the model
        obj->setReferedMatrix(coord_);
        QVector3D shape_min = obj->getMinBounds();
        QVector3D shape_max = obj->getMaxBounds();

        // Expand the model's bounding box to include the new shape
        model_min_.setX(std::min(model_min_.x(), shape_min.x()));
        model_min_.setY(std::min(model_min_.y(), shape_min.y()));
        model_min_.setZ(std::min(model_min_.z(), shape_min.z()));

        model_max_.setX(std::max(model_max_.x(), shape_max.x()));
        model_max_.setY(std::max(model_max_.y(), shape_max.y()));
        model_max_.setZ(std::max(model_max_.z(), shape_max.z()));
    }

    camera_.setModelSize(model_min_, model_max_);
}

void RenderView::setGeometryCSPath(const String& geom_path, const String& cs_path, const bzmag::String& post_path)
{
    geom_path_ = geom_path;
    cs_path_ = cs_path;
    post_path_ = post_path;
    updateLink();
    updatePostLink();
}

void RenderView::updateLink()
{
    Ref<Node> geom_root = Kernel::instance()->lookup(geom_path_);
    if (geom_root.invalid()) return;

    Ref<Node> cs_root = Kernel::instance()->lookup(cs_path_);
    if (cs_root.invalid()) return;

    clearGeomObjects();
    generateGeomObejct(geom_root);

    clearAxesObject();
    generateAxesObejct(cs_root);

    //updateModelSize();
    fitToWindow();
}

void RenderView::updatePostLink()
{
    Ref<Node> post_root = Kernel::instance()->lookup(post_path_);
    if (post_root.invalid()) return;

    post_nodes_.clear();
    bool selected_exist = false;
    for (auto it = post_root->firstChildNode(); it != post_root->lastChildNode(); ++it)
    {
        Node* nn = *it;
        PostNode* post_node = dynamic_cast<PostNode*>(nn);
        if (post_node) {
            post_nodes_.push_back(post_node);
            if (post_node->getDrawingObject() == selected_getdp_obj_) {
                selected_exist = true;
            }
        }
    }
    if (!selected_exist) {
        selected_getdp_obj_ = nullptr;
    }

    update();
}

void RenderView::selectObject(bzmag::Node* node, bool multi_selection)
{
    object_cs_id_ = 0;

    // 다중선택이 아닌경우 Geometry 선택상태 초기화
    if (!multi_selection) {
        for (auto& it : objects_) {
            it.second->setSelected(false);
        }
    }

    // Geometry 인 경우
    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(node);
    if(head) {
        for (auto& it : objects_) {
            if (it.second->getID() == head->getID()) {
                it.second->setSelected(true);
                CSNode* cs = dynamic_cast<CSNode*>(head->getReferedCS());
                if (cs) {
                    object_cs_id_ = cs->getID();
                }
                break;
            }
        }
    }

    GeomPrimitiveNode* prim = dynamic_cast<GeomPrimitiveNode*>(node);
    if (prim) {
        for (auto& it : objects_) {
            if (it.second->getID() == prim->getHeadNode()->getID()) {
                CSNode* cs = dynamic_cast<CSNode*>(prim->getReferedCS());
                if (cs) {
                    object_cs_id_ = cs->getID();
                }
                break;
            }
        }
    }

    // CS인 경우
    CSNode* cs = dynamic_cast<CSNode*>(node);
    if (cs) {
        if (axes_.find(cs->getID()) != axes_.end()) {
            current_cs_id_ = cs->getID();
        }
        else {
            current_cs_id_ = default_cs_id_;
        }
        if (axes_.find(current_cs_id_) != axes_.end()) {
            coord_ = axes_[current_cs_id_]->getMatrix();
        }
    }

    // Excitation인 경우
    CoilNode* coil = dynamic_cast<CoilNode*>(node);
    if (coil) {
        GeomHeadNode* ref_head = coil->getReferenceNode();
        if (ref_head) {
            for (auto& it : objects_) {
                if (it.second->getID() == ref_head->getID()) {
                    it.second->setSelected(true);
                    CSNode* cs = dynamic_cast<CSNode*>(ref_head->getReferedCS());
                    if (cs) {
                        object_cs_id_ = cs->getID();
                    }
                    break;
                }
            }
        }
    }

    // GeomHeadRefNode 인 경우
    GeomHeadRefNode* refhead = dynamic_cast<GeomHeadRefNode*>(node);
    if (refhead) {
        GeomHeadNode* ref_head = refhead->getHeadNode();
        if (ref_head) {
            for (auto& it : objects_) {
                if (it.second->getID() == ref_head->getID()) {
                    it.second->setSelected(true);
                    CSNode* cs = dynamic_cast<CSNode*>(ref_head->getReferedCS());
                    if (cs) {
                        object_cs_id_ = cs->getID();
                    }
                    break;
                }
            }
        }
    }

    // GeomSpiltNode의 경우
    GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(node);
    if (spilt) {
        CSNode* cs = spilt->getReferedCS();
        if (cs) {
            object_cs_id_ = cs->getID();
        }
    }

    update();
}

void RenderView::updateAllObject()
{
    QMutexLocker locker(&update_mutex_); // Qt의 QMutex로 동기화

    if (is_updating_) return; // 이미 업데이트 중이라면 무시
    is_updating_ = true;

    emit labelUpdated("Starting update..."); // 작업 시작 메시지
    emit progressUpdated(0);                 // 진행률 초기화

    if (watcher_) {
        watcher_->cancel(); // 현재 작업 취소
        watcher_->waitForFinished(); // 기존 작업 종료 대기
        watcher_->deleteLater();
        watcher_ = nullptr;
    }

    watcher_ = new QFutureWatcher<void>(this);

    stop_requested_ = false; // 중단 요청 초기화

    future_ = QtConcurrent::run([this]() {
        const int total_steps = (int)(axes_.size() + objects_.size());
        int current_step = 0;

        for (auto& [key, cs] : axes_) {
            if (stop_requested_.load()) return; // 종료 요청 시 중단
            cs->update();
            int progress = int(((float)(++current_step) / (float)(total_steps)) * 100);
            emit progressUpdated(progress);
        }

        for (auto& [key, geom] : objects_) {
            if (stop_requested_.load()) return; // 종료 요청 시 중단
            geom->update();
            geom->generateScaleFactorDenpendantData(camera_.getZoomScale());
            int progress = int(((float)(++current_step) / (float)(total_steps)) * 100);
            emit progressUpdated(progress);
        }
    });

    connect(watcher_, &QFutureWatcher<void>::finished, this, [this]() {
        updateModelSize();                      // 모델 사이즈 갱신
        update();                               // 화면 갱신 (메인 스레드)
        emit progressUpdated(100);             // 진행률 100%
        emit labelUpdated("Complete update..."); // 작업 완료 메시지

        watcher_->deleteLater();
        watcher_ = nullptr;
        is_updating_ = false; // 업데이트 완료
    });

    watcher_->setFuture(future_);
}

void RenderView::requestUpdateAllObject(bool now)
{
    if (now) {
        updateAllObject();  // 즉시 업데이트 실행
    }
    else {
        pending_update_request_ = true;
    }
}

void RenderView::requestUpdateHeadObject(bzmag::Node* node)
{
    auto object = objects_.find(node->getID());
    if (object != objects_.end()) {
        object->second->update();
        update();
    }
}

void RenderView::requestUpdate()
{
    update();
}

void RenderView::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);    // 깊이 테스트 활성화
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 알파 블렌딩 설정
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);   // 어두운 회색 배경

    shader_ = new Flat3DShader(this); // OpenGL 컨텍스트 공유
    shader_->use(); // 셰이더 프로그램 활성화

    // FBO 설정
    prepairPickingBuffer();
}

void RenderView::paintGL()
{
    Modeler* modeler = Modeler::instance();

    makeCurrent(); // OpenGL 컨텍스트 활성화
    QMatrix4x4 transform = camera_.projection_transformation();

    // 일반 렌더링 (화면 렌더링용)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST); // 깊이 테스트 비활성화

    GeomObject* selected_obj = nullptr;

    // 오브젝트 면 그리기
    for (auto& it : objects_) {
        it.second->drawSurface(transform);
        if (it.second->isSelected()) selected_obj = it.second;
    }

    // 오브젝트 외곽선 그리기
    for (auto& it : objects_) {
        it.second->drawBoundary(transform);
    }

    //qreal dpi_scale = this->devicePixelRatio();
    // 오브젝트 절점
    if (modeler->getDrawVertices()) {
        for (auto& it : objects_) {
            it.second->drawVertices(transform);
        }
    }

    // 선택된 오브젝트는 마지막으로 한번더 그려준다
    if (selected_obj) {
        selected_obj->drawSurface(transform, false, true);
        selected_obj->drawBoundary(transform, true);
        if (modeler->getDrawVertices()) {
            selected_obj->drawVertices(transform, true);
        }
    }

    // 기타 Drawing Objects 그리기
    for (auto& node : post_nodes_) {
        if (node->getDrawingFlag()) {

            // GmshObject인 경우
            GmshObject* gmsh_obj = dynamic_cast<GmshObject*>(node->getDrawingObject());
            if (gmsh_obj) {
                gmsh_obj->drawMesh(transform);
            }

            // GetdpObject인 경우 
            GetdpObject* getdp_obj = dynamic_cast<GetdpObject*>(node->getDrawingObject());
            if (getdp_obj) {
                getdp_obj->drawPlot(transform, camera_.getZoomScale());
            }
        }
    }

    // 좌표계 그리기
    if (axes_.find(current_cs_id_) != axes_.end()) {
        axes_[current_cs_id_]->draw(transform, camera_.getZoomScale());
    }

    if (axes_.find(object_cs_id_) != axes_.end()) {
        axes_[object_cs_id_]->draw(transform, camera_.getZoomScale());
    }

    glEnable(GL_DEPTH_TEST); // 깊이 테스트 활성화

    // 비가시 피킹 렌더링 (오브젝트 피킹용)
    glBindFramebuffer(GL_FRAMEBUFFER, picking_FBO_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    for (auto& it : objects_) {
        it.second->drawForPicking(transform);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        spdlog::error("OpenGL error: {}", error);
    }
    
    // 기타 Drawing Objects의 범례 그리기; QPaint 사용
    // OpenGL 렌더링 끝나고 호출
    for (auto& node : post_nodes_) {
        if (node->getDrawingFlag()) {
            // GetdpObject인 경우 
            GetdpObject* getdp_obj = dynamic_cast<GetdpObject*>(node->getDrawingObject());
            if (getdp_obj) {
                // 범례 그리기 - opengl 함수 아닌 QPaint 사용
                QPainter painter(this);
                painter.beginNativePainting();
                getdp_obj->drawLegend(painter);
                painter.endNativePainting();
            }
        }
    }
}

void RenderView::resizeGL(int w, int h)
{
    float ratio = devicePixelRatioF();
    glViewport(0, 0, static_cast<int>(w * ratio), static_cast<int>(h * ratio));
    camera_.reshape(QRect(0, 0, w, h));
    for (auto& [key, obj] : objects_) {
        if (obj) {
            obj->generateScaleFactorDenpendantData(camera_.getZoomScale());
        }
    }

    // 기존 FBO, 텍스처 및 렌더버퍼 삭제
    glDeleteFramebuffers(1, &picking_FBO_);
    glDeleteTextures(1, &picking_texture_);
    glDeleteRenderbuffers(1, &depth_render_buffer_);

    // 새로운 FBO, 텍스처 및 렌더버퍼 생성 및 설정
    prepairPickingBuffer();
    update();
}

void RenderView::mousePressEvent(QMouseEvent* event)
{
    if (pending_update_request_) {
        pending_update_request_ = false; // 플래그 초기화
        updateAllObject();          // 업데이트 실행
        return;
    }

    press_mouse_position_ = last_mouse_position_ = event->pos();

    for (auto& node : post_nodes_) {
        if (event->button() != Qt::LeftButton) return;

        GetdpObject* getdp_obj = dynamic_cast<GetdpObject*>(node->getDrawingObject());
        if (getdp_obj) {
            if (getdp_obj->legendPick(event->pos())) {
                selected_getdp_obj_ = getdp_obj;
                update();
                break;
            }
        }
    }
}

void RenderView::mouseReleaseEvent(QMouseEvent* event)
{
    if (selected_getdp_obj_) {
        if (event->button() != Qt::LeftButton) return;

        selected_getdp_obj_->legendRelease(); // GetdpObject에서 처리
        selected_getdp_obj_ = nullptr;
    }
    else {
        if (event->button() != Qt::LeftButton && press_mouse_position_ == event->pos()) {

            std::vector<bzmag::engine::GeomHeadNode*> heads = getSelectedHeads();

            QMenu contextMenu(this);
            contextMenu.addAction("Reset viewport", this, [this]() {
                fitToWindow();
            });
            contextMenu.addAction("Reload", this, [this]() {
                updateLink();
            });
            contextMenu.addSeparator();

            if (heads.size() == 1) {
                bzmag::engine::GeomHeadNode* head = heads.front();
                contextMenu.addAction("Move", this, [this, head]() {
                    emit modelerMove(head);
                });
                contextMenu.addAction("Rotate", this, [this, head]() {
                    emit modelerRotate(head);
                });
                contextMenu.addAction("Split", this, [this, head]() {
                    emit modelerSplit(head);
                });
                contextMenu.addAction("Clone", this, [this, head]() {
                    emit modelerClone(head);
                });
                contextMenu.addSeparator();

                // "Assign BC" 하위 메뉴 생성
                QMenu* assignBCMenu = contextMenu.addMenu("Assign to BC");
                assignBCMenu->addAction("Assign to Fixed BC", this, [this]() {
                    emit modelerQuery("FixedBC");
                });
                assignBCMenu->addAction("Assign to Master BC", this, [this]() {
                    emit modelerQuery("MasterBC");
                });
                assignBCMenu->addAction("Assign to Slave BC", this, [this]() {
                    emit modelerQuery("SlaveBC");
                });
                assignBCMenu->addAction("Assign to Moving Band", this, [this]() {
                    emit modelerQuery("MovingBand");
                });

                contextMenu.addSeparator();
                contextMenu.addAction("Delete", this, [this, head]() {
                    emit modelerDelete(head);
                });
            }
            else if (heads.size() > 1) {
                bzmag::engine::GeomHeadNode* from = nullptr;
                std::vector<bzmag::engine::GeomHeadNode*> tools;
                for (auto& head : heads) {
                    if (!from) from = head;
                    else {
                        tools.push_back(head);
                    }
                }
                if (tools.size() > 0) {
                    contextMenu.addAction("Subtract", this, [this, from, tools]() {
                        emit modelerBoolean(from, tools, 1);
                    });
                    contextMenu.addAction("Unite", this, [this, from, tools]() {
                        emit modelerBoolean(from, tools, 2);
                    });
                    contextMenu.addAction("Intersect", this, [this, from, tools]() {
                        emit modelerBoolean(from, tools, 3);
                    });
                    contextMenu.addSeparator();
                    // "Assign BC" 하위 메뉴 생성
                    QMenu* assignBCMenu = contextMenu.addMenu("Assign to BC");
                    assignBCMenu->addAction("Assign to Fixed BC", this, [this]() {
                        emit modelerQuery("FixedBC");
                    });
                    assignBCMenu->addAction("Assign to Master BC", this, [this]() {
                        emit modelerQuery("MasterBC");
                    });
                    assignBCMenu->addAction("Assign to Slave BC", this, [this]() {
                        emit modelerQuery("SlaveBC");
                    });
                }
            }
            contextMenu.exec(mapToGlobal(event->pos()));
            return;

        }
        else if (event->button() == Qt::LeftButton && press_mouse_position_ == event->pos())
        {
            // FBO 바인딩 및 픽셀 읽기
            glBindFramebuffer(GL_FRAMEBUFFER, picking_FBO_);
            int x = static_cast<int>(event->pos().x() * devicePixelRatioF());
            int y = static_cast<int>((height() - event->pos().y()) * devicePixelRatioF());
            unsigned char pixelColor[4];
            glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixelColor);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // 색상에서 ID 추출
            unsigned int pickedID = (pixelColor[0] << 16) | (pixelColor[1] << 8) | pixelColor[2];

            // 객체 선택
            Node* node = nullptr;
            if (objects_.find(pickedID) != objects_.end()) {
                node = objects_[pickedID]->getHead();
            }
            if (event->modifiers() & Qt::ControlModifier) {
                selectObject(node, true);
                emit objectPicked(node, true);
            }
            else {
                selectObject(node, false);
                emit objectPicked(node, false);
            }
        }
    }
    update();
}

void RenderView::mouseMoveEvent(QMouseEvent* event)
{
    if (selected_getdp_obj_) {
        if (event->buttons() & Qt::LeftButton) {
            selected_getdp_obj_->legendMove(event->pos()); // GetdpObject에서 처리
            update();
        }
    }
    else {
        bool flag_dirty = false;
        QPoint delta = event->pos() - last_mouse_position_;
        if (event->buttons() & Qt::LeftButton) {
            camera_.pan(QPointF(delta.x(), delta.y()));
            flag_dirty = true;
        }
        else if (event->buttons() & Qt::RightButton) {
            //qreal dpi_scale = this->devicePixelRatio();
            camera_.rotate(QPointF(delta.x() * 0.3f, delta.y() * 0.3f));
            flag_dirty = true;
        }
        last_mouse_position_ = event->pos();
        if(flag_dirty) update();
    }
}

void RenderView::wheelEvent(QWheelEvent* event)
{
    float zoom_factor = 1.0f + event->angleDelta().y() / 1080.0f; // 줌 속도 조절
    camera_.zoom(zoom_factor);
    for (auto& [key, obj] : objects_) {
        if (obj) {
            obj->generateScaleFactorDenpendantData(camera_.getZoomScale());
        }
    }
    update();
}

void RenderView::keyPressEvent(QKeyEvent* event)
{
    switch (event->key()) {
    case Qt::Key_F:
        fitToWindow();
        update();
        break;
    default:
        QWidget::keyPressEvent(event);  // 기본 처리
    }
}

void RenderView::closeEvent(QCloseEvent* event)
{
    if (future_.isRunning()) {
        stop_requested_ = true; // 종료 요청 플래그 설정

        watcher_->waitForFinished(); // 쓰레드 완료 대기
    }

    event->accept(); // 실행 중인 작업이 없으면 바로 종료
}

void RenderView::prepairPickingBuffer()
{
    float hidpi = devicePixelRatioF();
    int w = static_cast<int>(width() * hidpi);
    int h = static_cast<int>(height() * hidpi);

    // 새로운 FBO, 텍스처 및 렌더버퍼 생성 및 설정
    glGenFramebuffers(1, &picking_FBO_);
    glBindFramebuffer(GL_FRAMEBUFFER, picking_FBO_);

    glGenTextures(1, &picking_texture_);
    glBindTexture(GL_TEXTURE_2D, picking_texture_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, picking_texture_, 0);

    glGenRenderbuffers(1, &depth_render_buffer_);
    glBindRenderbuffer(GL_RENDERBUFFER, depth_render_buffer_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_render_buffer_);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        spdlog::error("Failed to resize picking framebuffer!");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // FBO 바인딩 해제
}

void RenderView::generateAxesObejct(Node* cs_root)
{
    for (auto it = cs_root->firstChildNode(); it != cs_root->lastChildNode(); ++it)
    {
        Node* nn = *it;
        CSNode* cs_node = dynamic_cast<CSNode*>(nn);
        if (!cs_node) continue;

        AxisObject* obj = new AxisObject(cs_node, shader_);
        axes_[cs_node->getID()] = obj;

        // 첫번째 CS가 default_cs로 지정됨(첫번째를 그렇게 만들었으니깐)
        if (current_cs_id_ == 0) {
            default_cs_id_ = cs_node->getID();
            current_cs_id_ = default_cs_id_;
            coord_ = axes_[current_cs_id_]->getMatrix();
        }

        // 자식 노드들을 재귀적으로 순회
        generateAxesObejct(cs_node);
    }
}

void RenderView::generateGeomObejct(Node* root)
{
    for (auto it = root->firstChildNode(); it != root->lastChildNode(); ++it)
    {
        Node* nn = *it;
        GeomHeadNode* head_node = dynamic_cast<GeomHeadNode*>(nn);
        if (!head_node || head_node->isHide() || !head_node->isStandAlone()) continue;

        GeomObject* obj = new GeomObject(head_node, shader_);
        objects_[obj->getID()] = obj;
    }
}

void RenderView::clearGeomObjects()
{
    // drawing object 삭제
    for (auto it = objects_.begin(); it != objects_.end(); ++it) {
        delete it->second;
    }
    objects_.clear();

    updateModelSize();
}

void RenderView::clearAxesObject()
{
    // axis object 삭제
    for (auto it = axes_.begin(); it != axes_.end(); ++it) {
        delete it->second;
    }
    axes_.clear();
    current_cs_id_ = 0;
    default_cs_id_ = 0;
    object_cs_id_ = 0;
}

std::vector<bzmag::engine::GeomHeadNode*> RenderView::getSelectedHeads() const
{
    std::vector<bzmag::engine::GeomHeadNode*> selected;
    for (auto obj : objects_) {
        if (obj.second->isSelected()) {
            selected.push_back(obj.second->getHead());
        }
    }
    return selected;
}
