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

    (renderBufferDescriptor));

static void
_CreateGrid(int nx, int ny, VtVec3fArray *points,
            VtIntArray *numVerts, VtIntArray *verts)
{
    // create a unit plane (-1 ~ 1)
    for (int y = 0; y <= ny; ++y)
    {
        for (int x = 0; x <= nx; ++x)
        {
            GfVec3f p(2.0 * x / float(nx) - 1.0,
                      2.0 * y / float(ny) - 1.0,
                      0);
            points->push_back(p);
        }
    }
    for (int y = 0; y < ny; ++y)
    {
        for (int x = 0; x < nx; ++x)
        {
            numVerts->push_back(4);
            verts->push_back(y * (nx + 1) + x);
            verts->push_back(y * (nx + 1) + x + 1);
            verts->push_back((y + 1) * (nx + 1) + x + 1);
            verts->push_back((y + 1) * (nx + 1) + x);
        }
    }
}

namespace
{
    class ShadowMatrix : public HdxShadowMatrixComputation
    {
    public:
        ShadowMatrix(GlfSimpleLight const &light)
        {
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
            const GfVec4f &viewport, CameraUtilConformWindowPolicy policy)
        {
            return std::vector<GfMatrix4d>(1, _shadowMatrix);
        }

    private:
        GfMatrix4d _shadowMatrix;
    };

    class DrawTargetTextureResource : public HdTextureResource
    {
    public:
        DrawTargetTextureResource(GlfDrawTargetRefPtr const &drawTarget)
            : _drawTarget(drawTarget)
        {
        }
        virtual ~DrawTargetTextureResource(){};

        virtual HdTextureType GetTextureType() const override
        {
            return HdTextureType::Uv;
        }

        virtual GLuint GetTexelsTextureId()
        {
            return _drawTarget->GetAttachment("color")->GetGlTextureName();
        }
        virtual GLuint GetTexelsSamplerId()
        {
            return 0;
        }
        virtual uint64_t GetTexelsTextureHandle()
        {
            return 0;
        }

        virtual GLuint GetLayoutTextureId()
        {
            return 0;
        }
        virtual uint64_t GetLayoutTextureHandle()
        {
            return 0;
        }

        size_t GetMemoryUsed() override
        {
            return 0;
        }

    private:
        GlfDrawTargetRefPtr _drawTarget;
    };

} // namespace

// ------------------------------------------------------------------------
Hdx_UnitTestDelegate::Hdx_UnitTestDelegate(HdRenderIndex *index)
    : HdSceneDelegate(index, SdfPath::AbsoluteRootPath())
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

    // HdRenderIndex &index = GetRenderIndex();
    index->InsertRprim(HdPrimTypeTokens->mesh, this, SdfPath("/cube0"));
}

void Hdx_UnitTestDelegate::SetCamera(SdfPath const &cameraId,
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

void Hdx_UnitTestDelegate::AddRenderTask(SdfPath const &id)
{
    GetRenderIndex().InsertTask<HdxRenderTask>(this, id);
    _ValueCache &cache = _valueCacheMap[id];
    cache[HdTokens->collection] = HdRprimCollection(HdTokens->geometry,
                                                    HdReprSelector(HdReprTokens->smoothHull));

    // Don't filter on render tag.
    // XXX: However, this will mean no prim passes if any stage defines a tag
    cache[HdTokens->renderTags] = TfTokenVector();
}

void Hdx_UnitTestDelegate::AddRenderSetupTask(SdfPath const &id)
{
    GetRenderIndex().InsertTask<HdxRenderSetupTask>(this, id);
    _ValueCache &cache = _valueCacheMap[id];
    HdxRenderTaskParams params;
    params.camera = _cameraId;
    params.viewport = GfVec4f(0, 0, 512, 512);
    cache[HdTokens->params] = VtValue(params);
}

GfRange3d
Hdx_UnitTestDelegate::GetExtent(SdfPath const &id)
{
    GfRange3d range;
    VtVec3fArray points;
    TF_FOR_ALL(it, points)
    {
        range.UnionWith(*it);
    }
    return range;
}

GfMatrix4d
Hdx_UnitTestDelegate::GetTransform(SdfPath const &id)
{
    return GfMatrix4d(1);
}

bool Hdx_UnitTestDelegate::GetVisible(SdfPath const &id)
{
    return true;
}

HdMeshTopology
Hdx_UnitTestDelegate::GetMeshTopology(SdfPath const &id)
{
    pxr::VtArray<int> vertCountsPerFace;
    vertCountsPerFace.push_back(3);
    pxr::VtArray<int> verts;
    verts.push_back(0);
    verts.push_back(1);
    verts.push_back(2);
    return pxr::HdMeshTopology(pxr::PxOsdOpenSubdivTokens->none, pxr::HdTokens->rightHanded, vertCountsPerFace, verts);
}

VtValue
Hdx_UnitTestDelegate::Get(SdfPath const &id, TfToken const &key)
{
    // tasks
    _ValueCache *vcache = TfMapLookupPtr(_valueCacheMap, id);
    VtValue ret;
    if (vcache && TfMapLookup(*vcache, key, &ret))
    {
        return ret;
    }

    // prims
    if (key == HdTokens->points)
    {
        pxr::VtVec3fArray points;
        points.push_back(pxr::GfVec3f(0, 0, 0));
        points.push_back(pxr::GfVec3f(1, 0, 0));
        points.push_back(pxr::GfVec3f(0, 0, 1));
        return pxr::VtValue(points);
    }

    return VtValue();
}

HdPrimvarDescriptorVector
Hdx_UnitTestDelegate::GetPrimvarDescriptors(SdfPath const &id,
                                            HdInterpolation interpolation)
{
    HdPrimvarDescriptorVector primvars;
    if (interpolation == HdInterpolationVertex)
    {
        primvars.emplace_back(HdTokens->points, interpolation,
                              HdPrimvarRoleTokens->point);
    }
    return primvars;
}

/*virtual*/
VtValue
Hdx_UnitTestDelegate::GetCameraParamValue(SdfPath const &cameraId,
                                          TfToken const &paramName)
{
    _ValueCache *vcache = TfMapLookupPtr(_valueCacheMap, cameraId);
    VtValue ret;
    if (vcache && TfMapLookup(*vcache, paramName, &ret))
    {
        return ret;
    }

    return VtValue();
}

PXR_NAMESPACE_CLOSE_SCOPE
