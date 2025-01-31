#include "TemplateNode.h"

#include "core/kernel.h"
#include "core/simplepropertybinder.h"
#include "engine/ExpressionServer.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(TemplateNode, Node);

//----------------------------------------------------------------------------
TemplateNode::TemplateNode()
{
    // 빌드 루트 생성
    Kernel* kernel = Kernel::instance();
    build_root_ = kernel->create("Node");

    geom_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    geom_root_->setName(Modeler::getGeometryName());
    build_root_->attach(geom_root_);

    cs_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    cs_root_->setName(Modeler::getCoordinateSystemName());
    build_root_->attach(cs_root_);

    material_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    material_root_->setName(Modeler::getMaterialName());
    build_root_->attach(material_root_);

    bc_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    bc_root_->setName(Modeler::getBCName());
    build_root_->attach(bc_root_);

    excitation_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    excitation_root_->setName(Modeler::getExcitationName());
    build_root_->attach(excitation_root_);

    setup_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    setup_root_->setName(Modeler::getSetupName());
    build_root_->attach(setup_root_);

    post_root_ = dynamic_cast<Node*>(kernel->create("Node"));
    post_root_->setName(Modeler::getPostName());
    build_root_->attach(post_root_);


    // 글로벌 좌표계 생성
    default_cs_ = dynamic_cast<CSNode*>(kernel->create("CSNode"));
    default_cs_->setName(Modeler::getDefaultCSName());
    cs_root_->attach(default_cs_);

    // 기본 재질 생성
    default_material_ = dynamic_cast<MaterialNode*>(kernel->create("MaterialNode"));
    default_material_->setName(Modeler::getDefaultMaterialName());
    material_root_->attach(default_material_);

    // 기본 솔루션 셋업 생성
    default_setup_ = dynamic_cast<SolutionSetup*>(kernel->create("SolutionSetup"));
    default_setup_->setName(Modeler::getDefaultSetupName());
    setup_root_->attach(default_setup_);
}

//----------------------------------------------------------------------------
TemplateNode::~TemplateNode()
{
    if (manager_) delete manager_;
}

//----------------------------------------------------------------------------
void TemplateNode::initialize()
{
    // 기존 노드들 모두 클리어(클리어 순서 중요함!)
    post_root_->clearChildren<Node>();
    setup_root_->clearChildren<Node>();
    cs_root_->clearChildren<Node>();
    material_root_->clearChildren<Node>();
    bc_root_->clearChildren<Node>();
    excitation_root_->clearChildren<Node>();
    geom_root_->clearChildren<Node>();
    post_root_->clearChildren<Node>();

    // 기존 수식도 모두 삭제
    while (firstExpression() != lastExpression()) {
        auto it = firstExpression();
        Expression* expr = it->second;
        removeExpression(expr->getKey());
    }

    Kernel* kernel = Kernel::instance();

    // 글로벌 좌표계 생성
    default_cs_ = dynamic_cast<CSNode*>(kernel->create("CSNode"));
    default_cs_->setName(Modeler::getDefaultCSName());
    cs_root_->attach(default_cs_);

    // 기본 재질 생성
    default_material_ = dynamic_cast<MaterialNode*>(kernel->create("MaterialNode"));
    default_material_->setName(Modeler::getDefaultMaterialName());
    material_root_->attach(default_material_);

    // 기본 솔루션 셋업 생성
    default_setup_ = dynamic_cast<SolutionSetup*>(kernel->create("SolutionSetup"));
    default_setup_->setName(Modeler::getDefaultSetupName());
    setup_root_->attach(default_setup_);

    // 템플릿 이름
    template_name_.clear();
}

//----------------------------------------------------------------------------
bool TemplateNode::loadTemplate(const std::string& name)
{
    if (manager_) delete manager_;

    // 파일이 존재하는지 판단
    if (!std::filesystem::is_regular_file(name)) {
        return false;
    }

    // 파일 로딩이 되었는지 판단
    manager_ = new TemplateManager(name);
    try {
        // load 함수 호출
        if (!manager_->load()) {
            std::cout << "Fail to load template!" << std::endl;
            return false;
        }
    }
    catch (const std::exception& e) {
        // 예외 처리: 표준 예외
        std::cerr << "Exception caught while loading: " << e.what() << std::endl;
        return false;
    }
    catch (...) {
        // 예외 처리: 기타 모든 예외
        std::cerr << "Unknown exception caught while loading." << std::endl;
        return false;
    }

    template_name_ = manager_->getTemplateName();

    // 현재 노드 이름을 템플릿 이름으로 변경
    this->setName(template_name_);

    // 설정 빌드
    buildConfigureVariables(true);

    // 템플릿에 정의된 모델 빌드
    buildTree();

    return true;
}

//----------------------------------------------------------------------------
void TemplateNode::setProjectPath(const String& path)
{
    project_path_ = path;
}

//----------------------------------------------------------------------------
const String& TemplateNode::getProjectPath() const
{
    return project_path_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getGeomRootNode()
{
    return geom_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getCoordRootNode()
{
    return cs_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getMaterialRootNode()
{
    return material_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getBCRootNode()
{
    return bc_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getExcitationRootNode()
{
    return excitation_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getSetupRootNode()
{
    return setup_root_;
}

//----------------------------------------------------------------------------
Node* TemplateNode::getPostRootNode()
{
    return post_root_;
}

//----------------------------------------------------------------------------
CSNode* TemplateNode::getDefaultCSNode()
{
    return default_cs_;
}

//----------------------------------------------------------------------------
MaterialNode* TemplateNode::getDefaultMaterialNode()
{
    return default_material_;
}

//----------------------------------------------------------------------------
SolutionSetup* TemplateNode::getDefaultSetupNode()
{
    return default_setup_;
}

//----------------------------------------------------------------------------
TemplateManager* TemplateNode::getManager()
{
    return manager_;
}

//----------------------------------------------------------------------------
Expression* TemplateNode::getExpression(const String& key)
{
    auto it = variables_.find(key);
    if (it != variables_.end())
        return it->second;

    return 0;
}

//----------------------------------------------------------------------------
Expression* TemplateNode::createExpression(const String& key, const String& expression, const String& descriptioin)
{
    // key, expression에 대해서는 유효성 체크
    ExpressionServer* expr_server = ExpressionServer::instance();
    if (expr_server->findExpression(key)) {
        return nullptr;
    }
    if (!expr_server->checkConsistancy(key, expression)) {
        return nullptr;
    }

    try {
        Expression* expr = new Expression;
        expr->setUserDefined(true); // 이 속성을 먼저 설정한다
        expr->setHidden(false);     // 이 속성을 먼저 설정한다
        expr->setKey(key);          // setKey() 함수 호출시 자동으로 서버에 등록이 됨
        expr->setExpression(expression);
        expr->setComment(descriptioin);

        // 수식 목록에 추가
        variables_[expr->getKey()] = expr;

        return expr;
    }

    catch (const std::exception& e)
    {
        std::cerr << "[Error] Fail to make Expression: " << e.what() << std::endl;
        return nullptr;
    }
    catch (...)
    {
        std::cerr << "[Error] Unkown exception." << std::endl;
        return nullptr;
    }
}

//----------------------------------------------------------------------------
bool TemplateNode::removeExpression(const String& key)
{
    Expression* expr = getExpression(key);
    if (!expr) return false;

    // 수식의 표현식을 "0"으로 설정하여 다른 수식들과의 관계를 끊음
    expr->setExpression("0");

    expr->disable();        // 서버에서 삭제
    variables_.erase(key);  // 템플릿에서 삭제
    expr->releaseMe();      // 메모리에서 해제

    return true;
}

//----------------------------------------------------------------------------
void TemplateNode::buildConfigureVariables(bool with_default)
{
    // TemplateNode 에 createExpression() 매서드가 있으나 빌드시에는 이를 직접활용하지 않음
    json configure_object = manager_->buildExpressionsFromConfigure(true);
    Modeler::buildVariable(configure_object, this, with_default);
}

//----------------------------------------------------------------------------
void TemplateNode::buildTree()
{
    if (manager_ == nullptr) return;

    json build_object = manager_->getBuildObject();

    if (!build_object.is_object()) {
        std::cerr << "Invalid JSON structure: invalid 'build' structure.\n";
        return;
    }
    else {
        // 첫번째: 수식 빌드
        // TemplateNode 에 createExpression() 매서드가 있으나 빌드시에는 이를 직접활용하지 않고
        // Modeler를 통해 간접 호출한다; 수식만들고 값 채우고 하는 일이 번거롭기 때문이다
        if (build_object.contains("expression") && build_object["expression"].is_object()) {
            Modeler::buildVariable(build_object["expression"], this);
        }

        // 두번째: 좌표계 빌드
        if (build_object.contains("coordinate_system") && build_object["coordinate_system"].is_object()) {
            Modeler::buildSection(build_object["coordinate_system"], getCoordRootNode());
        }

        // 세번째: 재질 빌드
        if (build_object.contains("material") && build_object["material"].is_object()) {
            Modeler::buildSection(build_object["material"], getMaterialRootNode());
        }

        // 네번째: 형상 빌드
        if (build_object.contains("geometry") && build_object["geometry"].is_object()) {
            Modeler::buildSection(build_object["geometry"], getGeomRootNode());
        }

        // 다섯번째: 경계조건 빌드
        if (build_object.contains("boundary_condition") && build_object["boundary_condition"].is_object()) {
            Modeler::buildSection(build_object["boundary_condition"], getBCRootNode());
        }

        // 여섯번째: 여자조건 빌드
        if (build_object.contains("excitation") && build_object["excitation"].is_object()) {
            Modeler::buildSection(build_object["excitation"], getExcitationRootNode());
        }

        // 일곱번째: 솔루션 셋업 조건 빌드
        if (build_object.contains("setup") && build_object["setup"].is_object()) {
            Modeler::buildSection(build_object["setup"], getSetupRootNode());
        }
    }
}

//----------------------------------------------------------------------------
void TemplateNode::clearBelongings()
{
    build_root_ = nullptr;
    geom_root_ = nullptr;
    cs_root_ = nullptr;
    material_root_ = nullptr;
    bc_root_ = nullptr;
    excitation_root_ = nullptr;
    setup_root_ = nullptr;
    post_root_ = nullptr;

    default_cs_ = nullptr;
    default_material_ = nullptr;
    default_setup_ = nullptr;

    variables_.clear();
}
