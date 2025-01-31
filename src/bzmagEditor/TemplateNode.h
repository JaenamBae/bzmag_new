#pragma once

#include "core/ref.h"
#include "core/singleton3.h"
#include "core/node.h"
#include "TemplateManager.h"
#include "Modeler.h"
#include <map>

class TemplateNode : public bzmag::Node
{
public:
    using Expressions = std::map<bzmag::String, bzmag::Ref<bzmag::engine::Expression>>;
    using ExprIter = Expressions::iterator;

    TemplateNode();
    virtual ~TemplateNode();
    DECLARE_CLASS(TemplateNode, bzmag::Node);

public:
    void initialize();
    bool loadTemplate(const std::string& name);

    void setProjectPath(const bzmag::String& path);
    const bzmag::String& getProjectPath() const;

    bzmag::Node* getGeomRootNode();
    bzmag::Node* getCoordRootNode();
    bzmag::Node* getMaterialRootNode();
    bzmag::Node* getBCRootNode();
    bzmag::Node* getExcitationRootNode();
    bzmag::Node* getSetupRootNode();
    bzmag::Node* getPostRootNode();
    bzmag::engine::CSNode* getDefaultCSNode();
    bzmag::engine::MaterialNode* getDefaultMaterialNode();
    bzmag::engine::SolutionSetup* getDefaultSetupNode();

    TemplateManager* getManager();

    bzmag::engine::Expression* createExpression(const bzmag::String& key, const bzmag::String& expression, const bzmag::String& descriptioin);
    bool removeExpression(const bzmag::String& key);
    bzmag::engine::Expression* getExpression(const bzmag::String& key);

    ExprIter firstExpression() { return variables_.begin(); };
    ExprIter lastExpression() { return variables_.end(); };

protected:
    void buildConfigureVariables(bool with_default);
    void buildTree();

protected:
    virtual void clearBelongings() override;

private:
    bzmag::String template_name_;
    bzmag::Ref<bzmag::Node> build_root_;

    Expressions variables_;
    bzmag::Ref<bzmag::Node> geom_root_;
    bzmag::Ref<bzmag::Node> cs_root_;
    bzmag::Ref<bzmag::Node> material_root_;
    bzmag::Ref<bzmag::Node> bc_root_;
    bzmag::Ref<bzmag::Node> excitation_root_;
    bzmag::Ref<bzmag::Node> setup_root_;
    bzmag::Ref<bzmag::Node> post_root_;

    bzmag::Ref<bzmag::engine::CSNode> default_cs_ = nullptr;
    bzmag::Ref<bzmag::engine::MaterialNode> default_material_ = nullptr;
    bzmag::Ref<bzmag::engine::SolutionSetup> default_setup_ = nullptr;

    TemplateManager* manager_ = nullptr;

    bzmag::String project_path_;
};