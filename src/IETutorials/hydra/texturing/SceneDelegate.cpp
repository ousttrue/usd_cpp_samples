#include "SceneDelegate.h"

#include <fstream>

#include "pxr/imaging/glf/textureRegistry.h"
#include "pxr/imaging/hd/resourceRegistry.h"

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
	frustum.SetPosition(pxr::GfVec3d(0, 0, 2));
	SetCamera(frustum.ComputeViewMatrix(), frustum.ComputeProjectionMatrix());

	// single quad
	GetRenderIndex().InsertRprim(pxr::HdPrimTypeTokens->mesh, this, pxr::SdfPath("/quad") );
	// add a shader
	GetRenderIndex().InsertSprim(pxr::HdPrimTypeTokens->material, this, pxr::SdfPath("/shader") );
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
	params.enableLighting = true;
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

	if (id == pxr::SdfPath("/quad") && key == pxr::HdShaderTokens->material)
	{
		return pxr::VtValue(pxr::SdfPath("/shader"));
	}

	if (key == pxr::HdTokens->points)
	{
		pxr::VtVec3fArray points;

		points.push_back(pxr::GfVec3f(-1,-1,0));
		points.push_back(pxr::GfVec3f(1,-1,0));
		points.push_back(pxr::GfVec3f(1,1,0));
		points.push_back(pxr::GfVec3f(-1,1,0));
		return pxr::VtValue(points);
	}
	if (key == pxr::TfToken("uv"))
	{
		pxr::VtVec2fArray uvs;

		uvs.push_back(pxr::GfVec2f(0,0));
		uvs.push_back(pxr::GfVec2f(1,0));
		uvs.push_back(pxr::GfVec2f(1,1));
		uvs.push_back(pxr::GfVec2f(0,1));

		return pxr::VtValue(uvs);
	}
	if (key == pxr::HdTokens->color)
	{
		return pxr::VtValue(pxr::GfVec4f(0.9f, 0.9f, 0.9f, 1.0));
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
	return pxr::GfMatrix4d(1.0f);
}

pxr::HdMeshTopology SceneDelegate::GetMeshTopology(pxr::SdfPath const &id)
{
	std::cout << "[" << id.GetString() <<"][Topology]" << std::endl;
	pxr::VtArray<int> vertCountsPerFace;
	pxr::VtArray<int> verts;
	vertCountsPerFace.push_back(4);

	verts.push_back(0);
	verts.push_back(1);
	verts.push_back(2);
	verts.push_back(3);

	pxr::HdMeshTopology topology(pxr::PxOsdOpenSubdivTokens->none, pxr::HdTokens->rightHanded, vertCountsPerFace, verts);
	return topology;
}


pxr::HdPrimvarDescriptorVector SceneDelegate::GetPrimvarDescriptors(pxr::SdfPath const& id, pxr::HdInterpolation interpolation)
{
	std::cout << "[" << id.GetString() <<"][GetPrimvarDescriptors]" << std::endl;
	pxr::HdPrimvarDescriptorVector primvarDescriptors;

	if (interpolation == pxr::HdInterpolation::HdInterpolationVertex)
	{
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->points, interpolation));
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::TfToken("uv"), interpolation));
	}
	else if (interpolation == pxr::HdInterpolationConstant)
	{
		primvarDescriptors.push_back(pxr::HdPrimvarDescriptor(pxr::HdTokens->color, interpolation));
	}


	return primvarDescriptors;
}


std::string SceneDelegate::GetSurfaceShaderSource(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderSource]" << std::endl;

	// render the UVs
	//return "vec4 surfaceShader(vec4 Peye, vec3 Neye, vec4 color, vec4 patchCoord) { return vec4(HdGet_uv().x, HdGet_uv().y, 0, 1) * color; }";

	// sample from the textures using the UVs
	return "vec4 surfaceShader(vec4 Peye, vec3 Neye, vec4 color, vec4 patchCoord) { return HdGet_textureColor(HdGet_uv()) * color; }";
}

std::string SceneDelegate::GetDisplacementShaderSource(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetDisplacementShaderSource]" << std::endl;
	return "vec4 displacementShader(int  a, vec4 b, vec3 c, vec4 d) { return b; }";
}

pxr::VtValue SceneDelegate::GetMaterialParamValue(pxr::SdfPath const &shaderId, const pxr::TfToken &paramName)
{
	std::cout << "[" << shaderId.GetString() << "." << paramName <<"][GetSurfaceShaderParamValue]" << std::endl;

	return  pxr::VtValue();
}

pxr::HdMaterialParamVector SceneDelegate::GetMaterialParams(pxr::SdfPath const &shaderId)
{
	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderParams]" << std::endl;

	pxr::HdMaterialParamVector r;
	pxr::HdMaterialParam param(pxr::TfToken("textureColor"),  pxr::VtValue(pxr::GfVec4f(0.9f, 0.9f, 0.9f, 1.0)), pxr::SdfPath("/texture") );
	r.push_back(param);
	return r;
}

//pxr::SdfPathVector SceneDelegate::GetSurfaceShaderTextures(pxr::SdfPath const &shaderId)
//{
//	std::cout << "[" << shaderId.GetString() <<"][GetSurfaceShaderTextures]" << std::endl;
//	pxr::SdfPathVector r = { pxr::SdfPath("/texture") };
//	return r;
//}

//pxr::HdTextureResource::ID SceneDelegate::GetTextureResourceID(pxr::SdfPath const &textureId)
//{
//	std::cout << "[" << textureId.GetString() <<"][GetTextureResourceID]" << std::endl;
//	return 0;
//}

pxr::HdTextureResourceSharedPtr SceneDelegate::GetTextureResource(pxr::SdfPath const &textureId)
{
//	std::cout << "[" << textureId.GetString() <<"][GetTextureResource]" << std::endl;
//	if (!textureHandle)
//	{
//		textureHandle = pxr::GlfTextureRegistry::GetInstance().GetTextureHandle(pxr::TfToken("default.jpg"));
//		textureHandle->AddMemoryRequest(1024 * 1024);
//		std::cout << textureHandle << std::endl;
//
//		pxr::HdSimpleTextureResource *p = new pxr::HdSimpleTextureResource(textureHandle, false /* not ptex */ );
//		textureResource = pxr::HdTextureResourceSharedPtr(p);
//	}
//
//	return textureResource;
	return pxr::HdTextureResourceSharedPtr();

}
