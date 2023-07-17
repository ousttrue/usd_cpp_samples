#include "SceneDelegate.h"

#include "pxr/imaging/hd/camera.h"
#include "pxr/imaging/cameraUtil/conformWindow.h"
#include "pxr/imaging/pxOsd/tokens.h"

#include "pxr/imaging/hdx/renderTask.h"

#include "pxr/base/gf/range3f.h"
#include "pxr/base/gf/frustum.h"
#include "pxr/base/vt/array.h"


SceneDelegate::SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const &delegateID)
 : pxr::HdSceneDelegate(parentIndex, delegateID),
  rotation(45.0f)
{
	cameraPath = pxr::SdfPath("/camera");
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->camera, this, cameraPath);
	pxr::GfFrustum frustum;
	frustum.SetPosition(pxr::GfVec3d(0, 0, 10));
	frustum.SetNearFar(pxr::GfRange1d(0.01, 100.0));

	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	pxr::SdfPath instancerPath("/instances");
	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, pxr::SdfPath("/cube"), instancerPath); //,
	GetRenderIndex().InsertInstancer(this, instancerPath);
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
	params.enableLighting = true;
	params.enableHardwareShading = true;
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
	std::cout <<  "[" << id.GetString() <<"][Get][" << key << "]" << std::endl;
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
		else if (key == pxr::HdTokens->points)
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
		else if (key == pxr::HdTokens->normals)
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
	else if (id == pxr::SdfPath("/instances"))
	{

		if (key == pxr::HdTokens->instanceIndices)
		{
			pxr::VtIntArray indices;

			int index = 0;
			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					indices.push_back(index++);
				}
			}

			return pxr::VtValue(indices);
		}
		else if (key == pxr::HdTokens->instanceTransform )
		{
			pxr::VtMatrix4dArray transforms;

			for (int i = 0; i < 10; ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					transforms.push_back(
							pxr::GfMatrix4d(pxr::GfRotation(pxr::GfVec3d(1, 0, 0), 0.0), pxr::GfVec3d(16.0 * (i / 10.0 - 0.5), 16.0 * (j / 10.0 - 0.5), 0)));
				}
			}

			return pxr::VtValue(transforms);
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
		m.SetTransform(pxr::GfRotation(pxr::GfVec3d(0,1,0),  rotation) * pxr::GfRotation(pxr::GfVec3d(1,0,0),  rotation) , pxr::GfVec3d(0,0,0));
		return m;
	}

	pxr::GfMatrix4d();
}

pxr::HdPrimvarDescriptorVector SceneDelegate::GetPrimvarDescriptors(pxr::SdfPath const& id, pxr::HdInterpolation interpolation)
{
	std::cout << "[" << id.GetString() <<"][GetPrimvarDescriptors]" << std::endl;
	pxr::HdPrimvarDescriptorVector primvarDescriptors;

	if (id == pxr::SdfPath("/cube"))
	{
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
//		else if (interpolation == pxr::HdInterpolationInstance)
//		{
//			primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->instanceIndices, interpolation));
//			primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->instanceTransform, interpolation));
//		}
	}
	return primvarDescriptors;
}

//
//pxr::TfTokenVector SceneDelegate::GetPrimVarInstanceNames(pxr::SdfPath const &id)
//{
//	std::cout << "[" << id.GetString() <<"][GetPrimVarInstanceNames]" << std::endl;
//
//	pxr::TfTokenVector names;
//	names.push_back(pxr::HdTokens->instanceIndices);
//	names.push_back(pxr::HdTokens->instanceTransform);
//	return names;
//}

void SceneDelegate::UpdateTransform()
{
	rotation += 1.0f;
}

pxr::VtIntArray SceneDelegate::GetInstanceIndices(pxr::SdfPath const &instancerId, pxr::SdfPath const &prototypeId)
{
	std::cout << "[" << instancerId.GetString() <<"][GetInstanceIndices]:" << prototypeId << std::endl;
	pxr::VtIntArray indices;

	int index = 0;
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			indices.push_back(index++);
		}
	}

	return indices;

}

pxr::GfMatrix4d SceneDelegate::GetInstancerTransform(pxr::SdfPath const &instancerId, pxr::SdfPath const &prototypeId)
{
	std::cout << "[" << instancerId.GetString() <<"][GetInstancerTransform]:" << prototypeId << std::endl;
	return pxr::GfMatrix4d(1.0);
}

pxr::SdfPath SceneDelegate::GetPathForInstanceIndex(const pxr::SdfPath &protoPrimPath, int instanceIndex, int *absoluteInstanceIndex,
									   pxr::SdfPath *rprimPath,
									   pxr::SdfPathVector *instanceContext)
{
	std::cout << "[" << protoPrimPath.GetString() <<"][GetPathForInstanceIndex]:" << instanceIndex << std::endl;
	pxr::VtIntArray tmp;//

	int index = 0;
	for (int i = 0; i < 10; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			absoluteInstanceIndex[index] = index;
			index++;
		}
	}

	return pxr::SdfPath();
}

pxr::HdMeshTopology SceneDelegate::GetMeshTopology(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Topology]" << std::endl;

	if ( id == pxr::SdfPath("/cube") )
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
