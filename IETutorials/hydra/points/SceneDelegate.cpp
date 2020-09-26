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

	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->points, this, pxr::SdfPath("/points") );

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
				for (size_t k = 0; k < arraySize; ++k)
				{
					points.push_back(pxr::GfVec3f(i * cellSize, j * cellSize , k * cellSize) - pxr::GfVec3f(0.5f * size, 0.5f *size, 0.5f *size));
				}
			}
		}


		return pxr::VtValue(points);
	}

	if (key == pxr::HdTokens->widths)
	{
		pxr::VtFloatArray widths;

		for (size_t i = 0; i < arraySize; ++i)
		{
			for (size_t j = 0; j < arraySize; ++j)
			{
				for (size_t k = 0; k < arraySize; ++k)
				{
					float a = 0.3f * sin((float) j * 0.2f + rotation * 0.01f);
					a *= a;

					widths.push_back(0.02f + a  );
				}
			}
		}

		return pxr::VtValue(widths);
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
	if (id == pxr::SdfPath("/points") )
	{
		pxr::GfMatrix4d m ;
		m.SetTransform(pxr::GfRotation(pxr::GfVec3d(0,1,0), 0.05f * rotation) , pxr::GfVec3d(0,0,0));
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
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->widths, interpolation));
	}


	return primvarDescriptors;
}

void SceneDelegate::UpdateTransform()
{
	rotation += 1.0f;

	GetRenderIndex().GetChangeTracker().MarkRprimDirty(pxr::SdfPath("/points"), pxr::HdChangeTracker::DirtyTransform | pxr::HdChangeTracker::DirtyWidths);
}