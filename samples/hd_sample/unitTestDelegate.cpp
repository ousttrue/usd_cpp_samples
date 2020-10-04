//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#include "unitTestDelegate.h"

#include "pxr/base/gf/frustum.h"

#include "pxr/imaging/hd/engine.h"
#include "pxr/imaging/hd/mesh.h"
#include "pxr/imaging/hd/sprim.h"
#include "pxr/imaging/hd/texture.h"
#include "pxr/imaging/hd/textureResource.h"

#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/hdSt/drawTarget.h"
#include "pxr/imaging/hdSt/drawTargetAttachmentDescArray.h"
#include "pxr/imaging/hdSt/light.h"

#include "pxr/imaging/hdx/drawTargetTask.h"
#include "pxr/imaging/hdx/drawTargetResolveTask.h"
#include "pxr/imaging/hdx/pickTask.h"
#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/hdx/selectionTask.h"
#include "pxr/imaging/hdx/simpleLightTask.h"
#include "pxr/imaging/hdx/shadowTask.h"
#include "pxr/imaging/hdx/shadowMatrixComputation.h"

#include "pxr/imaging/glf/drawTarget.h"
#include "pxr/imaging/pxOsd/tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PRIVATE_TOKENS(
    _tokens,

    (renderBufferDescriptor)
);    

static void
_CreateGrid(int nx, int ny, VtVec3fArray *points,
            VtIntArray *numVerts, VtIntArray *verts)
{
    // create a unit plane (-1 ~ 1)
    for (int y = 0; y <= ny; ++y) {
        for (int x = 0; x <= nx; ++x) {
            GfVec3f p(2.0*x/float(nx) - 1.0,
                      2.0*y/float(ny) - 1.0,
                      0);
            points->push_back(p);
        }
    }
    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            numVerts->push_back(4);
            verts->push_back(    y*(nx+1) + x    );
            verts->push_back(    y*(nx+1) + x + 1);
            verts->push_back((y+1)*(nx+1) + x + 1);
            verts->push_back((y+1)*(nx+1) + x    );
        }
    }
}

namespace {
class ShadowMatrix : public HdxShadowMatrixComputation
{
public:
    ShadowMatrix(GlfSimpleLight const &light) {
        GfFrustum frustum;
        frustum.SetProjectionType(GfFrustum::Orthographic);
        frustum.SetWindow(GfRange2d(GfVec2d(-10, -10), GfVec2d(10, 10)));
        frustum.SetNearFar(GfRange1d(0, 100));
        GfVec4d pos = light.GetPosition();
        frustum.SetPosition(GfVec3d(0, 0, 10));
        frustum.SetRotation(GfRotation(GfVec3d(0, 0, 1),
                                       GfVec3d(pos[0], pos[1], pos[2])));

        _shadowMatrix =
            frustum.ComputeViewMatrix() * frustum.ComputeProjectionMatrix();
    }

    virtual std::vector<GfMatrix4d> Compute(
        const GfVec4f &viewport, CameraUtilConformWindowPolicy policy) {
        return std::vector<GfMatrix4d>(1, _shadowMatrix);
    }
private:
    GfMatrix4d _shadowMatrix;
};

class DrawTargetTextureResource : public HdTextureResource
{
public:
    DrawTargetTextureResource(GlfDrawTargetRefPtr const &drawTarget)
        : _drawTarget(drawTarget) {
    }
    virtual ~DrawTargetTextureResource() {
    };

    virtual HdTextureType GetTextureType() const override {
        return HdTextureType::Uv;
    }

    virtual GLuint GetTexelsTextureId() {
        return _drawTarget->GetAttachment("color")->GetGlTextureName();
    }
    virtual GLuint GetTexelsSamplerId() {
        return 0;
    }
    virtual uint64_t GetTexelsTextureHandle() {
        return 0;
    }

    virtual GLuint GetLayoutTextureId() {
        return 0;
    }
    virtual uint64_t GetLayoutTextureHandle() {
        return 0;
    }

    size_t GetMemoryUsed() override {
        return 0;
    }

private:
    GlfDrawTargetRefPtr _drawTarget;
};

}

// ------------------------------------------------------------------------
Hdx_UnitTestDelegate::Hdx_UnitTestDelegate(HdRenderIndex *index)
    : HdSceneDelegate(index, SdfPath::AbsoluteRootPath())
    , _refineLevel(0)
{
    // add camera
    _cameraId = SdfPath("/camera");
    GetRenderIndex().InsertSprim(HdPrimTypeTokens->camera, this, _cameraId);
    GfFrustum frustum;
    frustum.SetPosition(GfVec3d(0, 0, 3));
    SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

    // Add draw target state tracking support.
    GetRenderIndex().GetChangeTracker().AddState(
            HdStDrawTargetTokens->drawTargetSet);
}

void
Hdx_UnitTestDelegate::SetRefineLevel(int level)
{
    _refineLevel = level;
    TF_FOR_ALL (it, _meshes) {
        GetRenderIndex().GetChangeTracker().MarkRprimDirty(
            it->first, HdChangeTracker::DirtyDisplayStyle);
    }
    TF_FOR_ALL (it, _refineLevels) {
        it->second = level;
    }
}

void
Hdx_UnitTestDelegate::SetCamera(GfMatrix4d const &viewMatrix,
                                GfMatrix4d const &projMatrix)
{
    SetCamera(_cameraId, viewMatrix, projMatrix);
}

void
Hdx_UnitTestDelegate::SetCamera(SdfPath const &cameraId,
                                GfMatrix4d const &viewMatrix,
                                GfMatrix4d const &projMatrix)
{
    _ValueCache &cache = _valueCacheMap[cameraId];
    cache[HdCameraTokens->windowPolicy] = VtValue(CameraUtilFit);
    cache[HdCameraTokens->worldToViewMatrix] = VtValue(viewMatrix);
    cache[HdCameraTokens->projectionMatrix] = VtValue(projMatrix);

    GetRenderIndex().GetChangeTracker().MarkSprimDirty(cameraId,
                                                       HdCamera::AllDirty);
}


void
Hdx_UnitTestDelegate::AddRenderTask(SdfPath const &id)
{
    GetRenderIndex().InsertTask<HdxRenderTask>(this, id);
    _ValueCache &cache = _valueCacheMap[id];
    cache[HdTokens->collection]
        = HdRprimCollection(HdTokens->geometry, 
            HdReprSelector(HdReprTokens->smoothHull));

    // Don't filter on render tag.
    // XXX: However, this will mean no prim passes if any stage defines a tag
    cache[HdTokens->renderTags] = TfTokenVector();
}

void
Hdx_UnitTestDelegate::AddRenderSetupTask(SdfPath const &id)
{
    GetRenderIndex().InsertTask<HdxRenderSetupTask>(this, id);
    _ValueCache &cache = _valueCacheMap[id];
    HdxRenderTaskParams params;
    params.camera = _cameraId;
    params.viewport = GfVec4f(0, 0, 512, 512);
    cache[HdTokens->params] = VtValue(params);
}

void
Hdx_UnitTestDelegate::SetTaskParam(
    SdfPath const &id, TfToken const &name, VtValue val)
{
    _ValueCache &cache = _valueCacheMap[id];
    cache[name] = val;

    if (name == HdTokens->collection) {
        GetRenderIndex().GetChangeTracker().MarkTaskDirty(
            id, HdChangeTracker::DirtyCollection);
    } else if (name == HdTokens->params) {
        GetRenderIndex().GetChangeTracker().MarkTaskDirty(
            id, HdChangeTracker::DirtyParams);
    }
}

VtValue
Hdx_UnitTestDelegate::GetTaskParam(SdfPath const &id, TfToken const &name)
{
    return _valueCacheMap[id][name];
}

//------------------------------------------------------------------------------
//                                  PRIMS
//------------------------------------------------------------------------------
void
Hdx_UnitTestDelegate::AddMesh(SdfPath const &id,
                             GfMatrix4d const &transform,
                             VtVec3fArray const &points,
                             VtIntArray const &numVerts,
                             VtIntArray const &verts,
                             bool guide,
                             SdfPath const &instancerId,
                             TfToken const &scheme,
                             TfToken const &orientation,
                             bool doubleSided)
{
    HdRenderIndex& index = GetRenderIndex();
    index.InsertRprim(HdPrimTypeTokens->mesh, this, id, instancerId);

    _meshes[id] = _Mesh(scheme, orientation, transform,
                        points, numVerts, verts, PxOsdSubdivTags(),
                        /*color=*/VtValue(GfVec3f(1, 1, 0)),
                        /*colorInterpolation=*/HdInterpolationConstant,
                        /*opacity=*/VtValue(1.0f),
                        HdInterpolationConstant,
                        guide, doubleSided);
    if (!instancerId.IsEmpty()) {
        _instancers[instancerId].prototypes.push_back(id);
    }
}

void
Hdx_UnitTestDelegate::AddMesh(SdfPath const &id,
                             GfMatrix4d const &transform,
                             VtVec3fArray const &points,
                             VtIntArray const &numVerts,
                             VtIntArray const &verts,
                             PxOsdSubdivTags const &subdivTags,
                             VtValue const &color,
                             HdInterpolation colorInterpolation,
                             VtValue const &opacity,
                             HdInterpolation opacityInterpolation,
                             bool guide,
                             SdfPath const &instancerId,
                             TfToken const &scheme,
                             TfToken const &orientation,
                             bool doubleSided)
{
    HdRenderIndex& index = GetRenderIndex();
    index.InsertRprim(HdPrimTypeTokens->mesh, this, id, instancerId);

    _meshes[id] = _Mesh(scheme, orientation, transform,
                        points, numVerts, verts, subdivTags,
                        color, colorInterpolation, opacity,
                        opacityInterpolation, guide, doubleSided);
    if (!instancerId.IsEmpty()) {
        _instancers[instancerId].prototypes.push_back(id);
    }
}

void
Hdx_UnitTestDelegate::AddCube(SdfPath const &id, GfMatrix4d const &transform, 
                              bool guide, SdfPath const &instancerId, 
                              TfToken const &scheme, VtValue const &color,
                              HdInterpolation colorInterpolation,
                              VtValue const &opacity,
                              HdInterpolation opacityInterpolation)
{
    GfVec3f points[] = {
        GfVec3f( 1.0f, 1.0f, 1.0f ),
        GfVec3f(-1.0f, 1.0f, 1.0f ),
        GfVec3f(-1.0f,-1.0f, 1.0f ),
        GfVec3f( 1.0f,-1.0f, 1.0f ),
        GfVec3f(-1.0f,-1.0f,-1.0f ),
        GfVec3f(-1.0f, 1.0f,-1.0f ),
        GfVec3f( 1.0f, 1.0f,-1.0f ),
        GfVec3f( 1.0f,-1.0f,-1.0f ),
    };

    if (scheme == PxOsdOpenSubdivTokens->loop) {
        int numVerts[] = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3 };
        int verts[] = {
            0, 1, 2, 0, 2, 3,
            4, 5, 6, 4, 6, 7,
            0, 6, 5, 0, 5, 1,
            4, 7, 3, 4, 3, 2,
            0, 3, 7, 0, 7, 6,
            4, 2, 1, 4, 1, 5,
        };
        AddMesh(
            id,
            transform,
            _BuildArray(points, sizeof(points)/sizeof(points[0])),
            _BuildArray(numVerts, sizeof(numVerts)/sizeof(numVerts[0])),
            _BuildArray(verts, sizeof(verts)/sizeof(verts[0])),
            PxOsdSubdivTags(),
            color,
            colorInterpolation,
            opacity,
            opacityInterpolation,
            guide,
            instancerId,
            scheme);
    } else {
        int numVerts[] = { 4, 4, 4, 4, 4, 4 };
        int verts[] = {
            0, 1, 2, 3,
            4, 5, 6, 7,
            0, 6, 5, 1,
            4, 7, 3, 2,
            0, 3, 7, 6,
            4, 2, 1, 5,
        };
        AddMesh(
            id,
            transform,
            _BuildArray(points, sizeof(points)/sizeof(points[0])),
            _BuildArray(numVerts, sizeof(numVerts)/sizeof(numVerts[0])),
            _BuildArray(verts, sizeof(verts)/sizeof(verts[0])),
            PxOsdSubdivTags(),
            color,
            colorInterpolation,
            opacity,
            opacityInterpolation,
            guide,
            instancerId,
            scheme);
    }
}

HdReprSelector
Hdx_UnitTestDelegate::GetReprSelector(SdfPath const &id)
{
    if (_meshes.find(id) != _meshes.end()) {
        return HdReprSelector(_meshes[id].reprName);
    }

    return HdReprSelector();
}


GfRange3d
Hdx_UnitTestDelegate::GetExtent(SdfPath const & id)
{
    GfRange3d range;
    VtVec3fArray points;
    if(_meshes.find(id) != _meshes.end()) {
        points = _meshes[id].points; 
    }
    TF_FOR_ALL(it, points) {
        range.UnionWith(*it);
    }
    return range;
}

GfMatrix4d
Hdx_UnitTestDelegate::GetTransform(SdfPath const & id)
{
    if(_meshes.find(id) != _meshes.end()) {
        return _meshes[id].transform;
    }
    return GfMatrix4d(1);
}

bool
Hdx_UnitTestDelegate::GetVisible(SdfPath const& id)
{
    return true;
}

HdMeshTopology
Hdx_UnitTestDelegate::GetMeshTopology(SdfPath const& id)
{
    HdMeshTopology topology;
    const _Mesh &mesh = _meshes[id];

    return HdMeshTopology(PxOsdOpenSubdivTokens->catmullClark,
                          HdTokens->rightHanded,
                          mesh.numVerts,
                          mesh.verts);
}

VtValue
Hdx_UnitTestDelegate::Get(SdfPath const& id, TfToken const& key)
{
    // tasks
    _ValueCache *vcache = TfMapLookupPtr(_valueCacheMap, id);
    VtValue ret;
    if (vcache && TfMapLookup(*vcache, key, &ret)) {
        return ret;
    }

    // prims
    if (key == HdTokens->points) {
        if(_meshes.find(id) != _meshes.end()) {
            return VtValue(_meshes[id].points);
        }
    } else if (key == HdTokens->displayColor) {
        if(_meshes.find(id) != _meshes.end()) {
            return VtValue(_meshes[id].color);
        }
    } else if (key == HdTokens->displayOpacity) {
        if(_meshes.find(id) != _meshes.end()) {
            return VtValue(_meshes[id].opacity);
        }
    } else if (key == HdInstancerTokens->scale) {
        if (_instancers.find(id) != _instancers.end()) {
            return VtValue(_instancers[id].scale);
        }
    } else if (key == HdInstancerTokens->rotate) {
        if (_instancers.find(id) != _instancers.end()) {
            return VtValue(_instancers[id].rotate);
        }
    } else if (key == HdInstancerTokens->translate) {
        if (_instancers.find(id) != _instancers.end()) {
            return VtValue(_instancers[id].translate);
        }
    }
    return VtValue();
}

/*virtual*/
VtIntArray
Hdx_UnitTestDelegate::GetInstanceIndices(SdfPath const& instancerId,
                                        SdfPath const& prototypeId)
{
    HD_TRACE_FUNCTION();
    VtIntArray indices(0);
    //
    // XXX: this is very naive implementation for unit test.
    //
    //   transpose prototypeIndices/instances to instanceIndices/prototype
    if (_Instancer *instancer = TfMapLookupPtr(_instancers, instancerId)) {
        size_t prototypeIndex = 0;
        for (; prototypeIndex < instancer->prototypes.size(); ++prototypeIndex) {
            if (instancer->prototypes[prototypeIndex] == prototypeId) break;
        }
        if (prototypeIndex == instancer->prototypes.size()) return indices;

        // XXX use const_ptr
        for (size_t i = 0; i < instancer->prototypeIndices.size(); ++i) {
            if (static_cast<size_t>(instancer->prototypeIndices[i]) == prototypeIndex) {
                indices.push_back(i);
            }
        }
    }
    return indices;
}

/*virtual*/
GfMatrix4d
Hdx_UnitTestDelegate::GetInstancerTransform(SdfPath const& instancerId)
{
    HD_TRACE_FUNCTION();
    if (_Instancer *instancer = TfMapLookupPtr(_instancers, instancerId)) {
        return GfMatrix4d(instancer->rootTransform);
    }
    return GfMatrix4d(1);
}

/*virtual*/
HdDisplayStyle
Hdx_UnitTestDelegate::GetDisplayStyle(SdfPath const& id)
{
    if (_refineLevels.find(id) != _refineLevels.end()) {
        return HdDisplayStyle(_refineLevels[id]);
    }
    return HdDisplayStyle(_refineLevel);
}

HdPrimvarDescriptorVector
Hdx_UnitTestDelegate::GetPrimvarDescriptors(SdfPath const& id, 
                                            HdInterpolation interpolation)
{       
    HdPrimvarDescriptorVector primvars;
    if (interpolation == HdInterpolationVertex) {
        primvars.emplace_back(HdTokens->points, interpolation,
                              HdPrimvarRoleTokens->point);
    }                       
    if(_meshes.find(id) != _meshes.end()) {
        if (_meshes[id].colorInterpolation == interpolation) {
            primvars.emplace_back(HdTokens->displayColor, interpolation,
                                  HdPrimvarRoleTokens->color);
        }
        if (_meshes[id].opacityInterpolation == interpolation) {
            primvars.emplace_back(HdTokens->displayOpacity, interpolation);
        }
    }
    if (interpolation == HdInterpolationInstance &&
        _instancers.find(id) != _instancers.end()) {
        primvars.emplace_back(HdInstancerTokens->scale, interpolation);
        primvars.emplace_back(HdInstancerTokens->rotate, interpolation);
        primvars.emplace_back(HdInstancerTokens->translate, interpolation);
    }
    return primvars;
}

void 
Hdx_UnitTestDelegate::AddMaterialResource(SdfPath const &id,
                                         VtValue materialResource)
{
    HdRenderIndex& index = GetRenderIndex();
    index.InsertSprim(HdPrimTypeTokens->material, this, id);
    _materials[id] = materialResource;
}

void
Hdx_UnitTestDelegate::BindMaterial(SdfPath const &rprimId,
                                 SdfPath const &materialId)
{
    _materialBindings[rprimId] = materialId;
}

/*virtual*/ 
SdfPath 
Hdx_UnitTestDelegate::GetMaterialId(SdfPath const &rprimId)
{
    SdfPath materialId;
    TfMapLookup(_materialBindings, rprimId, &materialId);
    return materialId;
}

/*virtual*/
VtValue 
Hdx_UnitTestDelegate::GetMaterialResource(SdfPath const &materialId)
{
    if (VtValue *material = TfMapLookupPtr(_materials, materialId)){
        return *material;
    }
    return VtValue();
}

/*virtual*/
VtValue
Hdx_UnitTestDelegate::GetCameraParamValue(SdfPath const &cameraId,
                                          TfToken const &paramName)
{
    _ValueCache *vcache = TfMapLookupPtr(_valueCacheMap, cameraId);
    VtValue ret;
    if (vcache && TfMapLookup(*vcache, paramName, &ret)) {
        return ret;
    }

    return VtValue();
}

TfTokenVector
Hdx_UnitTestDelegate::GetTaskRenderTags(SdfPath const& taskId)
{
    const auto it1 = _valueCacheMap.find(taskId);
    if (it1 == _valueCacheMap.end()) {
        return {};
    }

    const auto it2 = it1->second.find(HdTokens->renderTags);
    if (it2 == it1->second.end()) {
        return {};
    }

    return it2->second.Get<TfTokenVector>();
}


PXR_NAMESPACE_CLOSE_SCOPE

