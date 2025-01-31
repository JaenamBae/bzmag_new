#include "SolutionSetup.h"
#include "core/simplepropertybinder.h"
#include "core/nodeeventpublisher.h"
#include "core/methodbinder.h"
#include "Transient.h"

using namespace bzmag;
using namespace bzmag::engine;

IMPLEMENT_CLASS(SolutionSetup, Node);

//----------------------------------------------------------------------------
SolutionSetup::SolutionSetup()
{

}

//----------------------------------------------------------------------------
SolutionSetup::~SolutionSetup()
{

}

//----------------------------------------------------------------------------
void SolutionSetup::setAbsoluteTolerance(float64 tol)
{
    tol_abs_ = tol;
}

//----------------------------------------------------------------------------
float64 SolutionSetup::getAbsoluteTolerance() const
{
    return tol_abs_;
}

//----------------------------------------------------------------------------
void SolutionSetup::setRelativeTolerance(float64 tol)
{
    tol_rel_ = tol;
}

//----------------------------------------------------------------------------
float64 SolutionSetup::getRelativeTolerance() const
{
    return tol_rel_;
}

//----------------------------------------------------------------------------
void SolutionSetup::setMaxIteration(int32 iter)
{
    iter_max_ = iter;
}

//----------------------------------------------------------------------------
int32 SolutionSetup::getMaxIteration() const
{
    return iter_max_;
}

//----------------------------------------------------------------------------
void SolutionSetup::setSymmetrcyFactor(float64 factor)
{
    symmetry_factor_ = factor;
}

//----------------------------------------------------------------------------
float64 SolutionSetup::getSymmetrcyFactor() const
{
    return symmetry_factor_;
}

//----------------------------------------------------------------------------
void SolutionSetup::setLengthZ(float64 length)
{
    z_length_ = length;
}

//----------------------------------------------------------------------------
float64 SolutionSetup::getLengthZ() const
{
    return z_length_;
}

//----------------------------------------------------------------------------
Transient* SolutionSetup::getTransientNode()
{
    for (auto child = firstChildNode(); child != lastChildNode(); ++child) {
        Transient* transient = child->get<Transient*>();
        if (transient) return transient;
    }
    return nullptr;
}

//----------------------------------------------------------------------------
bool SolutionSetup::update()
{
    return true;
}

//----------------------------------------------------------------------------
void SolutionSetup::onAttachTo(Node* parent)
{

}

//----------------------------------------------------------------------------
void SolutionSetup::onDetachFrom(Node* parent)
{

}

//----------------------------------------------------------------------------
void SolutionSetup::bindProperty()
{
    BIND_PROPERTY(float64, Tolerance_Absolute, &setAbsoluteTolerance, &getAbsoluteTolerance);
    BIND_PROPERTY(float64, Tolerance_Relative, &setRelativeTolerance, &getRelativeTolerance);
    BIND_PROPERTY(int32, Max_Iteration, &setMaxIteration, &getMaxIteration);
    BIND_PROPERTY(float64, z_Length, &setLengthZ, &getLengthZ);
    BIND_PROPERTY(float64, Symmetry_Factor, &setSymmetrcyFactor, &getSymmetrcyFactor);
}


