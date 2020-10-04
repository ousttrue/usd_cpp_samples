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
#ifndef PXR_IMAGING_HDX_UNIT_TEST_DELEGATE_H
#define PXR_IMAGING_HDX_UNIT_TEST_DELEGATE_H

#include "pxr/pxr.h"
#include "pxr/imaging/hd/sceneDelegate.h"
#include "pxr/imaging/hd/tokens.h"
#include "pxr/imaging/glf/simpleLight.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/base/gf/vec3f.h"
#include "pxr/base/gf/vec3d.h"
#include "pxr/base/gf/vec4f.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/gf/matrix4f.h"
#include "pxr/base/gf/matrix4d.h"
#include "pxr/base/vt/array.h"
#include "pxr/base/tf/staticTokens.h"

PXR_NAMESPACE_OPEN_SCOPE

template <typename T>
static VtArray<T>
_BuildArray(T values[], int numValues)
{
    VtArray<T> result(numValues);
    std::copy(values, values+numValues, result.begin());
    return result;
}

class Hdx_UnitTestDelegate : public HdSceneDelegate
{
public:
    Hdx_UnitTestDelegate(HdRenderIndex *renderIndex);

    // camera
    void SetCamera(GfMatrix4d const &viewMatrix, GfMatrix4d const &projMatrix);

private:
    void SetCamera(
        SdfPath const &id, 
        GfMatrix4d const &viewMatrix, 
        GfMatrix4d const &projMatrix);

public:
    // tasks
    void AddRenderTask(SdfPath const &id);
    void AddRenderSetupTask(SdfPath const &id);

public:
    void SetTaskParam(SdfPath const &id, TfToken const &name, VtValue val);
    VtValue GetTaskParam(SdfPath const &id, TfToken const &name);

private:

    // prims       
    void AddMesh(SdfPath const &id,
                 GfMatrix4d const &transform,
                 VtVec3fArray const &points,
                 VtIntArray const &numVerts,
                 VtIntArray const &verts,
                 PxOsdSubdivTags const &subdivTags,
                 VtValue const &color,
                 HdInterpolation colorInterpolation,
                 VtValue const &opacity,
                 HdInterpolation opacityInterpolation,
                 bool guide=false,
                 SdfPath const &instancerId=SdfPath(),
                 TfToken const &scheme=PxOsdOpenSubdivTokens->catmullClark,
                 TfToken const &orientation=HdTokens->rightHanded,
                 bool doubleSided=false);

public:
    void AddCube(SdfPath const &id, GfMatrix4d const &transform, 
                 bool guide=false,
                 SdfPath const &instancerId=SdfPath(),
                 TfToken const &scheme=PxOsdOpenSubdivTokens->catmullClark,
                 VtValue const &color = VtValue(GfVec3f(1,1,1)),
                 HdInterpolation colorInterpolation = HdInterpolationConstant,
                 VtValue const &opacity = VtValue(1.0f),
                 HdInterpolation opacityInterpolation = HdInterpolationConstant);

public:
    // delegate methods
    GfRange3d GetExtent(SdfPath const & id) override;
    GfMatrix4d GetTransform(SdfPath const & id) override;
    bool GetVisible(SdfPath const& id) override;
    HdMeshTopology GetMeshTopology(SdfPath const& id) override;
    VtValue Get(SdfPath const& id, TfToken const& key) override;
    HdPrimvarDescriptorVector GetPrimvarDescriptors(
        SdfPath const& id, 
        HdInterpolation interpolation) override;

    VtValue GetCameraParamValue(
        SdfPath const &cameraId,
        TfToken const &paramName) override;

    TfTokenVector GetTaskRenderTags(SdfPath const& taskId) override;

private:
    struct _Mesh {
        _Mesh() { }
        _Mesh(TfToken const &scheme,
              TfToken const &orientation,
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
              bool doubleSided) :
            scheme(scheme), orientation(orientation),
            transform(transform),
            points(points), numVerts(numVerts), verts(verts),
            subdivTags(subdivTags), color(color),
            colorInterpolation(colorInterpolation), opacity(opacity),
            opacityInterpolation(opacityInterpolation), guide(guide),
            doubleSided(doubleSided) { }

        TfToken scheme;
        TfToken orientation;
        GfMatrix4d transform;
        VtVec3fArray points;
        VtIntArray numVerts;
        VtIntArray verts;
        PxOsdSubdivTags subdivTags;
        VtValue color;
        HdInterpolation colorInterpolation;
        VtValue opacity;
        HdInterpolation opacityInterpolation;
        bool guide;
        bool doubleSided;
        TfToken reprName;
    };
    
    std::map<SdfPath, _Mesh> _meshes;

    typedef std::map<SdfPath, SdfPath> SdfPathMap;

    typedef TfHashMap<TfToken, VtValue, TfToken::HashFunctor> _ValueCache;
    typedef TfHashMap<SdfPath, _ValueCache, SdfPath::Hash> _ValueCacheMap;
    _ValueCacheMap _valueCacheMap;

    SdfPath _cameraId;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif  // PXR_IMAGING_HDX_UNIT_TEST_DELEGATE_H
