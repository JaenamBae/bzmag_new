#pragma once

#include <QObject>
#include <QString>
#include <string>
#include <iostream>
#include <istream>
#include <boost/process.hpp>
#include <boost/process/v1/windows.hpp>

class GmshHelper : public QObject
{
    Q_OBJECT

public:
    explicit GmshHelper(const QString& geo_file, const QString& msh_file);
    virtual ~GmshHelper();

    void setup(const QString& geo_file, const QString& msh_file);
    void executeWithThreadedOutput(const std::string& command);

public slots:
    void run();
    void stop();

signals:
    void progressUpdated(int progress);        // 진행률 업데이트
    void messageUpdated(const QString& task);  // 작업 메시지 업데이트
    void taskStarted(const QString& task);     // 작업 시작 알림
    void taskCompleted(bool success);          // 작업 완료 알림

private:
    void readStream(std::istream& stream, const std::string& label);

    QString geo_file_;
    QString msh_file_;

private:
    std::atomic<bool> stop_requested_ = false; // 중단 요청 플래그
    boost::process::child process_;            // Boost.Process child 객체
};
