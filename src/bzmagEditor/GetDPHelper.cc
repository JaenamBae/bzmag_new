#include "GetDPHelper.h"
#include <thread>
#include <algorithm>
#include <vector>
#include <regex>
#include <QtConcurrent>
#include <spdlog/spdlog.h>
#include "engine/ExpressionServer.h"


using namespace bzmag;
using namespace bzmag::engine;
namespace bp = boost::process;

GetDPHelper::GetDPHelper(const QString& pro_file) : pro_file_(pro_file)
{
    modeler_ = Modeler::instance();

    function_map_ = {
        {"exp", "Exp"},
        {"ln", "Log"},
        {"log10", "Log10"},
        {"sqrt", "Sqrt"},
        {"sin", "Sin"},
        {"asin", "Asin"},
        {"cos", "Cos"},
        {"acos", "Acos"},
        {"tan", "Tan"},
        {"atan", "Atan"},
        {"atan2", "Atan2"},
        {"sinh", "Sinh"},
        {"cosh", "Cosh"},
        {"tanh", "Tanh"},
        {"abs", "Abs"},
        {"min", "Min"},
        {"max", "Max"},
        {"sign", "Sign"}
    };
}

void GetDPHelper::setup(const QString& pro_file)
{
    pro_file_ = pro_file;
}

void GetDPHelper::run()
{
    if (pro_file_.isEmpty()) return;

    // 솔버 타입 설정; 0: Static, 1: Transient
    SolutionSetup* setup = modeler_->getWorkingDefaultSetupNode();
    if (!setup->getTransientNode()) {
        type_ = 0;
    }
    else {
        type_ = 1;
    }


    emit taskStarted("Generating .pro ifle");

    generateGroup();
    emit progressUpdated(10);

    generateConstant();
    emit progressUpdated(20);

    generateFunction();
    emit progressUpdated(30);

    generateJacobian();
    emit progressUpdated(40);

    generateIntegration();
    emit progressUpdated(50);

    generateConstraint();
    emit progressUpdated(60);

    generateFunctionSpace();
    emit progressUpdated(70);

    generateFormulation();
    emit progressUpdated(80);

    generateResolution();
    emit progressUpdated(90);

    generatePostProcessing();
    generatePostOperation();
    emit progressUpdated(100);

    emit messageUpdated("Saving .pro ifle");

    bzmag::String save_temp(pro_file_.toStdWString().c_str());
    if (!saveToFile(save_temp.c_str())) {
        spdlog::error("Fail to save file: ", save_temp.c_str());
        emit taskCompleted(false);
        return;
    }

    try {
        emit messageUpdated("Starting analysis...");
        bzmag::String command = "getdp " + save_temp + " -solve Analysis v 3 -pos Get_LocalFields";
        //spdlog::info("Executing command: {}", command.c_str());
        executeWithThreadedOutput(command.c_str());

        emit progressUpdated(100);
        emit taskCompleted(true);
    }
    catch (const std::exception& e) {
        spdlog::error("Exception while writing .pro file: {}", e.what());
        emit taskCompleted(false);
        return;
    }
    catch (...) {
        spdlog::error("Unknown error occurred while writing .pro file.");
        emit taskCompleted(false);
        return;
    }
}

void GetDPHelper::readStream(std::istream& stream, const std::string& label)
{
    std::string buffer; // 데이터 버퍼
    char ch;
    while (stream.get(ch)) { // 문자 단위로 읽기
        if (ch == '\n' || ch == '\r') { // 캐리지 리턴 단위 혹은 줄단위로 처리
            // 현재 버퍼에 저장된 데이터를 처리
            if (!buffer.empty()) {
                processLine(buffer, label);
                buffer.clear(); // 버퍼 초기화
            }
        }
        else {
            buffer += ch; // 버퍼에 문자 추가
        }
    }

    // 스트림 끝에 도달한 후 남은 데이터 처리
    if (!buffer.empty()) {
        processLine(buffer, label);
    }
}

void GetDPHelper::processLine(const std::string& line, const std::string& label)
{
    // 진행률 메시지 처리
    if (is_processing_) {
        std::regex pattern(R"(bzMagProcessing\s+\[(\d+),\s*(\d+)\])");
        std::smatch match;

        // 문자열에서 패턴 찾기
        if (std::regex_search(line, match, pattern)) {
            // 추출된 숫자
            int current_step = std::stoi(match[1].str());
            int total_step = std::stoi(match[2].str());
            int progress = int((float)current_step / (float)total_step * 100.0f);

            emit messageUpdated(
                QString("Solving the problem with getdp\n(%1/%2)")
                .arg(current_step)
                .arg(total_step));
            emit progressUpdated(progress);
        }
    }

    // 후처리 메시지 처리
    if (is_postprocessing_field_) {
        // 정규 표현식 패턴 정의
        std::regex pattern(R"(>\s*'([^']+)')");

        // 정규 표현식 검색
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            QString post_file = QString::fromStdString(match[1]);
            emit postLocalField(post_file);
        }
        is_postprocessing_field_ = false;
    }

    if (is_postprocessing_torque_) {
        // 정규 표현식 패턴 정의
        std::regex pattern(R"(>\s*'([^']+)')");

        // 정규 표현식 검색
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            QString post_file = QString::fromStdString(match[1]);
            emit postGlobalQuantity(post_file);
        }
        is_postprocessing_torque_ = false;
    }

    if (is_postprocessing_global_) {
        // 정규 표현식 패턴 정의
        std::regex pattern(R"(>\s*'([^']+)')");

        // 정규 표현식 검색
        std::smatch match;
        if (std::regex_search(line, match, pattern)) {
            QString post_file = QString::fromStdString(match[1]);
            emit postGlobalQuantity(post_file);
        }
        is_postprocessing_global_ = false;
    }


    // 현재 메시지 파싱
    std::string processed_line = line;

    // 로그 메시지 전달
    if (label == "stdout") {
        spdlog::info("{}", line);
    }
    else if (label == "stderr") {
        spdlog::error("{}", line);
    }

    // 작업 시작 메시지 처리
    if (processed_line.find("P r e - P r o c e s s i n g . . .") != std::string::npos) {
        is_processing_ = true;
        emit taskStarted("Pre-processing");
    }
    else if (processed_line.find("P r o c e s s i n g . . .") != std::string::npos) {
        is_processing_ = true;
        emit taskStarted("Solving Problem with getdp");
    }
    else if (processed_line.find("PostOperation 'Get_LocalFields'") != std::string::npos) {
        is_postprocessing_field_ = true;
    }
    else if (processed_line.find("PostOperation 'Get_GlobalQuantities'") != std::string::npos) {
        is_postprocessing_global_ = true;
    }
    else if (processed_line.find("PostOperation 'Get_Torque'") != std::string::npos) {
        is_postprocessing_torque_ = true;
    }

    // 작업 완료 메시지 처리
    if (processed_line.find("E n d   P r e - P r o c e s s i n g") != std::string::npos) {
        emit progressUpdated(100);
        is_processing_ = false;
    }
    else if (processed_line.find("E n d   P r o c e s s i n g") != std::string::npos) {
        emit progressUpdated(100);
        is_processing_ = false;
    }
}

void GetDPHelper::executeWithThreadedOutput(const std::string& command)
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
        emit messageUpdated("Analysis is abored by user!");
    }

    stdout_thread.join();
    stderr_thread.join();

    process_.wait(); // 종료 대기
    spdlog::info("Process exited with code: {}", process_.exit_code());
    emit progressUpdated(100);
}

void GetDPHelper::stop()
{
    if (process_.running()) {
        stop_requested_ = true; // 중단 요청
    }
}

bool GetDPHelper::saveToFile(const std::string& filename) const
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    file
        << "// Generated by bzMagEditor\n\n"
        << constant_.c_str() << std::endl
        << function_.c_str() << std::endl
        << group_.c_str() << std::endl
        << jacobian_.c_str() << std::endl
        << integration_.c_str() << std::endl
        << constraint_.c_str() << std::endl
        << function_space_.c_str() << std::endl
        << formulation_.c_str() << std::endl
        << resolution_.c_str() << std::endl
        << post_processing_.c_str() << std::endl
        << post_operation_.c_str() << std::endl;

    return true;
}

void GetDPHelper::generateConstant()
{
    SolutionSetup* setup = modeler_->getWorkingDefaultSetupNode();

    // constant_str 구성
    std::ostringstream stream;
    stream 
        << "DefineConstant[\n"
        << "  AxialLength = " << setup->getLengthZ() << ",\n"
        << "  SymmetryFactor = " << setup->getSymmetrcyFactor() << ",\n";

    stream
        << "  NL_tol_abs = " << setup->getAbsoluteTolerance() << ",\n"
        << "  NL_tol_rel = " << setup->getRelativeTolerance() << ",\n"
        << "  NL_iter_max = " << setup->getMaxIteration() << ",\n";

    if (type_ == 1) {
        Transient* transient = setup->getTransientNode();

        stream << std::scientific << std::setprecision(16);
        if (transient) {
            stream
                << "  time0 = 0,\n"
                << "  timemax = " << transient->evalStopTime() << ",\n"
                << "  delta_time = " << transient->evalTimeStep() << ",\n";
        }
        else {
            stream
                << "  time0 = 0,\n"
                << "  timemax = 1,\n"
                << "  delta_time = 1,\n";
                
        }
        stream 
            << "  NTimeSteps = (timemax - time0) / delta_time + 1,\n";

        if (mb_) {
            // 회전기의 경우
            if (mb_->isCircular()) {
                stream
                    << "  RotatingSpeed = " << mb_->evalSpeed() << ",\n"
                    << "  delta_theta = 2*Pi*RotatingSpeed/60*delta_time,\n";
            }

            // 선형기의 경우
            else {

            }
        }
    }

    stream
        << "  ResId = \"\",\n"
        << "  ResDir = StrCat[\"res/\", ResId],\n"
        << "  ExtGmsh = \".pos\",\n"
        << "  ExtGnuplot = \".dat\",\n"
        << "  Flag_SaveAllSteps = 1,\n"
        << "  R_ = {\"Analysis\", Name \"GetDP / 1ResolutionChoices\", Visible 0},\n"
        << "  C_ = {\" -solve -v 3 -v2\", Name \"GetDP/9ComputeCommand\", Visible 0},\n"
        << "  P_ = {\"\", Name \"GetDP/2PostOperationChoices\", Visible 0}\n"
        << "];\n";

    constant_ = stream.str(); // 최종 문자열 생성

    // constant_ 출력
    //std::cout << constant_ << std::endl;
}

void GetDPHelper::generateFunction()
{
    // constant_str 구성
    std::ostringstream stream;
    stream
        << "Function {\n"
        << "  DefineFunction[nu, dhdb, sigma, Br, SurfCoil, NbWires, Idir];\n"
        << "}\n\n";

    //-----------------------------------------------------
    // 일반 함수
    stream
        << "Function {\n"
        << "  mu0 = 4.e-7 * Pi;\n"
        << "  T_max[] = (SquDyadicProduct[$1] - SquNorm[$1] * TensorDiag[0.5, 0.5, 0.5]) / mu0;\n"
        << "  Rotate2D[] = Vector[$1*Cos[$3] - $2*Sin[$3], $1*Sin[$3] + $2*Cos[$3], 0];\n"
        << "  Rotate2D_X[] = $1*Cos[$3] - $2*Sin[$3];\n"
        << "  Rotate2D_Y[] = $1*Sin[$3] + $2*Cos[$3];\n"
        << "  AngularPosition[] = (Atan2[$Y,$X]#1 >= 0.)? #1 : #1+2*Pi;\n"
        << "  RotatePZ[] = Rotate[Vector[$X,$Y,$Z], 0, 0, $1];\n";

    if(mb_) {
        std::string mb_name(mb_->getName().c_str());
        std::string torque_name1 = "Torque_" + mb_name + "_Outer";
        std::string torque_name2 = "Torque_" + mb_name + "_Inner";
        stream
            << "  " << torque_name1 << "[] = $" << torque_name1 << ";\n"
            << "  " << torque_name2 << "[] = $" << torque_name2 << ";\n";
    }

    stream
        << "  RotorPosition[] = $RotorPosition;\n"
        << "}\n\n";

    //-----------------------------------------------------
    // 재질조건
    Node* material_root = modeler_->getWorkingMaterialRootNode();
    stream
        << "// BH curves\n"
        << "Function {\n";
    for (auto it = material_root->firstChildNode(); it != material_root->lastChildNode(); ++it) {
        MaterialNode* material = *it;
        if (!material->isLinear()) {
            DataSetNode* dataset = material->getDataSetNode();
            std::vector<float64> x_components = dataset->extractXComponents();
            std::vector<float64> y_components = dataset->extractYComponents();

            std::string name_material(material->getName());
            stream 
                << "  mat_" + name_material + "_h" << " = {" << vectorToString(x_components) << "};\n"
                << "  mat_" + name_material + "_b" << " = {" << vectorToString(y_components) << "};\n"
                << "  mat_" + name_material + "_b2" << " = " << "mat_" + name_material + "_b" << "()^2;\n"
                << "  mat_" + name_material + "_nu" << " = " << "mat_" + name_material + "_h" << "() / " << "mat_" + name_material + "_b" << "();\n"
                << "  mat_" + name_material + "_nu" << "(0) = " << "mat_" + name_material + "_nu" << "(1);\n"
                << "  mat_" + name_material + "_nu_b2" << " = ListAlt[" << "mat_" + name_material + "_b2" << "(), " << "mat_" + name_material + "_nu" << "()];\n"
                << "  nu_" << name_material << "[] = InterpolationLinear[SquNorm[$1]]{ " << "mat_" + name_material + "_nu_b2" << "() };\n"
                << "  dnudb2_" << name_material << "[] = dInterpolationLinear[SquNorm[$1]]{ " << "mat_" + name_material + "_nu_b2" << "() };\n"
                << "  h_" << name_material << "[] = " << "nu_" << name_material << "[$1] * $1; \n"
                << "  dhdb_" << name_material << "[] = TensorDiag[1, 1, 1] * nu_" << name_material << "[$1] + 2 * dnudb2_" << name_material << "[$1] * SquDyadicProduct[$1];\n";
        }
    }
    stream
        << "}\n\n";

    stream
        << "Function {\n";

    // 투자율/증분투자율(면에 대해서)
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = *it;
        if (head && head->isCovered() && head->isModelNode()) {
            uint32 id = head->getID() * 100;
            MaterialNode* material = head->getMaterialNode();
            if (material) {
                float64 permeability = material->evalPermeability();
                if (material->isLinear()) {
                    stream << "  nu[Region[" << id << "]] = 1/(mu0*" << permeability << ");\n";
                }
                else {
                    std::string name_material(material->getName());
                    stream << "  nu[Region[" << id << "]] = nu_" << name_material << "[$1];\n";
                    stream << "  dhdb[Region[" << id << "]] = dhdb_" << name_material << "[$1];\n";
                }
            }
            else {
                stream << "  nu[Region[" << id << "]] = 1/mu0;\n";
            }
        }
    }
    // 무빙밴드 재질;
    // generateRegion() 함수에서 mb_map_ 데이터를 생성함; 고로 generateRegion()함수가 먼저 호출되었어야 함
    if (mb_) {
        int outer_id = mb_->getOuterAirgapID() * 100;
        int inner_id = mb_->getInnerAirgapID() * 100;
        int mb_id = mb_->getID() * 100;
        stream
            << "  nu[Region[" << outer_id << "]] = 1/mu0;\n"
            << "  nu[Region[" << inner_id << "]] = 1/mu0;\n"
            << "  nu[Region[" << mb_id << "]] = 1/mu0;\n";
    }

    // 도전율(면에 대해서)
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = *it;
        if (head && head->isCovered() && head->isModelNode()) {
            uint32 id = head->getID() * 100;
            MaterialNode* material = head->getMaterialNode();
            if (material) {
                float64 conductivity = material->evalConductivity();
                stream << "  sigma[Region[" << id << "]] = " << conductivity << ";\n";
            }
            else {
                stream << "  sigma[Region[" << id << "]] = " << 0 << ";\n";
            }
        }
    }
    // 무빙밴드 재질; 
    if (mb_) {
        int outer_id = mb_->getOuterAirgapID() * 100;
        int inner_id = mb_->getInnerAirgapID() * 100;
        int mb_id = mb_->getID() * 100;
        stream
            << "  sigma[Region[" << outer_id << "]] = 0;\n"
            << "  sigma[Region[" << inner_id << "]] = 0;\n"
            << "  sigma[Region[" << mb_id << "]] = 0;\n";
    }

    // 자화 설정(면에 대해서)
    ExpressionServer* server = ExpressionServer::instance();
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = *it;
        if (head && head->isCovered() && head->isModelNode()) {
            uint32 id = head->getID() * 100;
            MaterialNode* material = head->getMaterialNode();
            if (material && abs(material->evalMagnetization()) > 0) {
                float64 magnetization = material->evalMagnetization();
                Vector2 dir, org;
                CSNode* cs = head->getReferedCS();
                if (cs) {
                    Transformation trans = cs->transformation();
                    Point_2 origin = trans(Point_2(0, 0));
                    org = Vector2(CGAL::to_double(origin.x()), CGAL::to_double(origin.y()));
                }

                String direction = material->getDirectionOfMagnetization();
                std::vector<String> token = Expression::splitByTopLevelComma(direction);
                if (token.size() == 2) {
                    std::string dir_x(token[0].c_str());
                    server->setExpr(dir_x);
                    auto used_vars_x = server->getUsedVar();
                    for (auto var : used_vars_x) {
                        server->setExpr(var.first);
                        auto val = server->eval();

                        // $X, $Y, $Z는 독립변수로 취급한다
                        if (var.first != "$X" && var.first != "$Y" && var.first != "$Z") {
                            std::regex pattern(var.first);
                            dir_x = std::regex_replace(dir_x, pattern, std::to_string(val));
                        }
                    }

                    std::string dir_y(token[1].c_str());
                    server->setExpr(dir_y);
                    auto used_vars_y = server->getUsedVar();
                    for (auto var : used_vars_y) {
                        server->setExpr(var.first);
                        auto val = server->eval();

                        // $X, $Y, $Z는 독립변수로 취급한다
                        if (var.first != "$X" && var.first != "$Y" && var.first != "$Z") {
                            std::regex pattern(var.first);
                            dir_y = std::regex_replace(dir_y, pattern, std::to_string(val));
                        }
                    }

                    // 참조 좌표계의 중심은 회전자가 회전함에 따라 같이 회전해야 함
                    std::string cx, cy;
                    if (org.x() != 0 && org.y() != 0) {
                        cx = "-Rotate2D_X[" + std::to_string(org.x()) + "," + std::to_string(org.y()) + ",0]";
                        cy = "-Rotate2D_Y[" + std::to_string(org.x()) + "," + std::to_string(org.y()) + ",0]";
                    }

                    std::regex pattern_x("\\$X"); // $X를 찾기 위한 정규식
                    std::regex pattern_y("\\$Y"); // $Y를 찾기 위한 정규식

                    std::string replacement_x = "$X" + cx;
                    std::string replacement_y = "$Y" + cy;

                    dir_x = std::regex_replace(dir_x, pattern_x, replacement_x);
                    dir_x = std::regex_replace(dir_x, pattern_y, replacement_y);

                    dir_y = std::regex_replace(dir_y, pattern_x, replacement_x);
                    dir_y = std::regex_replace(dir_y, pattern_y, replacement_y);


                    // getdp 함수 표현 형태로 변환
                    dir_x = convertFunctionCall(dir_x);
                    dir_y = convertFunctionCall(dir_y);

                    stream
                        << "  Br[Region[" << id << "]] = " << magnetization
                        << " * (Vector["
                        << "(" << dir_x;

                    if (org.x() != 0 && org.y() != 0) {
                        stream
                            << "-Rotate2D_X[" << org.x() << "," << org.y() << ",0]";
                    }

                    stream
                        << "),(" << dir_y;

                    if (org.x() != 0 && org.y() != 0) {
                        stream
                            << "-Rotate2D_Y[" << org.x() << "," << org.y() << ",0]";
                    }

                    stream
                        << "),0]";

                    if (org.x() != 0 && org.y() != 0) {
                        stream
                            << "+Rotate2D[" << dir.x() << ", " << dir.y() << ", RotorPosition[]]";
                    }
                    stream << ");\n";
                }
            }
        }
    }

    stream
        << "}\n\n";

    //-----------------------------------------------------
    // 여자조건; 코일면적, 전류방향, 코일의 턴수 설정
    Node* excite_root = modeler_->getWorkingExcitationRootNode();
    stream
        << "Function {\n";
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        for (auto koil = winding->firstChildNode(); koil != winding->lastChildNode(); ++koil) {
            CoilNode* coil = *koil;
            int32 ref_id = coil->getReferenceNode()->getID() * 100;
            stream
                << "  SurfCoil[Region[" << ref_id << "]] = SurfaceArea[]{" << ref_id << "};\n";
        }
    }
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        for (auto koil = winding->firstChildNode(); koil != winding->lastChildNode(); ++koil) {
            CoilNode* coil = *koil;
            int32 ref_id = coil->getReferenceNode()->getID() * 100;
            stream
                << "  NbWires[Region[" << ref_id << "]] = " << coil->evaluateNumberOfTurns() << ";\n";
        }
    }
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        for (auto koil = winding->firstChildNode(); koil != winding->lastChildNode(); ++koil) {
            CoilNode* coil = *koil;
            int32 ref_id = coil->getReferenceNode()->getID() * 100;
            int direction = coil->getDirection() ? 1 : -1;
            stream
                << "  Idir[Region[" << ref_id << "]] = " << direction << ";\n";
        }
    }

    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        if (winding) {
            std::string current(winding->getCurrent().c_str()); 
            
            server->setExpr(current);
            auto used_vars = server->getUsedVar();
            for (auto var : used_vars) {
                server->setExpr(var.first);
                auto val = server->eval();

                if (var.first != "$TIME") {
                    std::regex pattern(var.first);
                    current = std::regex_replace(current, pattern, std::to_string(val));
                }
            }
            current = convertFunctionCall(current);

            String name = winding->getName();
            uint32 id = winding->getID() * 100;
            stream
                << "  I_" << name.c_str() << "[] = " << current << ";\n";
        }
    }

    stream
        << "}\n";

    function_ = stream.str(); // 최종 문자열 생성

    // function_ 출력
    //std::cout << function_ << std::endl;
}

void GetDPHelper::generateGroup()
{
    mb_ = nullptr;
    std::vector<unsigned int> domainC;
    std::vector<unsigned int> domainCC;
    std::vector<unsigned int> domainM;
    std::vector<unsigned int> domainB;
    std::vector<unsigned int> domainL;
    std::vector<unsigned int> domainNL;

    // 이하 면에대한 영역임
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = *it;
        if (head && head->isCovered() && head->isModelNode()) {
            uint32 id = head->getID() * 100;
            MaterialNode* material = head->getMaterialNode();
            if (material) {
                float64 conductiviey = material->getConductivity();

                // 도전율이 0보다 크면 domainC에 포함됨; 단, 추후 코일(인덕터)로 판명시 domainC에서 제외할 것임
                if (conductiviey > 0) {
                    domainC.push_back(id);
                }

                // 그렇지 않으면(도전율이 0이면 domainC에 포함시킴)
                else {
                    domainCC.push_back(id);
                }

                // 재질이 선형이면 domainL 에 포함시킴
                if (material->isLinear()) {
                    domainL.push_back(id);
                }

                // 그렇지 않으면 domainNL에 포함시킴
                else {
                    domainNL.push_back(id);
                }

                // 자화가 0보다 크면 domainM에 포함시킴
                if (abs(material->evalMagnetization()) > 0) {
                    domainM.push_back(id);
                }
            }
        }
    }

    // domainB(인덕터)에 포함될 Head를 찾음
    std::map<std::string, std::vector<int>> winding_map;
    std::map<std::string, std::vector<int>> winding_pos_map;
    Node* excitation_root = modeler_->getWorkingExcitationRootNode();
    for (auto it = excitation_root->firstChildNode(); it != excitation_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        if (winding) {
            for (auto coil = winding->firstChildNode(); coil != winding->lastChildNode(); ++coil) {
                CoilNode* koil = *coil;
                if (koil) {
                    GeomHeadNode* ref_head = koil->getReferenceNode();
                    if (ref_head) {
                        // 코일 노드가 참조하는 Head는 domainB에 포함시키고 대신, domainC 에서 제외시킴(stranded 권선이라 가정함)
                        // domainC에서 제외된 노드는 domainCC로 이동함
                        uint32 ref_id = ref_head->getID() * 100;
                        domainB.push_back(ref_id);
                        domainC.erase(std::remove(domainC.begin(), domainC.end(), ref_id), domainC.end());
                        domainCC.push_back(ref_id);

                        winding_map[winding->getName().c_str()].push_back(ref_id);
                        if (koil->getDirection()) {
                            winding_pos_map[winding->getName().c_str()].push_back(ref_id);
                        }
                    }
                }
            }
        }
    }

    //--------------------------------------
    // 이하 선, 점에 대한 영역임
    std::map<BCNode*, int> bc_map;
    std::map<BCNode*, std::vector<int>> bc_pointmap;
    Node* bc_root = modeler_->getWorkingBCRootNode();
    for (auto it = bc_root->firstChildNode(); it != bc_root->lastChildNode(); ++it) {
        BCNode* bc = *it;
        if (bc) {
            std::string bc_name(bc->getName());

            // 고정경계
            DirichletBCNode* dirichlet = dynamic_cast<DirichletBCNode*>(bc);
            if (dirichlet) {
                bc_map[bc] = dirichlet->getID() * 100;
                bc_pointmap[bc].push_back(dirichlet->getID() * 100 + 10);
            }

            // 주기경계
            SlavePeriodicBCNode* slave = dynamic_cast<SlavePeriodicBCNode*>(bc);
            if (slave) {
                MasterPeriodicBCNode* master = slave->getPair();
                if (master) {
                    bc_map[master] = master->getID() * 100;
                    bc_map[slave] = slave->getID() * 100;
                    bc_pointmap[master].push_back(master->getID() * 100 + 10);
                    bc_pointmap[slave].push_back(slave->getID() * 100 + 10);
                }
            }

            // 무빙 경계
            MovingBandNode* mb = dynamic_cast<MovingBandNode*>(bc);
            if (mb && mb->checkValid()) {
                mb_ = mb;
            }
        }
    }

    // 무빙밴드가 존재하면 이것도 도메인에 포함시켜야 함;
    // 무빙밴드의 피지컬 번호를 MovingBandNode의 id로 설정함
    if (mb_) {
        // 아우터 영역의 region_id 등록
        domainCC.push_back(mb_->getOuterAirgapID() * 100);
        domainL.push_back(mb_->getOuterAirgapID() * 100);

        // 이너 영역의 region_id 등록
        domainCC.push_back(mb_->getInnerAirgapID() * 100);
        domainL.push_back(mb_->getInnerAirgapID() * 100);

        // 무빙 밴드 MovingBand_PhysicalNb의 region_id
        domainCC.push_back(mb_->getID() * 100);
        domainL.push_back(mb_->getID() * 100);
    }

    // 각 도메인의 문자열 생성
    std::string domainC_str = vectorToString(domainC);
    std::string domainCC_str = vectorToString(domainCC);
    std::string domainM_str = vectorToString(domainM);
    std::string domainB_str = vectorToString(domainB);
    std::string domainL_str = vectorToString(domainL);
    std::string domainNL_str = vectorToString(domainNL);

    // group_str 구성
    std::ostringstream stream;
    stream << "Group {\n"
        << "  DomainC = Region[{" << domainC_str << "}];\n"
        << "  DomainCC = Region[{" << domainCC_str << "}];\n"
        << "  DomainM = Region[{" << domainM_str << "}];\n"
        << "  DomainB = Region[{" << domainB_str << "}];\n"
        << "  DomainL = Region[{" << domainL_str << "}];\n"
        << "  DomainNL = Region[{" << domainNL_str << "}];\n"
        << "  Domain = Region[{DomainC, DomainCC}];\n"
        << "  Inds = Region[{" << domainB_str << "}];\n";

    // 무빙밴드 공극 영역 등록
    if (mb_) {
        int32 id = mb_->getID() * 100;
        std::string mb_name(mb_->getName().c_str());
        stream
            << "  " << mb_name << "_PhysicalNb = Region[" << id << "];\n";
    }

    for (auto& [name, ids] : winding_map) {
        stream
            << "  " << name << " = Region[{" << vectorToString(ids) << "}];\n";
    }

    for (auto& [name, ids] : winding_pos_map) {
        stream
            << "  " << name << "_pos = Region[{" << vectorToString(ids) << "}];\n";
    }

    for (auto& [bc, id] : bc_map) {
        stream
            << "  " << bc->getName().c_str() << " = Region[{" << id << "}];\n";
    }
    
    // 무빙밴드 경계라인 추가
    if (mb_) {
        // 회전의 경우
        if (mb_->isCircular()) {
            float64 angle = mb_->getCircularCoefficient();
            int symmetry_factor = (int)(round(2 * CGAL_PI / angle));

            // 공극의 아우터/이너 영역 등록
            int32 outer_gap_id = mb_->getOuterAirgapID() * 100;
            int32 inner_gap_id = mb_->getInnerAirgapID() * 100;
            std::string mb_name(mb_->getName().c_str());
            std::string mb_name1 = "MB_" + mb_name + "_Outer";
            std::string mb_name2 = "MB_" + mb_name + "_Inner";
            stream
                << "  " << mb_name1 << " = Region[{" << outer_gap_id << "}];\n"
                << "  " << mb_name2 << " = Region[{" << inner_gap_id << "}];\n";

            BCNode* outer = mb_->getOuterBCNode();
            BCNode* inner = mb_->getInnerBCNode();
            int32 outer_id = outer->getID() * 100;
            int32 inner_id = inner->getID() * 100;

            std::string outer_name(outer->getName().c_str());
            std::string inner_name(inner->getName().c_str());
            std::string outer_name2 = "MB_" + mb_name + "_" + outer_name;
            std::string inner_name2 = "MB_" + mb_name + "_" + inner_name;

            // 아우터 바운드리; 복제 안함
            stream
                << "  " << outer_name2 << " = Region[{" << outer_id << "}];\n";

            // 이너 바운드리; 이것은 원 둘래만큼 복제해야 함
            std::vector<int> mb_ids;
            for (int i = 0; i < symmetry_factor; ++i) {
                stream
                    << "  " << inner_name2 << "_" << i << " = Region[{" << inner_id + i << "}];\n";
                mb_ids.push_back(inner_id + i);
            }
            
            // 복제된 경계를 모두 합친것을 inner_name 로 하여 영역을 만듬
            stream
                << "  " << inner_name2 << " = Region[{" << vectorToString(mb_ids) << "}];\n"
                << "  " << inner_name2 << "Aux = Region[{" << inner_name2 << ", -" << inner_name2 << "_0}];\n"
                << "  MB_" << mb_name << " = MovingBand2D[" << mb_name << "_PhysicalNb, " << outer_name2 << ", " << inner_name2 << ", " << symmetry_factor << "];\n";

            auto polyset = mb_->getMovingArea();
            auto moving_ids = findMovingObject(polyset);

            stream
                << "  Moving_" << mb_name << " = Region[{" << vectorToString(moving_ids) << "," << inner_name2 << "Aux}];\n"
                << "  Torque_" << mb_name << " = Region[{" << vectorToString(moving_ids) << "}];\n";
        }

        // 선형기의 경우 아직 지원 안함
        else {

        }
    }
    /*
    for (auto& [bc, ids] : bc_pointmap) {
        stream
            << "  " << bc->getName().c_str() << "_Point = Region[{" << vectorToString(ids) << "}];\n";
    }
    */
    stream
        << "}\n";

    group_ = stream.str(); // 최종 문자열 생성

    // group_ 출력
    //std::cout << group_ << std::endl;
}

void GetDPHelper::generateJacobian()
{
    std::ostringstream stream;
    stream
        << "Jacobian{\n"
        << "  { Name Vol; Case { { Region All; Jacobian Vol; } } }\n"
        << "  { Name Sur; Case { { Region All; Jacobian Sur; } } }\n"
        << "}\n";

    jacobian_ = stream.str(); // 최종 문자열 생성

    // jacobian_ 출력
    //std::cout << jacobian_ << std::endl;
}

void GetDPHelper::generateIntegration()
{
    std::ostringstream stream;
    stream
        << "Integration{\n"
        << "  { Name I1; Case {\n"
        << "      { Type Gauss;\n"
        << "        Case {\n"
        << "          { GeoElement Triangle; NumberOfPoints 6; }\n"
        << "          { GeoElement Quadrangle; NumberOfPoints 4; }\n"
        << "          { GeoElement Line; NumberOfPoints 13; }\n"
        << "        }\n"
        << "      }\n"
        << "    }\n"
        << "  }\n"
        << "}\n";

    integration_ = stream.str(); // 최종 문자열 생성

    // integration_ 출력
    //std::cout << integration_ << std::endl;
}

void GetDPHelper::generateConstraint()
{
    std::ostringstream stream;
    stream << std::scientific << std::setprecision(16);
    
    // 제약조건 셋팅
    stream
        << "Constraint {\n";

    stream
        << "  { Name MVP_2D;\n"
        << "    Case {\n";
    // 마그네틱 벡터 포텐셜에 대한 제약(고정경계, 주기경계, 무빙밴드 포함)
    {
        Node* bc_root = modeler_->getWorkingBCRootNode();
        for (auto it = bc_root->firstChildNode(); it != bc_root->lastChildNode(); ++it) {
            BCNode* bc = *it;
            std::string bc_name(bc->getName());

            // 고정경계
            DirichletBCNode* dirichlet = dynamic_cast<DirichletBCNode*>(bc);
            if (dirichlet) {
                std::string bc_name(dirichlet->getName());
                stream
                    << "      { Region " << bc_name << "; Type Assign; Value " << dirichlet->evaluateBCValue() << "; }\n";
            }

            // 주기경계
            SlavePeriodicBCNode* slave = dynamic_cast<SlavePeriodicBCNode*>(bc);
            if (slave && slave->checkValid()) {
                std::string slave_name(slave->getName());
                MasterPeriodicBCNode* master = slave->getPair();
                if (master) {
                    std::string master_name(master->getName());
                    stream
                        << "      { Region " << slave_name << "; "
                        //<<         "SubRegion Region[" << slave_name << "_Point]; "
                        << "Type Link; "
                        << "RegionRef " << master_name << ";\n";
                        //<<         "SubRegionRef Region["<< master_name <<"_Point];\n";

                    if (slave->isEven()) {
                        stream << "        Coefficient 1;\n";
                    }
                    else {
                        stream << "        Coefficient -1;\n";
                    }

                    if (slave->isCircular()) {
                        float64 angle = slave->getCircularCoefficient();
                        int symmetry_factor = (int)(round(2 * CGAL_PI / angle));
                        stream 
                            << "        Function RotatePZ[-2*Pi/" << symmetry_factor << "];\n"
                            << "      }\n";
                    }

                    // 선형기는 아직 지원 안함
                    else {
                        //Vector2 disp = slave->getLinearCoefficient();
                        //stream << "        Function MovePXY[" << disp.x() << "," << disp.y() << "]; }\n";
                    }
                }
            }
        }
    }

    // 주기 모델인 경우 무빙밴드에 대한 주기조건도 설정해야 함
    {
        if (mb_ && mb_->checkValid()) {
            // 회전기의 경우
            if (mb_->isCircular() && mb_->getCircularCoefficient() > 0) {
                float64 angle = mb_->getCircularCoefficient();
                int symmetry_factor = (int)(round(2 * CGAL_PI / angle));

                BCNode* inner = mb_->getInnerBCNode();
                std::string mb_name(mb_->getName().c_str());
                std::string inner_name(inner->getName().c_str());
                std::string inner_name2 = "MB_" + mb_name + "_" + inner_name;
                stream
                    << "      For k In {0:" << symmetry_factor - 2 << "}\n"
                    << "        { Region " << inner_name2 << "~{k+1}; SubRegion " << inner_name2 << "~{(k!=2)?k+2:0}; \n"
                    << "          Type Link;\n"
                    << "          RegionRef " << inner_name2 << "_0; SubRegionRef " << inner_name2 << "_1;\n";

                if (mb_->isEven()) {
                    stream
                        << "          Coefficient 1;\n";
                }
                else {
                    stream
                        << "          Coefficient (-1)^(k+1);\n";
                }
                stream
                    << "          Function RotatePZ[-2*Pi/" << symmetry_factor << "*(k+1)];\n"
                    << "        }\n"
                    << "      EndFor\n";
            }

            // 선형기는 아직 지원 안함
            else {

            }
        }
    }
    stream
        << "    }\n"
        << "  }\n";

    // 여자조건 셋팅
    {
        if (type_ == 0) {
            stream
                << "  { Name Current_2D;\n"
                << "    Case{\n";
            Node* excite_root = modeler_->getWorkingExcitationRootNode();
            for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
                WindingNode* winding = *it;
                std::string winding_name(winding->getName());
                float64 current(winding->evaluateCurrent());
                stream
                    << "      { Region " << winding_name << "; Value I_" << winding_name << "[]*Idir[];}\n";
            }
            stream
                << "    }\n"
                << "  }\n";
        }
        else if (type_ == 1) {
            // 전류 조건
            stream
                << "  { Name Current_2D;\n"
                << "    Case{\n";
            Node* excite_root = modeler_->getWorkingExcitationRootNode();
            for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
                WindingNode* winding = *it;
                std::string winding_name(winding->getName());
                float64 current(winding->evaluateCurrent());
                stream
                    << "      { Region " << winding_name << "; Value 1*Idir[]; TimeFunction I_" << winding_name << "[];}\n";
            }
            stream
                << "    }\n"
                << "  }\n";

            // 전압조건
            stream
                << "  { Name Voltage_2D;\n"
                << "    Case {\n"
                << "      { Region DomainC; Value 0.; }\n"
                << "    }\n"
                << "  }\n";
        }
    }

    stream
        << "}\n";

    constraint_ = stream.str(); // 최종 문자열 생성

    // constraint_ 출력
    //std::cout << constraint_ << std::endl;
}

void GetDPHelper::generateFunctionSpace()
{
    std::ostringstream stream;
    stream
        << "FunctionSpace {\n";
    // Magnetic vector potential a (b = curl a)
    {
        stream
            << "  { Name Hcurl_a_2D; Type Form1P;\n"
            << "    BasisFunction {\n"
            << "      { Name se1; NameOfCoef ae1; Function BF_PerpendicularEdge; ";

        // 무빙밴드가 있으면 영역에 밴드까지 추가해야 함
        stream
            << "Support Region[{Domain";
        if (mb_) {
            BCNode* inner = mb_->getInnerBCNode();
            std::string mb_name(mb_->getName().c_str());
            std::string inner_name(inner->getName().c_str());
            std::string inner_name2 = "MB_" + mb_name + "_" + inner_name;
            stream
                << "," << inner_name2 << "Aux";
        }
        stream
            << "}]; Entity NodesOf [ All ]; } }\n";

        stream
            << "    Constraint {\n"
            << "      { NameOfCoef ae1; EntityType NodesOf; NameOfConstraint MVP_2D; } }\n"
            << "  }\n";

        stream
            << "  { Name Hregion_i_Mag_2D; Type Vector;\n"
            << "    BasisFunction {\n"
            << "      { Name sr; NameOfCoef ir; Function BF_RegionZ; Support DomainB; Entity DomainB; } }\n"
            << "    GlobalQuantity {\n"
            << "      { Name Ib; Type AliasOf; NameOfCoef ir; } }\n"
            << "    Constraint {\n"
            << "      { NameOfCoef Ib; EntityType Region; NameOfConstraint Current_2D; } }\n"
            << "  }\n";
    }

    // Gradient of Electric scalar potential (2D)
    {
        if (type_ == 1) {
            stream
                << "  { Name Hregion_u_Mag_2D; Type Form1P;\n"
                << "    BasisFunction {\n"
                << "      { Name sr; NameOfCoef ur; Function BF_RegionZ; Support DomainC; Entity DomainC; } }\n"
                << "    GlobalQuantity {\n"
                << "      { Name U; Type AliasOf; NameOfCoef ur; }\n"
                << "      { Name I; Type AssociatedWith; NameOfCoef ur; } }\n"
                << "    Constraint {\n"
                << "      { NameOfCoef U; EntityType GroupsOfNodesOf; NameOfConstraint Voltage_2D; }\n"
                << "      { NameOfCoef I; EntityType GroupsOfNodesOf; NameOfConstraint Current_2D; } }\n"
                << "  }\n";
        }
    }

    stream
        << "}\n";

    function_space_ = stream.str(); // 최종 문자열 생성

    // function_space_ 출력
    //std::cout << function_space_.c_str() << std::endl;
}

void GetDPHelper::generateFormulation()
{
    std::ostringstream stream;
    stream
        << "Formulation {\n"
        << "  { Name bzMag; Type FemEquation;\n";

    {
        stream
            << "    Quantity {\n"
            << "      { Name a; Type Local; NameOfSpace Hcurl_a_2D; }\n"
            << "      { Name ir; Type Local; NameOfSpace Hregion_i_Mag_2D; }\n"
            << "      { Name Ib; Type Global; NameOfSpace Hregion_i_Mag_2D [Ib]; }\n";

        if (type_ == 1) {
            stream
                << "      { Name ur; Type Local; NameOfSpace Hregion_u_Mag_2D; }\n"
                << "      { Name I; Type Global; NameOfSpace Hregion_u_Mag_2D[I]; }\n"
                << "      { Name U; Type Global; NameOfSpace Hregion_u_Mag_2D[U]; }\n";
        }
        stream
            << "    }\n";
    }

    {
        stream
            << "    Equation {\n"
            << "      Galerkin { [  nu[] * Dof{Curl a}, {Curl a} ]; In DomainL; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { [  nu[{Curl a}] * {Curl a}, {Curl a} ]; In DomainNL; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { [  dhdb[{Curl a}] * Dof{Curl a}, {Curl a} ]; In DomainNL; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { [ -dhdb[{Curl a}] * {Curl a}, {Curl a} ]; In DomainNL; Jacobian Vol; Integration I1; }\n";

        // 무빙밴드 사용시 토크계산을 위해서 필요한 코드임
        if (mb_) {
            BCNode* inner = mb_->getInnerBCNode();
            std::string mb_name(mb_->getName().c_str());
            std::string inner_name(inner->getName().c_str());
            std::string inner_name2 = "MB_" + mb_name + "_" + inner_name;
            stream
                << "      Galerkin { [  0*Dof{Curl a}, {Curl a} ]; "
                << "In " << inner_name2 << "Aux; "
                << "Jacobian Sur; Integration I1; }\n";
        }

        stream
            << "      Galerkin { [ -nu[] * Br[], {Curl a} ]; In DomainM; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { [ -NbWires[]/SurfCoil[] * Dof{ir}, {a} ]; In DomainB; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { DtDof [ AxialLength * NbWires[]/SurfCoil[] * Dof{a}, {ir} ]; In DomainB; Jacobian Vol; Integration I1; }\n"
            << "      Galerkin { [ 1/SurfCoil[]* Dof{ir}, {ir} ]; In DomainB; Jacobian Vol; Integration I1; }\n"
            << "      GlobalTerm { [ Dof{Ib}, {Ib} ]; In DomainB; }\n";

        if (type_ == 1) {
            // 도체영역(와전류)
            // Electric field e = -Dt[{a}]-{ur}, with {ur} = Grad v constant in each region of DomainC
            // DtDof[] : Time derivative applied only to the Dof{} term of the equation.
            stream
                << "      Galerkin { DtDof[ sigma[] * Dof{a}, {a} ]; In DomainC; Jacobian Vol; Integration I1; }\n"
                << "      Galerkin { [ sigma[] * Dof{ur}, {a} ]; In DomainC; Jacobian Vol; Integration I1; }\n";

            // 도체영역(내부변수와 글로벌변수(0-전류원) 연결)
            // When {ur} act as a test function, one obtains the circuits relations,
            // relating the voltage and the current of each region in DomainC
            stream
                << "      Galerkin { DtDof[ sigma[] * Dof{a}, {ur} ]; In DomainC; Jacobian Vol; Integration I1; }\n"
                << "      Galerkin { [ sigma[] * Dof{ur}, {ur} ]; In DomainC; Jacobian Vol; Integration I1; }\n"
                << "      GlobalTerm { [ Dof{I}, {U} ]; In DomainC; }\n";
        }

        stream
            << "    }\n";
    }

    stream
        << "  }\n"
        << "}\n";

    formulation_ = stream.str(); // 최종 문자열 생성

    // formulation_ 출력
    //std::cout << formulation_ << std::endl;
}

void GetDPHelper::generateResolution()
{
    std::ostringstream stream;
    stream
        << "Resolution {\n"
        << "  { Name Analysis ;\n"
        << "    System {\n"
        << "      { Name A; NameOfFormulation bzMag; }\n"
        << "    }\n"
        << "    Operation {\n"
        << "      Evaluate[$RotorPosition = 0];\n"
        << "      Print[{$RotorPosition}, Format \"Rotor Position : % 03g\"];\n"
        << "      CreateDir[ResDir];\n";
    Node* excite_root = modeler_->getWorkingExcitationRootNode();
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        std::string winding_name(winding->getName().c_str());
        stream
            << "      DeleteFile[StrCat[ResDir,\"Current_" << winding_name << "\",ExtGnuplot]];\n"
            << "      DeleteFile[StrCat[ResDir,\"InducedVoltage_" << winding_name << "\",ExtGnuplot]];\n"
            << "      DeleteFile[StrCat[ResDir,\"FluxLinkage_" << winding_name << "\",ExtGnuplot]];\n";
    }

    if (mb_) {
        std::string mb_name(mb_->getName().c_str());
        std::string torque_name1 = "Torque_" + mb_name + "_Outer";
        std::string torque_name2 = "Torque_" + mb_name + "_Inner";

        stream
            << "      DeleteFile[StrCat[ResDir,\"" << torque_name1 << "\",ExtGnuplot]];\n"
            << "      DeleteFile[StrCat[ResDir,\"" << torque_name2 << "\",ExtGnuplot]];\n"
            << "      InitMovingBand2D[MB_" << mb_name << "];\n"
            << "      MeshMovingBand2D[MB_" << mb_name << "];\n";
    }

    if (type_ == 0) {
        stream
            << "      InitSolution[A];\n"
            << "      Generate[A]; Solve[A];\n"
            << "      If(NbrRegions[DomainNL])\n"
            << "        Generate[A]; GetResidual[A, $res0];\n"
            << "        Evaluate[ $res = $res0, $iter = 0 ];\n"
            << "        Print[{$iter, $res, $res / $res0}, Format \"Residual % 03g: abs % 14.12e rel % 14.12e\"];\n"
            << "        While[$res > NL_tol_abs && $res / $res0 > NL_tol_rel && $res / $res0 <= 1 && $iter < NL_iter_max] {\n"
            << "          Solve[A]; Generate[A]; GetResidual[A, $res];\n"
            << "          Evaluate[ $iter = $iter + 1 ];\n"
            << "          Print[{$iter, $res, $res / $res0}, Format \"Residual % 03g: abs % 14.12e rel % 14.12e\"];\n"
            << "        }\n"
            << "      EndIf\n"
            << "      SaveSolution[A];\n"
            << "      Print[{1,1}, Format \"bzMagProcessing [%g, %g]\"];\n"
            << "      PostOperation[Get_LocalFields];\n"
            << "      PostOperation[Get_GlobalQuantities];\n"
            << "    }\n"
            << "  }\n"
            << "}\n";
    }

    else if (type_ == 1) {
        stream
            << "      InitSolution[A]; InitSolution[A];\n"
            << "      SaveSolution[A];\n"
            << "      TimeLoopTheta[time0, timemax, delta_time, 1.] { // Euler implicit (1) -- Crank-Nicolson (0.5)\n"
            << "        Generate[A]; Solve[A];\n"
            << "        If(NbrRegions[DomainNL])\n"
            << "          Generate[A]; GetResidual[A, $res0];\n"
            << "          Evaluate[ $res = $res0, $iter = 0 ];\n"
            << "          Print[{$iter, $res, $res / $res0}, Format \"Residual % 03g: abs % 14.12e rel % 14.12e\"];\n"
            << "          While[$res > NL_tol_abs && $res / $res0 > NL_tol_rel && $res / $res0 <= 1 && $iter < NL_iter_max] {\n"
            << "            Solve[A]; Generate[A]; GetResidual[A, $res];\n"
            << "            Evaluate[ $iter = $iter + 1 ];\n"
            << "            Print[{$iter, $res, $res / $res0}, Format \"Residual % 03g: abs % 14.12e rel % 14.12e\"];\n"
            << "          }\n"
            << "        EndIf\n"
            << "        SaveSolution[A];\n"
            << "        Print[{$TimeStep,NTimeSteps}, Format \"bzMagProcessing [%g, %g]\"];\n"
            << "        PostOperation[Get_LocalFields];\n"
            << "        PostOperation[Get_GlobalQuantities];\n";

        if (mb_) {
            std::string mb_name(mb_->getName().c_str());
            std::string torque_name1 = "Torque_" + mb_name + "_Outer";
            std::string torque_name2 = "Torque_" + mb_name + "_Inner";
            stream
                << "        Test[ $TimeStep > 1 ] {\n"
                << "          PostOperation[Get_Torque];\n"
                << "        }\n"
                << "        {\n";
            stream
                << "          Evaluate[ $" << torque_name1 << " = 0 ];\n"
                << "          Evaluate[ $" << torque_name2 << " = 0 ];\n";
            stream
                << "        }\n"
                << "        ChangeOfCoordinates[ NodesOf[Moving_" << mb_name << "], RotatePZ[delta_theta]];\n"
                << "        MeshMovingBand2D[MB_" << mb_name << "];\n"
                << "        Evaluate[$RotorPosition = $RotorPosition + delta_theta];\n"
                << "        Print[{$RotorPosition}, Format \"Rotor Position : % 03g\"];\n";
        }

        stream
            << "      }\n"
            << "    }\n"
            << "  }\n"
            << "}\n";
    }

    resolution_ = stream.str(); // 최종 문자열 생성

    // resolution_ 출력
    //std::cout << resolution_ << std::endl;
}

void GetDPHelper::generatePostProcessing()
{
    std::ostringstream stream;
    stream
        << "PostProcessing {\n"
        << "  { Name Fields; NameOfFormulation bzMag;\n"
        << "    PostQuantity {\n"
        << "      { Name A; Value { Term { [ {a} ]; In Domain; Jacobian Vol; } } }\n"
        << "      { Name Az; Value { Term { [ CompZ[{a}] ]; In Domain; Jacobian Vol; } } }\n"
        << "      { Name B; Value { Term { [ {Curl a} ]; In Domain; Jacobian Vol; } } }\n";

    if (mb_) {
        stream
            << "      { Name B_radial;\n"
            << "        Value { Term { [ {Curl a}* Vector[  Cos[AngularPosition[]#2], Sin[#2], 0.] ]; In Domain; Jacobian Vol; } }\n"
            << "      }\n"
            << "      { Name B_tangent;\n"
            << "        Value { Term { [ {Curl a}* Vector[ -Sin[AngularPosition[]#3], Cos[#3], 0.] ]; In Domain; Jacobian Vol; } }\n"
            << "      }\n";
    }

    stream
        << "      { Name H; Value { Term{ [ nu[] * {Curl a} ]; In DomainL; Jacobian Vol; } \n"
        << "                        Term{ [ nu[{Curl a}] * {Curl a} ]; In DomainNL; Jacobian Vol; } }\n"
        << "      }\n"
        << "      { Name Flux; Value { Integral { [ SymmetryFactor*AxialLength*Idir[]*NbWires[]/SurfCoil[]* CompZ[{a}] ]; In Inds; Jacobian Vol; Integration I1; } } }\n"
        << "      { Name Js; Value { Term { [ NbWires[]/SurfCoil[] * {ir} ]; In DomainB; Jacobian Vol; } } }\n"
        << "      { Name Torque_Maxwell; Value {\n"
        << "          Integral {\n"
        << "            [ CompZ [ XYZ[] /\\ (T_max[{Curl a}] * XYZ[]) ] * 2*Pi*AxialLength/SurfaceArea[] ];\n"
        << "            In Domain; Jacobian Vol; Integration I1;} }\n"
        << "      }\n";

    stream
        << "      { Name I; Value {\n";

    if (type_ == 1) {
        stream
            << "          Term { [ {I} ]; In DomainC; }\n";
    }

    stream
        << "          Term { [ {Ib} ]; In DomainB; } }\n"
        << "      }\n";


    if (type_ == 1) {
        stream
            << "      { Name E; Value { Integral { [ SymmetryFactor*AxialLength*Idir[]*NbWires[]/SurfCoil[]* CompZ[ Dt[ {a} ] ] ]; In Inds; Jacobian Vol; Integration I1; } } }\n";

    }

     stream
        << "    }\n"
        << "  }\n"
        << "}\n";

    post_processing_ = stream.str(); // 최종 문자열 생성

    // post_processing_ 출력
    //std::cout << post_processing_.c_str() << std::endl;
}

void GetDPHelper::generatePostOperation()
{
    std::ostringstream stream;

    stream
        << "po  = StrCat[\"Output - Electromagnetics/\", ResId];\n"
        << "poI = StrCat[po,\"0Current [A]/\"];\n"
        << "poF = StrCat[po,\"1Flux linkage [Vs]/\"];\n"
        << "poV = StrCat[po,\"2Voltage [V]/\"];\n"
        << "poT = StrCat[po,\"3Torque [Nm]/\"];\n\n"
        << "PostOperation Get_LocalFields UsingPost Fields {\n"
        << "  //Print[ Js, OnElementsOf Inds, File StrCat[ResDir,\"Js\",ExtGmsh],\n"
        << "  //  LastTimeStepOnly, AppendTimeStepToFileName Flag_SaveAllSteps];\n"
        << "  Print[ B, OnElementsOf Domain, File StrCat[ResDir,\"B\",ExtGmsh],\n"
        << "    LastTimeStepOnly, AppendTimeStepToFileName Flag_SaveAllSteps];\n"
        << "  Print[ Az, OnElementsOf Domain, File StrCat[ResDir,\"A\",ExtGmsh],\n"
        << "    LastTimeStepOnly, AppendTimeStepToFileName Flag_SaveAllSteps];\n"
        << "}\n";

    if (mb_) {
        std::string mb_name(mb_->getName().c_str());
        std::string torque_name1 = "Torque_" + mb_name + "_Outer";
        std::string torque_name2 = "Torque_" + mb_name + "_Inner";

        std::string mb_name1 = "MB_" + mb_name + "_Outer";
        std::string mb_name2 = "MB_" + mb_name + "_Inner";
        stream
            << "PostOperation Get_Torque UsingPost Fields {\n";

        stream
            << "  Print[ Torque_Maxwell[" << mb_name1 << "], OnGlobal, Format TimeTable,\n"
            << "    File > StrCat[ResDir,\"" << torque_name1 << "\",ExtGnuplot], LastTimeStepOnly,\n"
            << "    StoreInVariable $" << torque_name1 << ", SendToServer StrCat[poT, \"" << torque_name1 << "\"]{0}];\n"
            << "  Print[ Torque_Maxwell[" << mb_name2 << "], OnGlobal, Format TimeTable,\n"
            << "    File > StrCat[ResDir,\"" << torque_name2 << "\",ExtGnuplot], LastTimeStepOnly,\n"
            << "    StoreInVariable $" << torque_name2 << ", SendToServer StrCat[poT, \"" << torque_name2 << "\"]{0}];\n";

        stream
            << "}\n";
    }

    // 권선영역에 대한 쇄교자속, 유기전압
    stream
        << "PostOperation Get_GlobalQuantities UsingPost Fields {\n";

    Node* excite_root = modeler_->getWorkingExcitationRootNode();
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        WindingNode* winding = *it;
        
        std::string winding_name(winding->getName().c_str());
        stream
            << "  Print[ Flux[" << winding_name << "], OnGlobal, Format TimeTable,\n"
            << "    File > StrCat[ResDir,\"FluxLinkage_" << winding_name << "\",ExtGnuplot], LastTimeStepOnly,\n"
            << "    StoreInVariable $Flux_" << winding_name << ", SendToServer StrCat[poF,\"" << winding_name << "\"]{0}];\n";

        stream
            << "  Print[ I, OnRegion " << winding_name << "_pos, Format Table,\n"
            << "    File > StrCat[ResDir,\"Current_" << winding_name << "\",ExtGnuplot], LastTimeStepOnly,\n"
            << "    SendToServer StrCat[poI,\"" << winding_name << "\"]{0}];\n";

        if (type_ == 1) {
            stream
                << "  Print[ E[" << winding_name << "], OnGlobal, Format TimeTable,\n"
                << "    File > StrCat[ResDir,\"InducedVoltage_" << winding_name << "\",ExtGnuplot], LastTimeStepOnly,\n"
                << "    StoreInVariable $E_" << winding_name << ", SendToServer StrCat[poV,\"" << winding_name << "\"]{0}];\n";
        }
    }

    stream
        << "}\n";
    post_operation_ = stream.str(); // 최종 문자열 생성

    // post_operation_ 출력
    //std::cout << post_operation_.c_str() << std::endl;
}

std::vector<int> GetDPHelper::findMovingObject(const Polygon_set_2& polyset)
{
    std::vector<int> results;
    Node* geom_root = modeler_->getWorkingGeomRootNode();
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        GeomHeadNode* head = it->get<GeomHeadNode*>();
        if (head && head->isCovered() && head->isModelNode()) {
            Polygon_set_2 ps = head->getPolyset();
            ps.difference(polyset);
            if (ps.is_empty()) {
                results.push_back(head->getID() * 100);
            }
        }
    }
    if (mb_) {
        results.push_back(mb_->getInnerAirgapID() * 100);
    }
    return results;
}

std::string GetDPHelper::convertFunctionCall(const std::string& term)
{
    // 함수 호출 추출 (예: sin($x) -> sin, cos($y) -> cos 등)
    std::regex function_call_regex("([a-zA-Z_][a-zA-Z0-9_]*)\\(([^)]*)\\)");
    std::smatch match;

    std::string converted = term;

    // 함수 호출 탐지 및 변환
    while (std::regex_search(converted, match, function_call_regex)) {
        std::string func_name = match[1].str(); // 함수 이름
        std::string args = match[2].str();     // 함수 인자

        // 함수 이름 변환
        auto it = function_map_.find(func_name);
        std::string new_func_name = (it != function_map_.end()) ? it->second : func_name;

        // 변수 변환: $X -> X[], $Y -> Y[], $Z -> Z[], $TIME -> $Time, _pi -> Pi
        args = std::regex_replace(args, std::regex("\\$X"), "X[]");
        args = std::regex_replace(args, std::regex("\\$Y"), "Y[]");
        args = std::regex_replace(args, std::regex("\\$Z"), "Z[]");
        args = std::regex_replace(args, std::regex("\\$TIME"), "$Time");
        args = std::regex_replace(args, std::regex("\\_pi"), "Pi");

        // GetDP 방식으로 함수 호출 생성 (괄호를 대괄호로 변환)
        std::string new_call = new_func_name + "[" + args + "]";
        converted = converted.replace(match.position(0), match.length(0), new_call);
    }

    return converted;
}
