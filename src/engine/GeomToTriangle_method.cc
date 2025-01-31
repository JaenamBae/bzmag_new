#include "GeomToTriangle.h"
#include "core/methodbinder.h"

using namespace bzmag;
using namespace bzmag::engine;

//----------------------------------------------------------------------------
static void GeomToTriangle_b_generateGmshStructures_ss(GeomToTriangle* self, Parameter* param)
{
    bool result = self->generateGmshStructures(param->in()->get<String>(0), param->in()->get<String>(1));
    param->out()->get<bool>(0) = result;
}

//----------------------------------------------------------------------------
static void GeomToTriangle_v_writeGeoFile_s(GeomToTriangle* self, Parameter* param)
{
    String filename = param->in()->get<String>(0);
    self->writeGeoFile(filename);
}

//----------------------------------------------------------------------------
static void GeomToTriangle_d_getDomainArea_i(GeomToTriangle* self, Parameter* param)
{
    uint32 ID = param->in()->get<uint32>(0);
    param->out()->get<float64>(0) = self->getDomainArea(ID);
}

//----------------------------------------------------------------------------
static void GeomToTriangle_z_getVertices_v(GeomToTriangle* self, Parameter* param)
{
    param->out()->clear();

    for (auto vv = self->firstVertex(); vv != self->lastVertex(); ++vv)
    {
        GeomToTriangle::Vert v = *vv;
        param->out()->add<float64>(v[0]);
        param->out()->add<float64>(v[1]);
    }
}

//----------------------------------------------------------------------------
static void GeomToTriangle_z_getSegments_v(GeomToTriangle* self, Parameter* param)
{
    param->out()->clear();

    for (auto ss = self->firstSegment(); ss != self->lastSegment(); ++ss)
    {
        GeomToTriangle::Seg s = *ss;
        param->out()->add<int32>(s[0]);
        param->out()->add<int32>(s[1]);
    }
}

//----------------------------------------------------------------------------
static void GeomToTriangle_z_getSegmentMarkers_v(GeomToTriangle* self, Parameter* param)
{
    param->out()->clear();

    for (auto ss = self->firstSegmentMarker(); ss != self->lastSegmentMarker(); ++ss)
    {
        int32 s = (int32)*ss;
        param->out()->add<int32>(s);
    }
}

//----------------------------------------------------------------------------
static void GeomToTriangle_z_getRegions_v(GeomToTriangle* self, Parameter* param)
{
    param->out()->clear();

    for (auto vv = self->firstRegion(); vv != self->lastRegion(); ++vv)
    {
        GeomToTriangle::Region reg = vv->second;
        int32 ID = reg.attribute;
        float64 area = reg.max_area;
        const GeomToTriangle::Vert& v = reg.point;
        param->out()->add<float64>(v[0]);
        param->out()->add<float64>(v[1]);
        param->out()->add<int32>(ID);
        param->out()->add<float64>(area);
    }
}

//----------------------------------------------------------------------------
static void GeomToTriangle_z_getHoles_v(GeomToTriangle* self, Parameter* param)
{
    param->out()->clear();

    for (auto pp = self->firstHole(); pp != self->lastHole(); ++pp)
    {
        param->out()->add<float64>((*pp)[0]);
        param->out()->add<float64>((*pp)[1]);
    }
}

//----------------------------------------------------------------------------
void GeomToTriangle::bindMethod()
{
    BIND_METHOD(b_generateGmshStructures_ss, GeomToTriangle_b_generateGmshStructures_ss);
    BIND_METHOD(v_writeGeoFile_s, GeomToTriangle_v_writeGeoFile_s);
    BIND_METHOD(d_getDomainArea_i, GeomToTriangle_d_getDomainArea_i);
    BIND_METHOD(z_getVertices_v, GeomToTriangle_z_getVertices_v);
    BIND_METHOD(z_getSegments_v, GeomToTriangle_z_getSegments_v);
    BIND_METHOD(z_getSegmentMarkers_v, GeomToTriangle_z_getSegmentMarkers_v);
    BIND_METHOD(z_getRegions_v, GeomToTriangle_z_getRegions_v);
    BIND_METHOD(z_getHoles_v, GeomToTriangle_z_getHoles_v);
}
