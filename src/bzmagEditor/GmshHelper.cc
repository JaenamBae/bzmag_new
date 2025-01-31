#include "GmshHelper.h"
#include <spdlog/spdlog.h>
#include <thread>
#include <algorithm>
#include "Modeler.h"
#include "engine/GeomToTriangle.h"

using namespace bzmag;
using namespace bzmag::engine;
namespace bp = boost::process;

GmshHelper::GmshHelper(const QString& geo_file, const QString& msh_file) :
    geo_file_(geo_file), msh_file_(msh_file)
{

}

GmshHelper::~GmshHelper()
{

}

void GmshHelper::setup(const QString& geo_file, const QString& msh_file)
{
    geo_file_ = geo_file;
    msh_file_ = msh_file;
}

void GmshHelper::run()
{
    stop_requested_ = false;
    String poly_path = "/" + Modeler::getGeometryName();
    String bc_path = "/" + Modeler::getBCName();

    try {
        GeomToTriangle* triangle = GeomToTriangle::instance();

        emit taskStarted("Model check / Generating gmsh input structure...");

        // Step 1: Gmsh input structure 생성
        bool success = triangle->generateGmshStructures(poly_path, bc_path, [this](int level, int progress) {
            QString task;
            switch (level) {
            case 1:
                task = "Generating gmsh structure\n(1/7) check domain and generate based domains";
                break;

            case 2:
                task = "Generating gmsh structure\n(2/7) generate extra based points";
                break;

            case 3:
                task = "Generating gmsh structure\n(3/7) refine the unique domains";
                break;

            case 4:
                task = "Generating gmsh structure\n(4/7) calculate area of the unique domains";
                break;

            case 5:
                task = "Generating gmsh structure\n(5/7) segment boundaries of the hole domains";
                break;

            case 6:
                task = "Generating gmsh structure\n(6/7) genetate geometry data for the gmesh";
                break;

            case 7:
                task = "Generating gmsh structure\n(7/7) genetate geometry data for the gmsh";
                break;

            default:
                task = "Generating gmsh structure\nunknown task";
                break;
            }
            emit messageUpdated(task);
            emit progressUpdated(progress);
        });

        
        if (!success) {
            if (stop_requested_) {
                emit taskCompleted(false);
                emit messageUpdated("Mesh generation is aborted by user!");
                return;
            }
            else {
                spdlog::error("Fail to generate mesh input structure!");
                emit taskCompleted(false);
                return;
            }
        }

        // Step 2: .geo 파일 생성
        bzmag::String geo_temp(geo_file_.toStdWString().c_str());
        bzmag::String msh_temp(msh_file_.toStdWString().c_str());
        if (!triangle->writeGeoFile(geo_temp)) {
            spdlog::error("Fail to write Gmsh .geo file");
            emit taskCompleted(false);
            return;
        }

        // Step 3: .msh 파일 생성
        bzmag::String command = "gmsh " + geo_temp + " -2 -format msh4 -o " + msh_temp;
        //spdlog::info("Executing command: {}", command.c_str());
        executeWithThreadedOutput(command.c_str());

        emit progressUpdated(100);
        emit taskCompleted(true);
    }
    catch (const std::exception& e) {
        spdlog::error("Exception: {}", e.what());
        emit taskCompleted(false);
    }
    catch (...) {
        spdlog::error("Unknown error occurred.");
        emit taskCompleted(false);
    }
}

void GmshHelper::readStream(std::istream& stream, const std::string& label) {
    std::string line;
    bool is_meshing = false;

    while (std::getline(stream, line)) {
        // 불필요한 '\r' 문자 제거
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        // "Info    : "와 같은 접두사 제거
        auto pos = line.find(":");
        if (pos != std::string::npos && pos + 2 < line.size()) {
            line = line.substr(pos + 2); // ":" 이후의 내용만 남김
        }

        // 로그 메시지 전달
        if (label == "stdout") {
            spdlog::info("{}", line);
        }
        else if (label == "stderr") {
            spdlog::error("{}", line);
        }

        // "Meshing 1D" / "Meshing 2D" 메시지가 나오면 진행 시작
        if (line.find("Meshing 1D") != std::string::npos) {
            is_meshing = true;
            emit messageUpdated("Meshing 1D...");
            continue;
        }
        if (line.find("Meshing 2D") != std::string::npos) {
            is_meshing = true;
            emit messageUpdated("Meshing 2D...");
            continue;
        }

        // "Done meshing 1D" / "Done meshing 2D" 메시지가 나오면 완료 처리
        if (line.find("Done meshing 1D") != std::string::npos) {
            emit progressUpdated(100); // 100%로 설정
            is_meshing = false;
            continue;
        }
        if (line.find("Done meshing 2D") != std::string::npos) {
            emit progressUpdated(100); // 100%로 설정
            is_meshing = false;
            continue;
        }

        // 진행률 업데이트[~%]를 찾음
        if (is_meshing) {
            auto progress_start = line.find("[");
            auto progress_end = line.find("%]");
            if (progress_start != std::string::npos && progress_end != std::string::npos) {
                try {
                    std::string progress_str = line.substr(progress_start + 1, progress_end - progress_start - 1);
                    int progress = std::stoi(progress_str);
                    emit progressUpdated(progress); // 진행률 업데이트
                }
                catch (...) {
                    spdlog::warn("Failed to parse progress from line: {}", line);
                }
            }
        }
    }
}

void GmshHelper::executeWithThreadedOutput(const std::string& command)
{
    namespace bp = boost::process;

    stop_requested_ = false; // 중단 요청 초기화
    bp::ipstream stdout_stream, stderr_stream;

    // Boost.Process child 생성
    process_ = bp::child(command, bp::std_out > stdout_stream, bp::std_err > stderr_stream, bp::windows::hide);

    std::thread stdout_thread([this, &stdout_stream]() {
        readStream(stdout_stream, "stdout");
    });

    std::thread stderr_thread([this, &stderr_stream]() {
        readStream(stderr_stream, "stderr");
    });

    // 중단 요청 감지
    while (process_.running() && !stop_requested_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (stop_requested_) {
        process_.terminate(); // 외부 프로세스 강제 종료
        spdlog::warn("Process terminated by user.");
        emit taskCompleted(false);
        emit messageUpdated("Mesh generation is abored by user!");
    }

    stdout_thread.join();
    stderr_thread.join();

    process_.wait(); // 종료 대기
    spdlog::info("Process exited with code: {}", process_.exit_code());
    emit progressUpdated(100);
}

void GmshHelper::stop()
{
    GeomToTriangle* triangle = GeomToTriangle::instance();
    triangle->stop();

    stop_requested_ = true; // 중단 요청
}
