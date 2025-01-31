#ifndef BZMAG_MODELER_H
#define BZMAG_MODELER_H

/*
Description : Modeler Server
Last Update : 2024.11.26
Author : Jaenam Bae (jaenam@dongyang.ac.kr)
*/

#include "core/string.h"
#include "core/node.h"
#include "core/ref.h"
#include "core/singleton3.h"

#include "engine/GeomHeadNode.h"
#include "engine/GeomLineNode.h"
#include "engine/GeomCurveNode.h"
#include "engine/GeomCircleNode.h"
#include "engine/GeomRectNode.h"
#include "engine/GeomBandNode.h"
#include "engine/GeomCoverlineNode.h"
#include "engine/GeomMoveNode.h"
#include "engine/GeomRotateNode.h"
#include "engine/GeomSubtractNode.h"
#include "engine/GeomUniteNode.h"
#include "engine/GeomIntersectionNode.h"
#include "engine/GeomSplitNode.h"
#include "engine/GeomClonefromNode.h"
#include "engine/GeomClonetoNode.h"
#include "engine/CSNode.h"
#include "engine/MaterialNode.h"
#include "engine/DataSetNode.h"
#include "engine/MasterPeriodicBCNode.h"
#include "engine/SlavePeriodicBCNode.h"
#include "engine/DirichletBCNode.h"
#include "engine/MovingBandNode.h"
#include "engine/GeomHeadRefNode.h"
#include "engine/CoilNode.h"
#include "engine/WindingNode.h"
#include "engine/Expression.h"
#include "engine/SolutionSetup.h"
#include "engine/Transient.h"
#include "PostNode.h"
#include "PostPlotNode.h"
#include "bzMagFileManager.h"

class TemplateNode;
class Modeler : public bzmag::Node, public bzmag::Singleton3<Modeler>
{
public:
    Modeler(void);
    virtual ~Modeler(void);
    DECLARE_CLASS(Modeler, bzmag::Node);

    void setWorkingTemplateAsDefault();
    void setWorkingTemplate(TemplateNode* template_node);
    TemplateNode* getWorkingTemplate();
    TemplateNode* getDefaultTemplate();
    bool loadSavedData(const BzmagFileManager& saved, std::function<void(int)> progressCallback = 0);
    bool saveCurrentData(BzmagFileManager& current, bool without_template = false);

    void lockModeler(bool lock);
    bool isModelerLocked() const;

    bzmag::Node* getDefaultGeomRootNode();
    bzmag::Node* getDefaultCoordRootNode();
    bzmag::Node* getDefaultMaterialRootNode();
    bzmag::Node* getDefaultBCRootNode();
    bzmag::Node* getDefaultExcitationRootNode();
    bzmag::Node* getDefaultSetupRootNode();
    bzmag::engine::CSNode* getDefaultCSNode();
    bzmag::engine::MaterialNode* getDefaultMaterialNode();
    bzmag::engine::SolutionSetup* getDefaultSetupNode();

    bzmag::Node* getWorkingGeomRootNode();
    bzmag::Node* getWorkingCoordRootNode();
    bzmag::Node* getWorkingMaterialRootNode();
    bzmag::Node* getWorkingBCRootNode();
    bzmag::Node* getWorkingExcitationRootNode();
    bzmag::Node* getWorkingSetupRootNode();
    bzmag::engine::CSNode* getWorkingDefaultCSNode();
    bzmag::engine::MaterialNode* getWorkingDefaultMaterialNode();
    bzmag::engine::SolutionSetup* getWorkingDefaultSetupNode();

    void setCurrentCSNode(bzmag::engine::CSNode* cs);
    bzmag::engine::CSNode* getCurrentCSNode();


public:
    bzmag::engine::GeomLineNode* createLine(const bzmag::String& start, const bzmag::String& end, const bzmag::String& name);
    bzmag::engine::GeomCurveNode* createCurve(const bzmag::String& start, const bzmag::String& end, const bzmag::String& center, const bzmag::String& radius, const bzmag::String& name);
    bzmag::engine::GeomCoverLineNode* createCircle(const bzmag::String& center, const bzmag::String& radius, const bzmag::String& segments, const bzmag::String& name);
    bzmag::engine::GeomCoverLineNode* createRectangle(const bzmag::String& point, const bzmag::String& dx, const bzmag::String& dy, const bzmag::String& name);
    bzmag::engine::GeomCoverLineNode* createBand(const bzmag::String& center, const bzmag::String& radius, const bzmag::String& width, const bzmag::String& segments, const bzmag::String& name);
    bzmag::engine::GeomCloneFromNode* clone(bzmag::engine::GeomBaseNode* obj, const bzmag::String& name);

    bzmag::engine::GeomMoveNode* move(bzmag::engine::GeomBaseNode* obj, const bzmag::String& dx, const bzmag::String& dy);
    bzmag::engine::GeomRotateNode* rotate(bzmag::engine::GeomBaseNode* obj, const bzmag::String& angle);

    bzmag::engine::GeomSubtractNode* booleanSubtract(bzmag::engine::GeomBaseNode* lhs, std::vector<bzmag::engine::GeomBaseNode*> rhs);
    bzmag::engine::GeomUniteNode* booleanUnite(bzmag::engine::GeomBaseNode* lhs, std::vector<bzmag::engine::GeomBaseNode*> rhs);
    bzmag::engine::GeomIntersectionNode* booleanIntersect(bzmag::engine::GeomBaseNode* lhs, std::vector<bzmag::engine::GeomBaseNode*> rhs);
    bzmag::engine::GeomSplitNode* split(bzmag::engine::GeomBaseNode* obj, bzmag::engine::GeomSplitNode::SPLIT_PLANE plane, bool pos, bzmag::engine::CSNode* cs);

    bzmag::engine::CSNode* createCS(const bzmag::String& origin, const bzmag::String& angle, const bzmag::String& name, bzmag::engine::CSNode* parent = nullptr);
    
    bzmag::engine::DirichletBCNode* createDirichletBC(const bzmag::String& value, const bzmag::String& name);
    bzmag::engine::MasterPeriodicBCNode* createMasterBC(bool direction, const bzmag::String& name);
    bzmag::engine::SlavePeriodicBCNode* createSlaveBC(bzmag::engine::MasterPeriodicBCNode* pair, bool direction, bool even, const bzmag::String& name);
    bzmag::engine::MovingBandNode* createMovingBand(const bzmag::String& speed, const bzmag::String& init_pos, const bzmag::String& name);
    bzmag::engine::GeomHeadRefNode* createBCObject(bzmag::engine::GeomHeadNode* head, const bzmag::String& name, bzmag::engine::BCNode* parent = nullptr);

    bzmag::engine::MaterialNode* createMaterial(const bzmag::String& name, bool withBH = false);

    bzmag::engine::WindingNode* createWinding(const bzmag::String& I, const bzmag::String& a, const bzmag::String& name);
    bzmag::engine::CoilNode* createCoil(const bzmag::String& Nc, bool direction, const bzmag::String& name, bzmag::engine::WindingNode* parent = nullptr);
    
    // 수식노드는 별도 관리되어야 함(디테치 함수를 가지고 있지않음)
    bzmag::engine::Expression* createExpression(const bzmag::String& key, const bzmag::String& expression, const bzmag::String& descriptioin);
    bool removeExpression(const bzmag::String& key);

    bzmag::engine::Transient* createTransientSetup(const bzmag::String& name, bzmag::engine::SolutionSetup* setup);
    PostNode* createPostObject(const bzmag::String& name);
    PostPlotNode* createPostPlotObject(const bzmag::String& name);

public:
    bool getDrawVertices() const { return draw_vertices_; }
    void setDrawVertices(bool draw) { draw_vertices_ = draw; }
    
public:
    virtual void clearBelongings() override;
    virtual bool update() override;

public:
    static const bzmag::String& getGeometryName() { return name_geometry_; }
    static const bzmag::String& getCoordinateSystemName() { return name_coordinate_; }
    static const bzmag::String& getMaterialName() { return name_material_; }
    static const bzmag::String& getBCName() { return name_BC_; }
    static const bzmag::String& getExcitationName() { return name_excitation_; }
    static const bzmag::String& getSetupName() { return name_setup_; }
    static const bzmag::String& getPostName() { return name_post_; }
    static const bzmag::String& getDefaultCSName() { return name_default_cs_; }
    static const bzmag::String& getDefaultMaterialName() { return name_default_material_; }
    static const bzmag::String& getDefaultSetupName() { return name_default_setup_; }
    static void bindMethod();
    static void bindProperty();

public:
    static bool buildVariable(const json& section, TemplateNode* target, bool used_for_template = false);
    static bool buildSection(const json& section, bzmag::Node* root);

protected:
    static void buildStructure(const json& item, bzmag::Node* parent, int depth);
    static void fillProperty(const json& item, bzmag::Node* parent);

    static void registerVariable(const json& item, TemplateNode* target, bool used_for_template);
    static void fillExpression(const json& item, TemplateNode* target);

    static json NodeToJson(bzmag::Object* obj);


private:
    static const bzmag::String template_root_;
    static const bzmag::String name_geometry_;
    static const bzmag::String name_coordinate_;
    static const bzmag::String name_material_;
    static const bzmag::String name_BC_;
    static const bzmag::String name_excitation_;
    static const bzmag::String name_setup_;
    static const bzmag::String name_post_;
    static const bzmag::String name_default_cs_;
    static const bzmag::String name_default_material_;
    static const bzmag::String name_default_setup_;

    BzmagFileManager file_managr_;
    TemplateNode* working_template_ = nullptr;
    TemplateNode* default_template_ = nullptr;
    bzmag::engine::CSNode* current_cs_node_ = nullptr;

    bool draw_vertices_ = false;
    float vertex_radi_ = 3.0f;

    // 모델러 잠금 (오브젝트 추가/삭제, 속성변경 불가) 
    bool locked_ = false;

    bzmag::engine::Expression* time_;
    bzmag::engine::Expression* x_;
    bzmag::engine::Expression* y_;
    bzmag::engine::Expression* z_;
};

#endif //BZMAG_MODELER_H