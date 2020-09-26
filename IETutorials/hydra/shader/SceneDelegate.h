#pragma once

#include "pxr/imaging/hd/sceneDelegate.h"

#include "pxr/base/gf/vec4f.h"

class SceneDelegate : public pxr::HdSceneDelegate
{
public:
	SceneDelegate(pxr::HdRenderIndex *parentIndex, pxr::SdfPath const& delegateID);

	void AddRenderTask(pxr::SdfPath const &id);
	void AddRenderSetupTask(pxr::SdfPath const &id);

	void SetCamera(pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix);
	void SetCamera(pxr::SdfPath const &cameraId, pxr::GfMatrix4d const &viewMatrix, pxr::GfMatrix4d const &projMatrix);

	// interface Hydra uses query information from the SceneDelegate when processing an 
	// Prim in the RenderIndex
	pxr::VtValue Get(pxr::SdfPath const &id, const pxr::TfToken &key) override;
	bool GetVisible(pxr::SdfPath const &id) override;

	pxr::GfRange3d GetExtent(pxr::SdfPath const &id) override;
	pxr::GfMatrix4d GetTransform(pxr::SdfPath const &id) override;

	pxr::HdMeshTopology GetMeshTopology(pxr::SdfPath const &id) override;
//	pxr::TfTokenVector GetPrimVarVertexNames(pxr::SdfPath const &id) override;
//	pxr::TfTokenVector GetPrimVarConstantNames(pxr::SdfPath const& id) override;

	pxr::HdPrimvarDescriptorVector GetPrimvarDescriptors(pxr::SdfPath const& id, pxr::HdInterpolation interpolation) override;

	std::string GetSurfaceShaderSource(pxr::SdfPath const &shaderId) override;
	std::string GetDisplacementShaderSource(pxr::SdfPath const &shaderId) override;
	pxr::VtValue GetMaterialParamValue(pxr::SdfPath const &shaderId, const pxr::TfToken &paramName) override;
	pxr::HdMaterialParamVector GetMaterialParams(pxr::SdfPath const &shaderId) override;
	//pxr::SdfPathVector GetMaterialTextures(pxr::SdfPath const &shaderId) override;

	void UpdateColor();
private:
	// per location (SdfPath) cache of dictionaries (TfToken -> VtValue) 
	typedef pxr::TfHashMap<pxr::TfToken, pxr::VtValue, pxr::TfToken::HashFunctor> _ValueCache;
	typedef pxr::TfHashMap<pxr::SdfPath, _ValueCache, pxr::SdfPath::Hash> _ValueCacheMap;
	_ValueCacheMap _valueCacheMap;

	// location of Camera (setup in ctor)
	pxr::SdfPath cameraPath;

	std::string shaderSource;

	pxr::GfVec4f color;
	float time;
};