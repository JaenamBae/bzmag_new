#pragma once

#include <QObject>
#include <QString>
#include <string>
#include <iostream>
#include <istream>
#include <sstream>
#include <boost/process.hpp>
#include <boost/process/v1/windows.hpp>
#include "Modeler.h"
#include "engine/GeometricEntity.h"

class GetDPHelper : public QObject
{
    Q_OBJECT

public:
    explicit GetDPHelper(const QString& pro_file);

    void setup(const QString& pro_file);
    void executeWithThreadedOutput(const std::string& command);

public slots:
    void run();
    void stop();

signals:
    void progressUpdated(int progress);        // 진행률 업데이트
    void messageUpdated(const QString& task);  // 작업 메시지 업데이트
    void taskStarted(const QString& task);     // 작업 시작 알림
    void taskCompleted(bool success);          // 작업 완료 알림
    void postLocalField(const QString& post_file); // 후처리 파일 통지
    void postGlobalQuantity(const QString& post_file); // 후처리 파일 통지

private:
    bool saveToFile(const std::string& filename) const;
    void readStream(std::istream& stream, const std::string& label);
    void processLine(const std::string& line, const std::string& label);

    void generateConstant();
    void generateFunction();
    void generateGroup();
    void generateJacobian();
    void generateIntegration();
    void generateConstraint();
    void generateFunctionSpace();
    void generateFormulation();
    void generateResolution();
    void generatePostProcessing();
    void generatePostOperation();

    //bool isStrandedCoil(bzmag::engine::GeomHeadNode* head);
    template <typename T>
    static std::string vectorToString(const std::vector<T>& vec) {
        std::ostringstream oss;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) oss << ","; // 쉼표 추가
            oss << vec[i];
        }
        return oss.str();
    }

    std::vector<int> findMovingObject(const bzmag::engine::Polygon_set_2& polyset);
    std::string convertFunctionCall(const std::string& term);


private:
    QString pro_file_;
    Modeler* modeler_ = nullptr;
    int type_ = 1;  // 0: Static, 1: Transient, 2: Complex

    std::string constant_;
    std::string function_;
    std::string group_;
    std::string jacobian_;
    std::string integration_;
    std::string constraint_;
    std::string function_space_;
    std::string formulation_;
    std::string resolution_;
    std::string post_processing_;
    std::string post_operation_;

    std::map<std::string, std::string> function_map_;
    bzmag::engine::MovingBandNode* mb_;

    int last_progress_ = 0;
    bool is_processing_ = false;
    bool is_postprocessing_field_ = false;
    bool is_postprocessing_torque_ = false;
    bool is_postprocessing_global_ = false;

private:
    std::atomic<bool> stop_requested_ = false; // 중단 요청 플래그
    boost::process::child process_;            // Boost.Process child 객체
};