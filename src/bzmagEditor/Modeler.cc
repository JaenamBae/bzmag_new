#include "Modeler.h"
#include "TemplateNode.h"
#include "core/kernel.h"
#include "core/enumproperty.h"
#include "core/simplepropertybinder.h"
#include "engine/Expression.h"
#include "engine/ExpressionServer.h"
#include "engine/EngineNodeStringConverter.h"
#include <regex>

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(Modeler, Node);

const String Modeler::template_root_ = "/template";
const String Modeler::name_geometry_ = "geometry";
const String Modeler::name_coordinate_ = "coordinate_system";
const String Modeler::name_material_ = "material";
const String Modeler::name_BC_ = "boundary_condition";
const String Modeler::name_excitation_ = "excitation";
const String Modeler::name_setup_ = "setup";
const String Modeler::name_post_ = "post_view";
const String Modeler::name_default_cs_ = "global";
const String Modeler::name_default_material_ = "vaccume";
const String Modeler::name_default_setup_ = "default";

//----------------------------------------------------------------------------
Modeler::Modeler(void)
{
    Kernel* kernel = Kernel::instance();

    time_ = new Expression();
    x_ = new Expression();
    y_ = new Expression();
    z_ = new Expression();

    time_->setKey("$TIME");
    x_->setKey("$X");
    y_->setKey("$Y");
    z_->setKey("$Z");

    time_->setExpression("0");
    x_->setExpression("0");
    y_->setExpression("0");
    z_->setExpression("0");

    // 빈 템플릿 노드 생성 --> Default ; 필수
    default_template_ = dynamic_cast<TemplateNode*>(kernel->create("TemplateNode"));
    if (default_template_) {
        default_template_->setName("Default");
        this->attach(default_template_);
        
    }

    setWorkingTemplate(default_template_);
}

//----------------------------------------------------------------------------
Modeler::~Modeler(void)
{

}

void Modeler::setWorkingTemplateAsDefault()
{
    setWorkingTemplate(default_template_);
}

//----------------------------------------------------------------------------
void Modeler::setWorkingTemplate(TemplateNode* template_node)
{
    if (!template_node || (template_node == working_template_)) return;

    Kernel* kernel = Kernel::instance();
    Node* root_node = kernel->getRoot();

    // 기존 "geometry" 노드를 디테치
    String geom_root_path = "/" + Modeler::getGeometryName();
    Node* prev_geom_root = kernel->lookup(geom_root_path.c_str());
    if (prev_geom_root) prev_geom_root->detach();

    // 기존 "coordinate" 노드를 디테치
    String cs_root_path = "/" + Modeler::getCoordinateSystemName();
    Node* prev_cs_root = kernel->lookup(cs_root_path.c_str());
    if (prev_cs_root) prev_cs_root->detach();
    
    // 기존 "material" 노드를 디테치
    String geom_material_path = "/" + Modeler::getMaterialName();
    Node* prev_material_root = kernel->lookup(geom_material_path.c_str());
    if (prev_material_root)prev_material_root->detach();
    
    // 기존 BC" 노드를 디테치
    String geom_BC_path = "/" + Modeler::getBCName();
    Node* prev_BC_root = kernel->lookup(geom_BC_path.c_str());
    if (prev_BC_root) prev_BC_root->detach();
    
    // 기존 "excitation" 노드를 디테치
    String geom_excitation_path = "/" + Modeler::getExcitationName();
    Node* prev_excitation_root = kernel->lookup(geom_excitation_path.c_str());
    if (prev_excitation_root) prev_excitation_root->detach();

    // 기존 "setup" 노드를 디테치
    String geom_setup_path = "/" + Modeler::getSetupName();
    Node* prev_setup_root = kernel->lookup(geom_setup_path.c_str());
    if (prev_setup_root) prev_setup_root->detach();

    // 기존 "post" 노드를 디테치
    String geom_post_path = "/" + Modeler::getPostName();
    Node* prev_post_root = kernel->lookup(geom_post_path.c_str());
    if (prev_post_root) prev_post_root->detach();

    // 기존 수식들을 disabled 상태로 만든다
    if (working_template_) {
        for (auto it = working_template_->firstExpression(); it != working_template_->lastExpression(); ++it) {
            it->second->disable();
        }
    }

    // 새로운 수식들을 enabled 상태로 만든다
    for (auto it = template_node->firstExpression(); it != template_node->lastExpression(); ++it) {
        it->second->enable();
    }
    
    // 새로운 노드를 붙인다
    root_node->attach(template_node->getGeomRootNode());
    root_node->attach(template_node->getCoordRootNode());
    root_node->attach(template_node->getMaterialRootNode());
    root_node->attach(template_node->getBCRootNode());
    root_node->attach(template_node->getExcitationRootNode());
    root_node->attach(template_node->getSetupRootNode());
    root_node->attach(template_node->getPostRootNode());

    // 템플릿의 기본 CS노드를 현재 사용중인 CS노드로 설정한다
    current_cs_node_ = template_node->getDefaultCSNode();

    // 새로운 템플릿 설정
    working_template_ = template_node;
}

//----------------------------------------------------------------------------
TemplateNode* Modeler::getWorkingTemplate()
{
    return working_template_;
}

//----------------------------------------------------------------------------
TemplateNode* Modeler::getDefaultTemplate()
{
    return default_template_;
}

//----------------------------------------------------------------------------
bool Modeler::loadSavedData(const BzmagFileManager& saved, std::function<void(int)> progressCallback)
{
    json templete_section = saved.getSectionData("template");

    // 템플릿 데이터를 읽는다
    if (!templete_section.empty()) {
        // 해당 템플릿을 working_template로 설정
        if (templete_section.contains("name") && templete_section["name"].is_string()) {
            std::string template_name = templete_section["name"];

            for (auto it = firstChildNode(); it != lastChildNode(); ++it) {
                TemplateNode* node = *it;
                if (node && (node->getName() == template_name)) {
                    TemplateManager* manager = node->getManager();
                    setWorkingTemplate(node);
                    if (manager) {
                        return manager->loadSavedData(saved);
                    }
                }
            }
        }
        return false;
    }

    // 일반 데이터를 읽는다
    else {
        // 기존 default_template_데이터를 삭제하고 초기화
        default_template_->initialize();

        // 디폴트 템플릿을 워킹 템플릿으로
        setWorkingTemplate(default_template_);

        //-----------------------------------------------
        // 다음 순서대로 빌드한다
        // 
        // 첫번째: 수식 빌드
        if (!buildVariable(saved.getSectionData("expression"), default_template_)) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(1.0f / 7.0f * 100));
        }

        // 두번째: 좌표계 빌드
        if (!buildSection(saved.getSectionData("coordinate_system"), default_template_->getCoordRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(2.0f / 7.0f * 100));
        }

        // 세번째: 재질 빌드
        if (!buildSection(saved.getSectionData("material"), default_template_->getMaterialRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(3.0f / 7.0f * 100));
        }

        // 네번째: 형상 빌드
        if (!buildSection(saved.getSectionData("geometry"), default_template_->getGeomRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(4.0f / 7.0f * 100));
        }

        // 다섯번째: 경계조건 빌드
        if (!buildSection(saved.getSectionData("boundary_condition"), default_template_->getBCRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(5.0f / 7.0f * 100));
        }

        // 여섯번째: 여자조건 빌드
        if (!buildSection(saved.getSectionData("excitation"), default_template_->getExcitationRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(int(6.0f / 7.0f * 100));
        }

        // 일곱번째: 솔루션 셋업조건 빌드
        if (!buildSection(saved.getSectionData("setup"), default_template_->getSetupRootNode())) {
            return false;
        }
        if (progressCallback) {
            progressCallback(100);
        }

        // default_template를 working_template로 설정
        setWorkingTemplate(default_template_);
    }
    return true;
}

//----------------------------------------------------------------------------
bool Modeler::saveCurrentData(BzmagFileManager& result, bool without_template)
{
    // 템플릿을 사용한다면 템플릿에게 저장을 맡김
    if (!without_template) {
        if ((working_template_ != default_template_)) {
            if (working_template_) {
                TemplateManager* manager = working_template_->getManager();
                if (manager) {
                    return manager->saveCurrentData(result);
                }
            }
        }
    }

    // 템플릿을 사용하지 않는다면 각 섹션별로 저장
    Kernel* kernel = Kernel::instance();
    ExpressionServer* expr_server = ExpressionServer::instance();

    // 첫번째: Expression을 Json으로 변환
    json expression_section = {
    {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto expr = expr_server->firstExpression(); expr != expr_server->lastExpression(); ++expr) {
        Expression* obj = expr->second;
        if (!obj->isHidden() && obj->isUserDefined()) {
            json new_expression = NodeToJson(obj);
            expression_section["children"].push_back(new_expression);
        }
    }
    result.setSectionData("expression", expression_section);

    // 두번째: 형상을 Json으로 변환
    Node* geom_root = getWorkingGeomRootNode();
    json geometry_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = geom_root->firstChildNode(); it != geom_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_geom = NodeToJson(node);
        geometry_section["children"].push_back(new_geom);
    }
    result.setSectionData("geometry", geometry_section);


    // 세번째: 좌표계를 Json으로 변환
    Node* cs_root = getWorkingCoordRootNode();
    json cs_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = cs_root->firstChildNode(); it != cs_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_cs = NodeToJson(node);
        cs_section["children"].push_back(new_cs);
    }
    result.setSectionData("coordinate_system", cs_section);

    // 네번째: 재질을 Json으로 변환
    Node* mat_root = getWorkingMaterialRootNode();
    json mat_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = mat_root->firstChildNode(); it != mat_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_mat = NodeToJson(node);
        mat_section["children"].push_back(new_mat);
    }
    result.setSectionData("material", mat_section);

    // 다섯번째: 경계조건을 Json으로 변환
    Node* bc_root = getWorkingBCRootNode();
    json bc_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = bc_root->firstChildNode(); it != bc_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_bc = NodeToJson(node);
        bc_section["children"].push_back(new_bc);
    }
    result.setSectionData("boundary_condition", bc_section);

    // 여섯번째: 여자조건을 Json으로 변환
    Node* excite_root = getWorkingExcitationRootNode();
    json excite_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = excite_root->firstChildNode(); it != excite_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_excite = NodeToJson(node);
        excite_section["children"].push_back(new_excite);
    }
    result.setSectionData("excitation", excite_section);

    // 일곱번째: 솔루션 셋업 조건을 Json으로 변환
    Node* setup_root = getWorkingSetupRootNode();
    json setup_section = {
        {"children", json::array()} // 빈 배열로 초기화
    };
    for (auto it = setup_root->firstChildNode(); it != setup_root->lastChildNode(); ++it) {
        Object* node = (*it).get<Object*>();
        json new_excite = NodeToJson(node);
        setup_section["children"].push_back(new_excite);
    }
    result.setSectionData("setup", setup_section);

    return true;
}

//----------------------------------------------------------------------------
void Modeler::lockModeler(bool lock)
{
    locked_ = lock;
}

//----------------------------------------------------------------------------
bool Modeler::isModelerLocked() const
{
    return locked_;
}

//----------------------------------------------------------------------------
bool Modeler::buildVariable(const json& section, TemplateNode* target, bool used_for_template /*=false*/)
{
    // 수식오브젝트 만들기
    if (section.contains("children") && section["children"].is_array()) {
        // 객체 등록
        for (const auto& child : section["children"]) {
            registerVariable(child, target, used_for_template);
        }

        // 값 채우기
        for (const auto& child : section["children"]) {
            fillExpression(child, target);
        }
        return true;
    }
    else {
        return false;
    }
}

//----------------------------------------------------------------------------
bool Modeler::buildSection(const json& section, Node* root)
{
    if (section.contains("children") && section["children"].is_array()) {
        // 노드 생성
        for (const auto& child : section["children"]) {
            buildStructure(child, root, 2);
        }

        // 노드 속성 채우기
        for (const auto& child : section["children"]) {
            fillProperty(child, root);
        }

        return true;
    }
    else {
        return false;
    }
}

//----------------------------------------------------------------------------
Node* Modeler::getDefaultGeomRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getGeomRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getDefaultCoordRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getCoordRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getDefaultMaterialRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getMaterialRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getDefaultBCRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getBCRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getDefaultExcitationRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getExcitationRootNode();
}

bzmag::Node* Modeler::getDefaultSetupRootNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getSetupRootNode();
}

//----------------------------------------------------------------------------
CSNode* Modeler::getDefaultCSNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getDefaultCSNode();
}

//----------------------------------------------------------------------------
MaterialNode* Modeler::getDefaultMaterialNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getDefaultMaterialNode();
}

//----------------------------------------------------------------------------
SolutionSetup* Modeler::getDefaultSetupNode()
{
    if (!default_template_) return nullptr;
    return default_template_->getDefaultSetupNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getWorkingGeomRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getGeomRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getWorkingCoordRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getCoordRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getWorkingMaterialRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getMaterialRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getWorkingBCRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getBCRootNode();
}

//----------------------------------------------------------------------------
Node* Modeler::getWorkingExcitationRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getExcitationRootNode();
}

//----------------------------------------------------------------------------
bzmag::Node* Modeler::getWorkingSetupRootNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getSetupRootNode();
}

//----------------------------------------------------------------------------
CSNode* Modeler::getWorkingDefaultCSNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getDefaultCSNode();
}

//----------------------------------------------------------------------------
MaterialNode* Modeler::getWorkingDefaultMaterialNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getDefaultMaterialNode();
}

//----------------------------------------------------------------------------
SolutionSetup* Modeler::getWorkingDefaultSetupNode()
{
    if (!working_template_) return nullptr;
    return working_template_->getDefaultSetupNode();
}

//----------------------------------------------------------------------------
void Modeler::setCurrentCSNode(CSNode* cs)
{
    current_cs_node_ = cs;
}

CSNode* Modeler::getCurrentCSNode()
{
    return current_cs_node_;
}

void Modeler::clearBelongings()
{
    time_->setExpression("0");
    x_->setExpression("0");
    y_->setExpression("0");
    z_->setExpression("0");

    time_ = nullptr;
    x_ = nullptr;
    y_ = nullptr;
    z_ = nullptr;
}

//----------------------------------------------------------------------------
bool Modeler::update()
{
    Node* geom_root = getWorkingGeomRootNode();
    for (auto child = geom_root->firstChildNode(); child != geom_root->lastChildNode(); ++child) {
        Node* node = *child;
        GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(node);
        if (head) head->forcedUpdate();
    }

    Node* cs_root = getWorkingCoordRootNode();
    for (auto child = cs_root->firstChildNode(); child != cs_root->lastChildNode(); ++child) {
        Node* node = *child;
        CSNode* cs = dynamic_cast<CSNode*>(node);
        if (cs) cs->update();
    }

    Node* mat_root = getWorkingMaterialRootNode();
    for (auto child = mat_root->firstChildNode(); child != mat_root->lastChildNode(); ++child) {
        Node* node = *child;
        MaterialNode* mat = dynamic_cast<MaterialNode*>(node);
        if (mat) mat->update();
    }

    Node* bc_root = getWorkingBCRootNode();
    for (auto child = bc_root->firstChildNode(); child != bc_root->lastChildNode(); ++child) {
        Node* node = *child;
        BCNode* bc = dynamic_cast<BCNode*>(node);
        if (bc) bc->update();
    }

    Node* excit_root = getWorkingExcitationRootNode();
    for (auto child = excit_root->firstChildNode(); child != excit_root->lastChildNode(); ++child) {
        Node* node = *child;
        node->update();
    }

    return true;
}

//----------------------------------------------------------------------------
void Modeler::bindProperty()
{
    BIND_PROPERTY(bool, DrawVertices, &setDrawVertices, &getDrawVertices);
}

void Modeler::buildStructure(const json& item, Node* parent, int depth)
{
   Kernel* kernel = Kernel::instance();

    // 노드의 이름과 타입 출력 (들여쓰기는 depth에 따라)
    Node* node = nullptr;
    if (item.contains("name") && item.contains("type")) {
        String name(item["name"]);
        String type(item["type"]);
        
        // 추가할 노드가 이미 존재한다면 패쓰(기본재질, 기본좌표계)
        Node* child = parent->findChild(name);
        if (!child) {
            node = dynamic_cast<Node*>(kernel->create(type));
            node->setName(name);
            parent->attach(node);
        }
        else {
            node = child;
        }
    }

    // 자식 노드가 있는 경우 순회
    if (item.contains("children") && item["children"].is_array()) {
        for (const auto& child : item["children"]) {
            buildStructure(child, node, depth + 1); // 재귀 호출
        }
    }
}

//----------------------------------------------------------------------------
void Modeler::fillProperty(const json& item, Node* parent)
{
    Kernel* kernel = Kernel::instance();
    Node* node = nullptr;
    if (item.contains("name")) {
        String name(item["name"]);

        // 이미 만들어진 노드를 찾는다
        node = parent->findChild(name);
        
        // 속성을 채운다
        if (node) {
            Type* type = node->getType();
            while (type) {
                for (auto prop = type->firstProperty(); prop != type->lastProperty(); ++prop)
                {
                    String prop_name(prop->second->getName());
                    String prop_type(prop->second->getTypeKeyword());
                    bool readonly = prop->second->isReadOnly();

                    String str_value;
                    if (item.contains(prop_name)) {
                        json value = item[prop_name];
                        if (value.empty()) continue;

                        str_value = value.get<String>();
                    }

                    if (prop_type == "bool" && !readonly) {
                        typedef SimpleProperty<bool> BoolProperty;
                        BoolProperty* property = static_cast<BoolProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type.contains("int") && !readonly) {
                        if (prop_type.contains("uint")) {
                            typedef SimpleProperty<unsigned int> UIntProperty;
                            UIntProperty* property = static_cast<UIntProperty*>(prop->second);
                            property->fromString(node, str_value);
                        }
                        else {
                            typedef SimpleProperty<int> IntProperty;
                            IntProperty* property = static_cast<IntProperty*>(prop->second);
                            property->fromString(node, str_value);
                        }
                    }
                    else if (prop_type.contains("float") && !readonly) {
                        typedef SimpleProperty<double> DoubleProperty;
                        DoubleProperty* property = static_cast<DoubleProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "string" && !readonly) {
                        typedef SimpleProperty<const String&> StringProperty;
                        StringProperty* property = static_cast<StringProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "color" && !readonly) {
                        typedef SimpleProperty<const Color&> ColorProperty;
                        ColorProperty* property = static_cast<ColorProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "vector2" && !readonly) {
                        typedef SimpleProperty<const Vector2&> PointProperty;
                        PointProperty* property = static_cast<PointProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "dataset" && !readonly) {
                        typedef SimpleProperty<const String&> StringProperty;
                        StringProperty* property = static_cast<StringProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "GeomHeadNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "CSNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "MaterialNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "GeomCloneToNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "MasterPeriodicBCNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }
                    else if (prop_type == "MovingBandBCNode" && !readonly) {
                        typedef SimpleProperty<Node*> NodeProperty;
                        NodeProperty* property = static_cast<NodeProperty*>(prop->second);
                        property->fromString(node, str_value);
                    }

                    // GeomSplitNode 의 Plane 속성은 특별처리
                    GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(node);
                    if (spilt && prop_name == "Plane") {
                        typedef EnumProperty<GeomSplitNode::SPLIT_PLANE> SpiltEnumProperty;
                        SpiltEnumProperty* property = static_cast<SpiltEnumProperty*>(prop->second);
                        for (auto it = property->firstEnumerator(); it != property->lastEnumerator(); ++it) {
                            if (it->second == str_value) {
                                property->set(node, it->first);
                            }
                        }
                    }
                }

                type = type->getBase();
            }
        }
    }

    // 자식 노드가 있는 경우 순회
    if (item.contains("children") && item["children"].is_array()) {
        for (const auto& child : item["children"]) {
            fillProperty(child, node); // 재귀 호출
        }
    }
}

//----------------------------------------------------------------------------
void Modeler::registerVariable(const json& item, TemplateNode* target, bool used_for_template/* = false*/)
{
    if (item.contains("Key") && item["Key"].is_string()) {
        String key(item["Key"]);

        // 내용 없이 수식만 등록
        if (target) {
            Expression* expr = target->createExpression(key, "0", "");

            // 템플릿 변수이면 유저에게 노출을 안시키는 걸로
            if (used_for_template) {
#ifndef _DEBUG
                expr->setHidden(true);
#endif
            }
        }
    }
}

//----------------------------------------------------------------------------
void Modeler::fillExpression(const json& item, TemplateNode* target)
{
    if (item.contains("Key") && item["Key"].is_string()) {
        String key(item["Key"]);

        String expression;
        if(item.contains("Expression") && item["Expression"].is_string())
            expression = String(item["Expression"]);

        String comment;
        if (item.contains("Comment") && item["Comment"].is_string())
            comment = String(item["Comment"]);

        // 이미 만들어진 수식을 찾는다
        Expression* expr = target->getExpression(key);
        expr->setExpression(expression);
        expr->setComment(comment);
    }
}

//----------------------------------------------------------------------------
json Modeler::NodeToJson(Object* obj)
{
    if (!obj) return json::object();

    json json_object;

    Type* type = obj->getType();

    // Node 이면 name 과, class type 을 반드시 포함
    Node* node = dynamic_cast<Node*>(obj);
    if (node) {
        json_object["name"] = node->getName();
        json_object["type"] = type->getName();
    }

    // 속성 추가
    while (type) {
        for (auto prop = type->firstProperty(); prop != type->lastProperty(); ++prop) {
            std::string prop_name(prop->second->getName());
            std::string prop_value(prop->second->toString(obj));

            // SpiltNode의 SPILT_PLANE 속성은 enum 타입으로 그 값이 String으로 자동 변환되지 않는다
            GeomSplitNode* spilt = dynamic_cast<GeomSplitNode*>(obj);
            if (spilt && prop_name == "Plane") {
                typedef EnumProperty<GeomSplitNode::SPLIT_PLANE> SplitEnumProperty;
                SplitEnumProperty* property = static_cast<SplitEnumProperty*> (prop->second);
                for (auto it = property->firstEnumerator(); it != property->lastEnumerator(); ++it) {
                    if (spilt->getPlane() == it->first) {
                        prop_value = std::string(it->second.c_str());
                        break;
                    }
                }
            }
            if (prop_name != "ObjectID") {
                json_object[prop_name] = prop_value;
            }
        }
        type = type->getBase();
    }

    // Node 이면 자식 노드에 대해서 작업
    if (node) {
        // 자식 노드 처리
        json children_array = json::array();
        for (auto it = node->firstChildNode(); it != node->lastChildNode(); ++it) {
            json child_json = NodeToJson(*it); // 재귀 호출
            children_array.push_back(child_json);
        }

        if (!children_array.empty()) {
            json_object["children"] = children_array;
        }
    }

    return json_object;
}

GeomLineNode* Modeler::createLine(const bzmag::String& start, const bzmag::String& end, const bzmag::String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_head = "/" + name_geometry_ + "/" + name;
    String path_node = path_head + "/Line";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomLineNode* line = dynamic_cast<GeomLineNode*>(kernel->create("GeomLineNode", path_node));
    line->setParameters(start, end);

    head->setMaterialNode(getWorkingDefaultMaterialNode());
    head->setReferedCS(getWorkingDefaultCSNode());
    line->setReferedCS(getCurrentCSNode());

    return line;
}

//----------------------------------------------------------------------------
GeomCurveNode* Modeler::createCurve(const String& start, const String& end, const String& center, const String& radius, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_head = "/" + name_geometry_ + "/" + name;
    String path_node = path_head + "/Curve";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomCurveNode* line = dynamic_cast<GeomCurveNode*>(kernel->create("GeomCurveNode", path_node));
    line->setParameters(start, end, center, radius);

    head->setMaterialNode(getWorkingDefaultMaterialNode());
    head->setReferedCS(getWorkingDefaultCSNode());
    line->setReferedCS(getCurrentCSNode());

    return line;
}

//----------------------------------------------------------------------------
GeomCoverLineNode* Modeler::createCircle(const String& center, const String& radius, const String& segments, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_head = "/" + name_geometry_ + "/" + name;
    String path_node = path_head + "/Circle";
    String path_cov = path_node + "/Cover";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomCircleNode* circle = dynamic_cast<GeomCircleNode*>(kernel->create("GeomCircleNode", path_node));
    GeomCoverLineNode* cov = dynamic_cast<GeomCoverLineNode*>(kernel->create("GeomCoverLineNode", path_cov));
    circle->setParameters(center, radius, segments);

    head->setMaterialNode(getWorkingDefaultMaterialNode());
    head->setReferedCS(getWorkingDefaultCSNode());
    circle->setReferedCS(getCurrentCSNode());

    return cov;
}

//----------------------------------------------------------------------------
GeomCoverLineNode* Modeler::createRectangle(const String& point, const String& dx, const String& dy, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_head = "/" + name_geometry_ + "/" + name;
    String path_node = path_head + "/Rectangle";
    String path_cov = path_node + "/Cover";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomRectNode* rect = dynamic_cast<GeomRectNode*>(kernel->create("GeomRectNode", path_node));
    GeomCoverLineNode* cov = dynamic_cast<GeomCoverLineNode*>(kernel->create("GeomCoverLineNode", path_cov));
    rect->setParameters(point, dx, dy);

    head->setMaterialNode(getWorkingDefaultMaterialNode());
    head->setReferedCS(getWorkingDefaultCSNode());
    rect->setReferedCS(getCurrentCSNode());

    return cov;
}

//----------------------------------------------------------------------------
GeomCoverLineNode* Modeler::createBand(const String& center, const String& radius, const String& width, const String& segments, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_head = "/" + name_geometry_ + "/" + name;
    String path_node = path_head + "/Band";
    String path_cov = path_node + "/Cover";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomBandNode* band = dynamic_cast<GeomBandNode*>(kernel->create("GeomBandNode", path_node));
    GeomCoverLineNode* cov = dynamic_cast<GeomCoverLineNode*>(kernel->create("GeomCoverLineNode", path_cov));
    band->setParameters(center, radius, width, segments);

    head->setMaterialNode(getWorkingDefaultMaterialNode());
    head->setReferedCS(getWorkingDefaultCSNode());
    band->setReferedCS(getCurrentCSNode());

    return cov;
}

//----------------------------------------------------------------------------
GeomCloneFromNode* Modeler::clone(GeomBaseNode* obj, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    // 이름 복제를 위한 람다 함수
    auto getNewName = [](const std::string& name, const std::smatch& match) -> std::string {
        int number = std::stoi(match[1].str());
        number++;
        std::string base_name = name.substr(0, match.position(0));
        return base_name + "_" + std::to_string(number);
    };

    Kernel* kernel = Kernel::instance();
    GeomCloneToNode* cloneto = nullptr;
    if (dynamic_cast<GeomCloneToNode*>(obj)) {
        cloneto = static_cast<GeomCloneToNode*> (obj);
    }
    else {
        String path_cloneto = obj->getAbsolutePath() + "/CloneTo";
        cloneto = dynamic_cast<GeomCloneToNode*>(kernel->lookup(path_cloneto));
        if (cloneto == nullptr) {
            cloneto = dynamic_cast<GeomCloneToNode*>(kernel->create("GeomCloneToNode", path_cloneto));
        }
    }

    String path_head = "/" + name_geometry_ + "/" + name;
    std::string cloned_name(name);

    // 이미 존재하는 이름인 경우
    // 정규식을 사용하여 "_숫자" 형식으로 끝나는지 검사
    std::regex number_suffix_regex("_(\\d+)$");
    std::smatch match;
    while (kernel->lookup(path_head)) {
        if (std::regex_search(cloned_name, match, number_suffix_regex)) {
            // "_숫자"가 있는 경우 숫자를 증가시키고 반환
            cloned_name = getNewName(cloned_name, match);
        }
        else {
            // "_숫자"가 없는 경우 "_1" 추가
            cloned_name = std::string(name) + "_1";
        }
        path_head = "/" + name_geometry_ + "/" + cloned_name;
    }
    String path_node = path_head + "/CloneFrom";

    GeomHeadNode* head = dynamic_cast<GeomHeadNode*>(kernel->create("GeomHeadNode", path_head));
    GeomCloneFromNode* clonefrom = dynamic_cast<GeomCloneFromNode*>(kernel->create("GeomCloneFromNode", path_node));
    clonefrom->setReferenceNode(cloneto);

    head->setMaterialNode(cloneto->getHeadNode()->getMaterialNode());
    head->setReferedCS(cloneto->getHeadNode()->getReferedCS());
    clonefrom->setReferedCS(getCurrentCSNode());

    return clonefrom;
}

//----------------------------------------------------------------------------
GeomMoveNode* Modeler::move(GeomBaseNode* obj, const String& dx, const String& dy)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_move = obj->getAbsolutePath() + "/MoveTo";
    GeomMoveNode* move = dynamic_cast<GeomMoveNode*>(kernel->create("GeomMoveNode", path_move));
    move->setParameters(dx, dy);
    move->setReferedCS(getCurrentCSNode());

    return move;
}

//----------------------------------------------------------------------------
GeomRotateNode* Modeler::rotate(GeomBaseNode* obj, const String& angle)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_rot = obj->getAbsolutePath() + "/Rotate";
    GeomRotateNode* rot = dynamic_cast<GeomRotateNode*>(kernel->create("GeomRotateNode", path_rot));
    rot->setParameters(angle);
    rot->setReferedCS(getCurrentCSNode());

    return rot;
}

//----------------------------------------------------------------------------
GeomSubtractNode* Modeler::booleanSubtract(GeomBaseNode* lhs, std::vector<GeomBaseNode*> rhs)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_subtract = lhs->getAbsolutePath() + "/Subtract";
    Node* subtract = kernel->lookup(path_subtract);
    if (subtract == nullptr) {
        subtract = kernel->create("GeomSubtractNode", path_subtract);
    }

    for (auto tool : rhs) {
        Node* tool_head = dynamic_cast<Node*>(tool->getHeadNode());
        tool_head->detach();
        subtract->attach(tool_head);
    }

    return dynamic_cast<GeomSubtractNode*>(subtract);
}

//----------------------------------------------------------------------------
GeomUniteNode* Modeler::booleanUnite(GeomBaseNode* lhs, std::vector<GeomBaseNode*> rhs)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_unite = lhs->getAbsolutePath() + "/Unite";
    Node* unite = kernel->lookup(path_unite);
    if (unite == nullptr) {
        unite = kernel->create("GeomUniteNode", path_unite);
    }

    for (auto tool : rhs) {
        Node* tool_head = dynamic_cast<Node*>(tool->getHeadNode());
        tool_head->detach();
        unite->attach(tool_head);
    }

    return dynamic_cast<GeomUniteNode*>(unite);
}

//----------------------------------------------------------------------------
GeomIntersectionNode* Modeler::booleanIntersect(GeomBaseNode* lhs, std::vector<GeomBaseNode*> rhs)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_intersect = lhs->getAbsolutePath() + "/Intersect";
    Node* intersect = kernel->lookup(path_intersect);
    if (intersect == nullptr) {
        intersect = kernel->create("GeomIntersectionNode", path_intersect);
    }

    for (auto tool : rhs) {
        Node* tool_head = dynamic_cast<Node*>(tool->getHeadNode());
        tool_head->detach();
        intersect->attach(tool_head);
    }

    return dynamic_cast<GeomIntersectionNode*>(intersect);
}

//----------------------------------------------------------------------------
GeomSplitNode* Modeler::split(GeomBaseNode* obj, GeomSplitNode::SPLIT_PLANE plane, bool pos, CSNode* cs)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_split = obj->getAbsolutePath() + "/Split";
    GeomSplitNode* split = dynamic_cast<GeomSplitNode*>(kernel->create("GeomSplitNode", path_split));
    split->setPlane(plane);
    split->setOrientation(pos);
    if (cs) {
        split->setReferedCS(cs);
    }
    else {
        split->setReferedCS(getCurrentCSNode());
    }

    return split;
}

//----------------------------------------------------------------------------
CSNode* Modeler::createCS(const String& origin, const String& angle, const String& name, CSNode* parent)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_cs;
    if (parent && parent != getWorkingDefaultCSNode()) {
        path_cs = parent->getAbsolutePath() + "/" + name;
    }
    else {
        path_cs = "/" + name_coordinate_ + "/" + name;
    }
    CSNode* cs = dynamic_cast<CSNode*>(kernel->create("CSNode", path_cs));
    cs->setParameters(origin, angle);

    return cs;
}

//----------------------------------------------------------------------------
DirichletBCNode* Modeler::createDirichletBC(const String& value, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_BC = "/" + name_BC_ + "/" + name;
    DirichletBCNode* bc = dynamic_cast<DirichletBCNode*>(kernel->create("DirichletBCNode", path_BC));
    bc->setBCValue(value);

    return bc;
}

//----------------------------------------------------------------------------
MasterPeriodicBCNode* Modeler::createMasterBC(bool direction, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_BC = "/" + name_BC_ + "/" + name;
    MasterPeriodicBCNode* bc = dynamic_cast<MasterPeriodicBCNode*>(kernel->create("MasterPeriodicBCNode", path_BC));
    bc->setDirection(direction);

    return bc;
}

//----------------------------------------------------------------------------
SlavePeriodicBCNode* Modeler::createSlaveBC(MasterPeriodicBCNode* pair, bool direction, bool even, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_BC = "/" + name_BC_ + "/" + name;
    SlavePeriodicBCNode* bc = dynamic_cast<SlavePeriodicBCNode*>(kernel->create("SlavePeriodicBCNode", path_BC));
    bc->setDirection(direction);
    bc->setEven(even);
    bc->setPair(pair);

    return bc;
}

MovingBandNode* Modeler::createMovingBand(const String& speed, const String& init_pos, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_BC = "/" + name_BC_ + "/" + name;
    MovingBandNode* bc = dynamic_cast<MovingBandNode*>(kernel->create("MovingBandNode", path_BC));
    bc->setSpeed(speed);
    bc->setInitialPosition(init_pos);

    return bc;
}

//----------------------------------------------------------------------------
GeomHeadRefNode* Modeler::createBCObject(GeomHeadNode* head, const String& name, BCNode* parent)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_bc_obj;
    if (parent) {
        path_bc_obj = parent->getAbsolutePath() + "/" + name;
    }
    else {
        path_bc_obj = "/" + name_BC_ + "/" + name;
    }

    GeomHeadRefNode* bc_obj = dynamic_cast<GeomHeadRefNode*>(kernel->create("GeomHeadRefNode", path_bc_obj));
    bc_obj->setHeadNode(head);

    return bc_obj;
}

//----------------------------------------------------------------------------
MaterialNode* Modeler::createMaterial(const String& name, bool withBH)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_materail = "/" + name_material_ + "/" + name;
    MaterialNode* materail = dynamic_cast<MaterialNode*>(kernel->create("MaterialNode", path_materail));

    if (withBH) {
        String path_bh = path_materail + "/bh_curve";
        DataSetNode* dataset = dynamic_cast<DataSetNode*>(kernel->create("DataSetNode", path_bh));
    }
    return materail;
}

//----------------------------------------------------------------------------
WindingNode* Modeler::createWinding(const String& I, const String& a, const String& name)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_winding = "/" + name_excitation_ + "/" + name;
    WindingNode* winding = dynamic_cast<WindingNode*>(kernel->create("WindingNode", path_winding));
    winding->setCurrent(I);
    winding->setNumberOfParallelBranches(a);

    return winding;
}

//----------------------------------------------------------------------------
CoilNode* Modeler::createCoil(const String& Nc, bool direction, const String& name, WindingNode* parent)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_coil;
    if (parent) {
        path_coil = parent->getAbsolutePath() + "/" + name;
    }
    else {
        path_coil = "/" + name_excitation_ + "/" + name;
    }

    CoilNode* coil = dynamic_cast<CoilNode*>(kernel->create("CoilNode", path_coil));
    coil->setNumberOfTurns(Nc);
    coil->setDirection(direction);

    return coil;
}

//----------------------------------------------------------------------------
Expression* Modeler::createExpression(const String& key, const String& expression, const String& descriptioin)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    if (working_template_) {
        return working_template_->createExpression(key, expression, descriptioin);
    }
    return nullptr;
}

//----------------------------------------------------------------------------
bool Modeler::removeExpression(const String& key)
{
    // 모델러가 잠겨있으면 변경이 안됨
    if (isModelerLocked()) return false;

    if (working_template_) {
        return working_template_->removeExpression(key);
    }
    return false;
}

//----------------------------------------------------------------------------
Transient* Modeler::createTransientSetup(const String& name, SolutionSetup* parent)
{
    // 모델러가 잠겨있으면 생성이 안됨
    if (isModelerLocked()) return nullptr;

    // 디폴트 템플릿을 사용하지 않는 경우는 새로 생성할 수 없음
    if (working_template_ != default_template_) return nullptr;

    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path;
    if (parent) {
        path = parent->getAbsolutePath() + "/" + name;
    }
    else {
        path = "/" + name_setup_ + "/" + name_default_setup_ + "/" + name;
    }

    Transient* setup = dynamic_cast<Transient*>(kernel->create("Transient", path));

    return setup;
}

//----------------------------------------------------------------------------
PostNode* Modeler::createPostObject(const String& name)
{
    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_post = "/" + name_post_ + "/" + name;
    PostNode* post = dynamic_cast<PostNode*>(kernel->create("PostNode", path_post));
    return post;
}

PostPlotNode* Modeler::createPostPlotObject(const bzmag::String& name)
{
    // 이름이 비어 있으면 안됨
    if (name.empty()) return nullptr;

    Kernel* kernel = Kernel::instance();

    String path_post = "/" + name_post_ + "/" + name;
    PostPlotNode* post = dynamic_cast<PostPlotNode*>(kernel->create("PostPlotNode", path_post));
    return post;
}
