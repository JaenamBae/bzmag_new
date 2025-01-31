#include "MaterialNode.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"
#include "engine/Expression.h"
#include "engine/ExpressionServer.h"
#include "DataSetNode.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(MaterialNode, Node);

//----------------------------------------------------------------------------
MaterialNode::MaterialNode() : conductivity_(nullptr), permeability_(nullptr),
    magnetization_(nullptr), dir_magnetization_x_(nullptr),
    dir_magnetization_y_(nullptr), bh_data_(nullptr)
{
    uint32 key = getID();

    conductivity_ = new Expression();
    permeability_ = new Expression();
    magnetization_ = new Expression();
    dir_magnetization_x_ = new Expression();
    dir_magnetization_y_ = new Expression();

    conductivity_->setKey("conductivity_" + std::to_string(key));
    permeability_->setKey("permeability_" + std::to_string(key));
    magnetization_->setKey("magnetization_" + std::to_string(key));
    dir_magnetization_x_->setKey("dir_magnetization_x_" + std::to_string(key));
    dir_magnetization_y_->setKey("dir_magnetization_y_" + std::to_string(key));

    conductivity_->setExpression("0");
    permeability_->setExpression("1");
    magnetization_->setExpression("0");
    dir_magnetization_x_->setExpression("0");
    dir_magnetization_y_->setExpression("0");

    sconductivity_ = "0";
    spermeability_ = "1";
    smagnetization_ = "0";
    sdir_magnetization_ = "0,0";
}

//----------------------------------------------------------------------------
MaterialNode::~MaterialNode()
{

}

//----------------------------------------------------------------------------
const String& MaterialNode::getPermeability() const
{
    return spermeability_;
}

//----------------------------------------------------------------------------
const String& MaterialNode::getConductivity() const
{
    return sconductivity_;
}

//----------------------------------------------------------------------------
const String& MaterialNode::getMagnetization() const
{
    return smagnetization_;
}

//----------------------------------------------------------------------------
const String& MaterialNode::getDirectionOfMagnetization() const
{
    return sdir_magnetization_;
}

//----------------------------------------------------------------------------
bool MaterialNode::isLinear() const
{
    if (!bh_data_) return true;

    return false;
}

//----------------------------------------------------------------------------
void MaterialNode::setPermeability(const String& mu)
{
    if (!permeability_.valid()) return;

    if (!permeability_->setExpression(mu)) {
        permeability_->setExpression(spermeability_);
        return;
    }

    spermeability_ = mu;
}

//----------------------------------------------------------------------------
void MaterialNode::setConductivity(const String& sigma)
{
    if (!conductivity_.valid()) return;

    if (!conductivity_->setExpression(sigma)) {
        conductivity_->setExpression(sconductivity_);
        return;
    }

    sconductivity_ = sigma;
}

//----------------------------------------------------------------------------
void MaterialNode::setMagnetization(const String& mag)
{
    if (!magnetization_.valid()) return;

    if (!magnetization_->setExpression(mag)) {
        magnetization_->setExpression(smagnetization_);
        return;
    }

    smagnetization_ = mag;
}

//----------------------------------------------------------------------------
void MaterialNode::setDirectionOfMagnetization(const String& dir)
{
    if (!dir_magnetization_x_.valid() || !dir_magnetization_y_.valid()) return;

    auto token = Expression::splitByTopLevelComma(dir);

    // (x,y) 로 분리되지 않으면 실패
    if (token.size() != 2) return;

    // 분리된 경우 x,y 값(스트링) 저장
    String mag_x = token[0];
    String mag_y = token[1];

    // 이전값 임시 저장
    const String& pdir_x = dir_magnetization_x_->getExpression();
    const String& pdir_y = dir_magnetization_x_->getExpression();
    if (!dir_magnetization_x_->setExpression(mag_x) ||
        !dir_magnetization_y_->setExpression(mag_y)) {
        dir_magnetization_x_->setExpression(pdir_x);
        dir_magnetization_y_->setExpression(pdir_y);
        return;
    }

    spatial_dependent_ = false;
    ExpressionServer* server = ExpressionServer::instance();
    {
        server->setExpr(mag_x.c_str());
        const auto& used_vars = server->getUsedVar();
        for (const auto& [name, val] : used_vars) {
            if (name == "$X" || name == "$Y", name == "$Z") {
                spatial_dependent_ = true;
            }
        }
    }
    {
        server->setExpr(mag_y.c_str());
        const auto& used_vars = server->getUsedVar();
        for (const auto& [name, val] : used_vars) {
            if (name == "$X" || name == "$Y", name == "$Z") {
                spatial_dependent_ = true;
            }
        }
    }

    sdir_magnetization_ = dir;
}

float64 MaterialNode::evalPermeability() const
{
    if (permeability_.invalid()) return 0;
    return permeability_->eval();
}

float64 MaterialNode::evalConductivity() const
{
    if (conductivity_.invalid()) return 0;
    return conductivity_->eval();
}

float64 MaterialNode::evalMagnetization() const
{
    if (magnetization_.invalid()) return 0;
    return magnetization_->eval();
}

Vector2 MaterialNode::evalDirectionOfMagnetization() const
{
    if (dir_magnetization_x_.invalid() || dir_magnetization_y_.invalid()) return Vector2();
    return Vector2(dir_magnetization_x_->eval(), dir_magnetization_y_->eval());
}

bool MaterialNode::hasEvenDirectionOfMagnetization() const
{
    return spatial_dependent_;
}

//----------------------------------------------------------------------------
DataSetNode* MaterialNode::getDataSetNode()
{
    return bh_data_;
}

//----------------------------------------------------------------------------
void MaterialNode::setDataSetNode(DataSetNode* node)
{
    bh_data_ = node;
}

//----------------------------------------------------------------------------
const DataSet MaterialNode::getBHdata() const
{
    if (bh_data_) {
        return bh_data_->getDataset();
    }
    return DataSet{};
}

void MaterialNode::clearBelongings()
{
    conductivity_->setExpression("0");
    permeability_->setExpression("0");
    magnetization_->setExpression("0");
    dir_magnetization_x_->setExpression("0");
    dir_magnetization_y_->setExpression("0");

    conductivity_ = nullptr;
    permeability_ = nullptr;
    magnetization_ = nullptr;
    dir_magnetization_x_ = nullptr;
    dir_magnetization_y_ = nullptr;
}

//----------------------------------------------------------------------------
bool MaterialNode::update()
{
    return true;
}

//----------------------------------------------------------------------------
void MaterialNode::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void MaterialNode::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void MaterialNode::bindProperty()
{
    BIND_PROPERTY(const String&, Permeability,
        &setPermeability,
        &getPermeability);

    BIND_PROPERTY(const String&, Conductivity,
        &setConductivity,
        &getConductivity);

    BIND_PROPERTY(const String&, Magnetization,
        &setMagnetization,
        &getMagnetization);

    BIND_PROPERTY(const String&, DirectionOfMagentization,
        &setDirectionOfMagnetization,
        &getDirectionOfMagnetization);
}

