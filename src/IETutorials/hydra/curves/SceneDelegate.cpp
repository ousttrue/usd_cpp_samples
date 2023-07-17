#include "SceneDelegate.h"

#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/imaging/hdx/renderTask.h"

#include "pxr/base/gf/range3f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/vt/array.h"


SceneDelegate::SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const &delegateID)
 : pxr::HdSceneDelegate(parentIndex, delegateID)
{
	cameraPath = pxr::SdfPath("/camera");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->camera, this, cameraPath);
	pxr::GfFrustum frustum;
	frustum.SetPosition(pxr::GfVec3d(0, 0, 3));
	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->basisCurves, this, pxr::SdfPath("/curves") );
}

void
SceneDelegate::AddRenderTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->collection]
			= pxr::HdRprimCollection(pxr::HdTokens->geometry, pxr::HdTokens->smoothHull);
}

void
SceneDelegate::AddRenderSetupTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderSetupTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	pxr::HdxRenderTaskParams params;
	params.camera = cameraPath;
	params.viewport = pxr::GfVec4f(0, 0, 512, 512);
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->params] = pxr::VtValue(params);
}

void SceneDelegate::SetCamera(pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix)
{
	SetCamera(cameraPath, viewMatrix, projMatrix);
}

void SceneDelegate::SetCamera(pxr::SdfPath const &cameraId, pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix)
{
	_ValueCache &cache = _valueCacheMap[cameraId];
	cache[pxr::HdCameraTokens->windowPolicy] = pxr::VtValue(pxr::CameraUtilFit);
	cache[pxr::HdCameraTokens->worldToViewMatrix] = pxr::VtValue(viewMatrix);
	cache[pxr::HdCameraTokens->projectionMatrix] = pxr::VtValue(projMatrix);

	GetRenderIndex().GetChangeTracker().MarkSprimDirty(cameraId, pxr::HdCamera::AllDirty);
}


pxr::VtValue SceneDelegate::Get(pxr::SdfPath const &id, const pxr::TfToken &key)
{
	std::cout << "[" << id.GetString() <<"][" << key << "]" << std::endl;
	_ValueCache *vcache = pxr::TfMapLookupPtr(_valueCacheMap, id);
	pxr::VtValue ret;
	if (vcache && pxr::TfMapLookup(*vcache, key, &ret)) {
		return ret;
	}

	if (key == pxr::HdShaderTokens->material)
	{
		return pxr::VtValue();
	}

	const int arraySize = 20;
	const float size = 2.0;

	const float cellSize = size / arraySize;

	if (key == pxr::HdTokens->points)
	{
		pxr::VtVec3fArray points;

		for (size_t i = 0; i < arraySize; ++i)
		{
			for (size_t j = 0; j < arraySize; ++j)
			{
				for (size_t k = 0; k < 4; ++k)
				{
					points.push_back(pxr::GfVec3f(i * cellSize + sin(rotation * 0.01f + 0.1f * (i+j+k)) * 0.01f * k, k * 0.25 , j * cellSize + cos(rotation * 0.01 + 0.1f *(i+j+k)) * 0.1f * k) - pxr::GfVec3f(0.5f * size, 0.5f *size, 0.5f *size));
				}
			}
		}


		return pxr::VtValue(points);
	}

	return pxr::VtValue();
}

bool SceneDelegate::GetVisible(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Visible]" << std::endl;
	return true;
}

pxr::GfRange3d SceneDelegate::GetExtent(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Extent]" << std::endl;
	return pxr::GfRange3d(pxr::GfVec3d(-1,-1,-1), pxr::GfVec3d(1,1,1));
}

pxr::GfMatrix4d SceneDelegate::GetTransform(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Transform]" << std::endl;
	if (id == pxr::SdfPath("/curves") )
	{
		pxr::GfMatrix4d m ;
		m.SetTransform(pxr::GfRotation(pxr::GfVec3d(0,1,0), 0.005f * rotation) , pxr::GfVec3d(0,0,0));
		return m;
	}
}

pxr::HdPrimvarDescriptorVector SceneDelegate::GetPrimvarDescriptors(pxr::SdfPath const& id, pxr::HdInterpolation interpolation)
{
	std::cout << "[" << id.GetString() <<"][GetPrimvarDescriptors]" << std::endl;
	pxr::HdPrimvarDescriptorVector primvarDescriptors;

	if (interpolation == pxr::HdInterpolation::HdInterpolationVertex)
	{
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->points, interpolation));

	}

	return primvarDescriptors;
}


void SceneDelegate::UpdateTransform()
{
	rotation += 1.0f;

	GetRenderIndex().GetChangeTracker().MarkRprimDirty(pxr::SdfPath("/curves"), pxr::HdChangeTracker::DirtyTransform | pxr::HdChangeTracker::DirtyPoints);

}

pxr::HdBasisCurvesTopology SceneDelegate::GetBasisCurvesTopology(pxr::SdfPath const& id)
{
	std::cout << "[" << id.GetString() <<"][BasisCurvesTopology]" << std::endl;

	const int arraySize = 20;


	pxr::VtIntArray curveVertexCounts;
	pxr::VtIntArray curveIndices;
	int index = 0;

	for (size_t i = 0; i < arraySize; ++i)
	{
		for (size_t j = 0; j < arraySize; ++j)
		{
			curveVertexCounts.push_back(4);
			for (size_t k = 0; k < 4; ++k)
			{
				curveIndices.push_back(index++);
			}
		}
	}

	return pxr::HdBasisCurvesTopology(pxr::HdTokens->cubic, pxr::HdTokens->bezier, pxr::HdTokens->nonperiodic, curveVertexCounts, curveIndices );
}