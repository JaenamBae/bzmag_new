#pragma once

#include <QMatrix4x4>
#include <QRect>
#include <QPoint>
#include <QVector3D>
#include <QQuaternion>

class Camera {
public:
    Camera() : base_scale_(1.0f), zoom_factor_(1.0f), max_zoom_(20.0f), min_zoom_(0.2f), flag_dirty_(true)
    {
        offset_matrix_.setToIdentity();
        zoom_matrix_.setToIdentity();
        trans_matrix_.setToIdentity();
        rotate_matrix_.setToIdentity();
        projection_matrix_.setToIdentity();
    }

    void fitWindow(const QMatrix4x4& coord = QMatrix4x4())
    {
        // 상위 3x3 부분(회전 성분)을 복사
        adjust_matrix_.setToIdentity();
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                adjust_matrix_(i, j) = coord(i, j);
            }
        }
        
        offset_matrix_.setToIdentity();
        offset_matrix_.translate(-offset_.x(), -offset_.y());

        // 줌 초기화; 
        zoom_factor_ = 1.0f;
        zoom(1.0f);

        // 회전, 이동 초기화
        trans_matrix_.setToIdentity();
        rotate_matrix_.setToIdentity();

        // Base Scale초기화
        updateBaseScale();
    }

    // 줌 설정
    void zoom(float zoom_factor)
    {
        zoom_factor_ *= zoom_factor;

        if (zoom_factor_ > max_zoom_) zoom_factor_ = max_zoom_;
        if (zoom_factor_ < min_zoom_) zoom_factor_ = min_zoom_;

        float zoom = base_scale_ * zoom_factor_;
        zoom_matrix_.setToIdentity();
        zoom_matrix_.scale(zoom, zoom, zoom);

        flag_dirty_ = true;
    }

    // 팬 이동 설정
    void pan(const QPointF& delta)
    {
        float zoom_scale = getZoomScale();
        float pan_x =  1.0f * delta.x() / zoom_scale;
        float pan_y = -1.0f * delta.y() / zoom_scale;

        // 기존 view_matrix_에 팬 이동을 누적하여 적용
        QMatrix4x4 translation_matrix;
        translation_matrix.translate(QVector3D(pan_x, pan_y, 0.0f));
        trans_matrix_ *= translation_matrix;
        flag_dirty_ = true;
    }

    // 회전 설정
    void rotate(const QPointF& delta)
    {
        QMatrix4x4 trans = transformation(false);
        QVector3D ref_right = trans.column(0).toVector3D();
        QVector3D ref_up = trans.column(1).toVector3D();

        // X축 회전: 로컬 right 벡터 기준
        QMatrix4x4 rotate_x;
        rotate_x.rotate(delta.y(), ref_right.normalized());

        // Y축 회전: 로컬 Y축 기준
        QMatrix4x4 rotate_y;
        rotate_y.rotate(delta.x(), ref_up.normalized());

        // 연속적으로 회전 적용
        rotate_matrix_ = rotate_y * rotate_x * rotate_matrix_;

        flag_dirty_ = true;
    }

    // 모델과 윈도우 크기를 설정하는 reshape 함수
    void reshape(const QRect& window_rect)
    {
        window_rect_ = window_rect;
        if (window_rect_.width() == 0 || window_rect_.height() == 0) {
            window_rect_ = QRect(0, 0, 1, 1);
        }

        // Projection Matrix 업데이트
        float dist_plane = std::max(window_rect_.width(), window_rect_.height()) * max_zoom_;
        projection_matrix_.setToIdentity();
        projection_matrix_.ortho(-0.5f * window_rect_.width(), 0.5f * window_rect_.width(),
            -0.5f * window_rect_.height(), 0.5f * window_rect_.height(),
            -dist_plane, dist_plane);

        updateBaseScale();
        flag_dirty_ = true;
    }

    void setModelSize(const QVector3D& min, const QVector3D& max)
    {
        if (min == max) {
            model_min_ = QVector3D( -1.0f, -1.0f, -1.0f );
            model_max_ = QVector3D(  1.0f,  1.0f,  1.0f );
        }
        else {
            model_min_ = min;
            model_max_ = max;
        }

        // ModelFit Matrix 업데이트
        offset_ = 0.5 * (model_max_ + model_min_);
        /*offset_matrix_.setToIdentity();
        offset_matrix_.translate(-offset_.x(), -offset_.y());*/

        //updateBaseScale();
        flag_dirty_ = true;
    }

    void updateBaseScale()
    {
        QVector3D model_size = model_max_ - model_min_;
        float scale_x = window_rect_.width() / model_size.x();
        float scale_y = window_rect_.height() / model_size.y();

        // 화면에 딱 맞추지 않고 95%만 맞춤
        base_scale_ = std::min(scale_x, scale_y) * 0.95f;
        zoom(1.0f);
    }

    float getBaseScale() const { return base_scale_; }
    float getZoomFactor() const { return zoom_factor_; }
    float getZoomScale() const { return base_scale_ * zoom_factor_; }

    // 투영 행렬을 반환하는 함수
    QMatrix4x4 projection()
    {
        return projection_matrix_;
    }

    // 최종 변환 행렬을 반환하는 함수
    QMatrix4x4 transformation(bool flag_adjusted = true)
    {
        if (flag_dirty_) {
            final_matrix1_ = zoom_matrix_ * trans_matrix_ * rotate_matrix_ * offset_matrix_;
            final_matrix2_ = final_matrix1_ * adjust_matrix_.inverted();
            
            flag_dirty_ = false;
        }
        if (!flag_adjusted)
            return final_matrix1_;
        else
            return final_matrix2_;
    }

    // 투영 포함 최종 변환 행렬을 반환하는 함수
    QMatrix4x4 projection_transformation(bool flag_adjusted = true)
    {
        return projection() * transformation(flag_adjusted);
    }

private:
    QVector3D model_min_;       // 모델 크기(최소)
    QVector3D model_max_;       // 모델 크기(최대)
    QVector3D offset_;          // 모델을 화면 중앙에 보이기 위한 오프셋
    QRect window_rect_;         // 화면 영역

    float base_scale_;          // 기본 스케일 (모델을 화면에 맞추기 위한 초기 값)
    float zoom_factor_;         // 줌 배율
    float max_zoom_;            // 최대 줌 배율
    float min_zoom_;            // 최소 줌 배율

    QMatrix4x4 adjust_matrix_;      // 좌표계 회전을 고려해 모델을 회전하는 행렬
    QMatrix4x4 offset_matrix_;      // 모델을 화면 중앙에 배치하는 행렬
    QMatrix4x4 zoom_matrix_;        // 줌 변환 행렬
    QMatrix4x4 trans_matrix_;       // 이동 변환 행렬
    QMatrix4x4 rotate_matrix_;      // 회전 변환 행렬
    QMatrix4x4 projection_matrix_;  // 투영 변환 행렬
    QMatrix4x4 final_matrix1_;      // 최종 변환(좌표계 무시) 행렬
    QMatrix4x4 final_matrix2_;      // 최종 변환(좌표계 고려) 행렬

    bool flag_dirty_;
};
