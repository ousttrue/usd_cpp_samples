#include "SceneDelegate.h"

#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/hdSt/light.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/imaging/hdx/renderTask.h"
#include "pxr/imaging/hdx/simpleLightTask.h"


#include "pxr/base/gf/range3f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/imaging/glf/simpleLight.h"

#include "pxr/base/vt/array.h"


SceneDelegate::SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const &delegateID)
 : pxr::HdSceneDelegate(parentIndex, delegateID),
		rotation(0.0)
{
	cameraPath = pxr::SdfPath("/camera");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->camera, this, cameraPath);
	pxr::GfFrustum frustum;
	frustum.SetPosition(pxr::GfVec3d(0, 2, 5));
	frustum.SetNearFar(pxr::GfRange1d(0.01, 100.0));
	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, pxr::SdfPath("/cube") );
	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, pxr::SdfPath("/plane") );

	// prep lights
	pxr::GlfSimpleLight light1;

	light1.SetDiffuse(pxr::GfVec4f(1.0, 1.0, 1.0, 1.0));
	light1.SetSpotFalloff(0.0f);
	light1.SetAttenuation(pxr::GfVec3f(1,0,0));
	light1.SetSpotCutoff(90.0);
	light1.SetPosition(pxr::GfVec4f(0, 2, 0, 0));
	light1.SetSpotDirection(pxr::GfVec3f(0,-1,0));
	light1.SetSpecular(pxr::GfVec4f(1,1,1,1));
	light1.SetAmbient(pxr::GfVec4f(0.0,0.0,0.0,1));
	light1.SetHasShadow(false);
	light1.SetIsCameraSpaceLight(false);

	pxr::GfMatrix4d transform(pxr::GfRotation(pxr::GfVec3d(0, 1, 0), 0), pxr::GfVec3d(0, 0, 0));
	light1.SetTransform(transform);

	auto path = pxr::SdfPath("/light1");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->simpleLight, this, path);
	_ValueCache &cache = _valueCacheMap[path];

	pxr::HdxShadowParams shadowParams;
	shadowParams.enabled = false;
	shadowParams.resolution = 512;
	shadowParams.bias = -0.001;
	shadowParams.blur = 0.1;

	cache[pxr::HdLightTokens->params] = light1;
	cache[pxr::HdLightTokens->shadowParams] = shadowParams;
	cache[pxr::HdLightTokens->shadowCollection] = pxr::HdRprimCollection(pxr::HdTokens->geometry, pxr::HdTokens->refined);
}

void SceneDelegate::AddRenderTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->collection] = pxr::HdRprimCollection(pxr::HdTokens->geometry, pxr::HdTokens->smoothHull);
}

void SceneDelegate::AddRenderSetupTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxRenderSetupTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	pxr::HdxRenderTaskParams params;
	params.enableLighting = true;
	params.camera = cameraPath;
	params.viewport = pxr::GfVec4f(0, 0, 512, 512);
	cache[pxr::HdTokens->children] = pxr::VtValue(pxr::SdfPathVector());
	cache[pxr::HdTokens->params] = pxr::VtValue(params);
}

void SceneDelegate::AddSimpleLightTask(pxr::SdfPath const &id)
{
	GetRenderIndex().InsertTask<pxr::HdxSimpleLightTask>(this, id);
	_ValueCache &cache = _valueCacheMap[id];
	pxr::HdxSimpleLightTaskParams params;
	params.cameraPath = cameraPath;
	params.viewport = pxr::GfVec4f(0,0,512,512);

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
	std::cout << "Get-" << "[" << id.GetString() <<"][" << key << "]" << std::endl;
	_ValueCache *vcache = pxr::TfMapLookupPtr(_valueCacheMap, id);
	pxr::VtValue ret;
	if (vcache && pxr::TfMapLookup(*vcache, key, &ret)) {
		return ret;
	}

	if (key == pxr::HdShaderTokens->material)
	{
		return pxr::VtValue();
	}

	if (id == pxr::SdfPath("/cube"))
	{
		if (key == pxr::HdTokens->color)
		{
			return pxr::VtValue(pxr::GfVec4f(0.3, 0.2, 0.2, 1.0));
		}
		if (key == pxr::HdTokens->points)
		{
			pxr::VtVec3fArray points;

			points.push_back(pxr::GfVec3f(0,0,0));
			points.push_back(pxr::GfVec3f(1,0,0));
			points.push_back(pxr::GfVec3f(1,1,0));
			points.push_back(pxr::GfVec3f(0,1,0));

			points.push_back(pxr::GfVec3f(0,0,1));
			points.push_back(pxr::GfVec3f(1,0,1));
			points.push_back(pxr::GfVec3f(1,1,1));
			points.push_back(pxr::GfVec3f(0,1,1));

			for (size_t i = 0; i < points.size(); ++i)
			{
				points[i] -= pxr::GfVec3f(0.5f, 0.5f, 0.5f);
			}
			return pxr::VtValue(points);
		}

		if (key == pxr::HdTokens->normals)
		{
			pxr::VtVec3fArray normals;

			normals.push_back(pxr::GfVec3f(0,0,-1));
			normals.push_back(pxr::GfVec3f(0,0,-1));
			normals.push_back(pxr::GfVec3f(0,0,-1));
			normals.push_back(pxr::GfVec3f(0,0,-1));

			normals.push_back(pxr::GfVec3f(0,0,1));
			normals.push_back(pxr::GfVec3f(0,0,1));
			normals.push_back(pxr::GfVec3f(0,0,1));
			normals.push_back(pxr::GfVec3f(0,0,1));

			normals.push_back(pxr::GfVec3f(1,0,0));
			normals.push_back(pxr::GfVec3f(1,0,0));
			normals.push_back(pxr::GfVec3f(1,0,0));
			normals.push_back(pxr::GfVec3f(1,0,0));

			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));

			normals.push_back(pxr::GfVec3f(-1,0,0));
			normals.push_back(pxr::GfVec3f(-1,0,0));
			normals.push_back(pxr::GfVec3f(-1,0,0));
			normals.push_back(pxr::GfVec3f(-1,0,0));

			normals.push_back(pxr::GfVec3f(0,-1,0));
			normals.push_back(pxr::GfVec3f(0,-1,0));
			normals.push_back(pxr::GfVec3f(0,-1,0));
			normals.push_back(pxr::GfVec3f(0,-1,0));


			return pxr::VtValue(normals);
		}
	}
	else if (id == pxr::SdfPath("/plane"))
	{
		if (key == pxr::HdTokens->color)
		{
			return pxr::VtValue(pxr::GfVec4f(0.2, 0.3, 0.2, 1.0));
		}
		if (key == pxr::HdTokens->points)
		{
			pxr::VtVec3fArray points;

			points.push_back(pxr::GfVec3f(-10,0,-10));
			points.push_back(pxr::GfVec3f(-10,0,10));
			points.push_back(pxr::GfVec3f(10,0,10));
			points.push_back(pxr::GfVec3f(10,0,-10));
			return pxr::VtValue(points);
		}


		if (key == pxr::HdTokens->normals)
		{
			pxr::VtVec3fArray normals;


			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));
			normals.push_back(pxr::GfVec3f(0,1,0));


			return pxr::VtValue(normals);
		}
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

	if (id == pxr::SdfPath("/cube") )
	{
		pxr::GfMatrix4d m ;
		m.SetTransform(pxr::GfRotation(pxr::GfVec3d(0,1,0),rotation) * pxr::GfRotation(pxr::GfVec3d(1,0,0), 1.1 *rotation), pxr::GfVec3d(0,2,2));
		return m;
	}

	return pxr::GfMatrix4d(1.0f);
}

pxr::HdMeshTopology SceneDelegate::GetMeshTopology(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Topology]" << std::endl;

	if ( id == pxr::SdfPath("/plane"))
	{
		pxr::VtArray<int> vertCountsPerFace;
		pxr::VtArray<int> verts;
		vertCountsPerFace.push_back(4);
		verts.push_back(0);
		verts.push_back(1);
		verts.push_back(2);
		verts.push_back(3);

		pxr::HdMeshTopology triangleTopology(pxr::PxOsdOpenSubdivTokens->none, pxr::HdTokens->rightHanded,
											 vertCountsPerFace, verts);
		return triangleTopology;
	}
	else if ( id == pxr::SdfPath("/cube") )
	{
		pxr::VtArray<int> vertCountsPerFace;
		pxr::VtArray<int> verts;
		vertCountsPerFace.push_back(4);
		vertCountsPerFace.push_back(4);
		vertCountsPerFace.push_back(4);
		vertCountsPerFace.push_back(4);
		vertCountsPerFace.push_back(4);
		vertCountsPerFace.push_back(4);

		verts.push_back(0);
		verts.push_back(3);
		verts.push_back(2);
		verts.push_back(1);

		verts.push_back(4);
		verts.push_back(5);
		verts.push_back(6);
		verts.push_back(7);

		verts.push_back(1);
		verts.push_back(2);
		verts.push_back(6);
		verts.push_back(5);

		verts.push_back(2);
		verts.push_back(3);
		verts.push_back(7);
		verts.push_back(6);

		verts.push_back(3);
		verts.push_back(0);
		verts.push_back(4);
		verts.push_back(7);

		verts.push_back(0);
		verts.push_back(1);
		verts.push_back(5);
		verts.push_back(4);

		pxr::HdMeshTopology topology(pxr::PxOsdOpenSubdivTokens->none, pxr::HdTokens->rightHanded,
											 vertCountsPerFace, verts);
		return topology;
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
	else if (interpolation == pxr::HdInterpolationFaceVarying)
	{
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->normals, interpolation));
	}
	else if (interpolation == pxr::HdInterpolationConstant)
	{
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->color, interpolation));
	}

	return primvarDescriptors;
}

void SceneDelegate::UpdateCubeTransform()
{
	rotation += 0.1f;

	GetRenderIndex().GetChangeTracker().MarkRprimDirty(pxr::SdfPath("/cube"), pxr::HdChangeTracker::DirtyTransform);
}