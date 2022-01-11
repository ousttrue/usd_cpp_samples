#include "source.h"
#include <iostream>
#include <fstream>

class TinySceneDelegate final : public pxr::HdSceneDelegate
{
public:

	TinySceneDelegate(pxr::HdRenderIndex* parentindex, pxr::SdfPath const& id) :pxr::HdSceneDelegate(parentindex, id) {};

	pxr::GfMatrix4d GetTransform(pxr::SdfPath const& id) override
	{
		if (_meshes.find(id) != _meshes.end()) {
			return pxr::GfMatrix4d(_meshes[id].transform);
		}

		return pxr::GfMatrix4d(1);
	}

	void setTime(unsigned int time) {
		pxr::SdfPath id1("/Cube1");
		GetRenderIndex().GetChangeTracker().MarkRprimDirty(id1, pxr::HdChangeTracker::DirtyTransform);
	}

	void AddMesh(pxr::SdfPath& id) {
		pxr::HdRenderIndex& index = GetRenderIndex();
		index.InsertRprim(pxr::HdPrimTypeTokens->mesh, this, id);
		_meshes[id] = _Mesh();
	}

	void Populate() {

		pxr::SdfPath id1("/Cube1");
		AddMesh(id1);


		pxr::SdfPath id2("/Cube2");
		AddMesh(id2);

	};

	struct _Mesh {
		_Mesh() { }
		pxr::GfMatrix4f transform;
	};

	std::map<pxr::SdfPath, _Mesh> _meshes;
};

class TinyRenderDelegate_Mesh final : public pxr::HdMesh
{
public:
	TinyRenderDelegate_Mesh(pxr::TfToken const& typeId, pxr::SdfPath const& rprimId)
		:pxr::HdMesh(rprimId) {}

	pxr::HdDirtyBits GetInitialDirtyBitsMask() const override
	{
		return pxr::HdChangeTracker::AllDirty;
	}

	void Sync(pxr::HdSceneDelegate *delegate, pxr::HdRenderParam *renderParam, pxr::HdDirtyBits *dirtyBits, pxr::TfToken const &reprToken) override {
		pxr::SdfPath const& id = GetId();

		if (pxr::HdChangeTracker::IsTransformDirty(*dirtyBits, id)) {
			delegate->GetTransform(id);
			std::cout << "pulling new transform -> " << id << std::endl;
		}

		*dirtyBits &= ~pxr::HdChangeTracker::AllSceneDirtyBits;
	}

	pxr::HdDirtyBits _PropagateDirtyBits(pxr::HdDirtyBits bits) const override {
		return bits;
	}

	void _InitRepr(pxr::TfToken const &reprToken, pxr::HdDirtyBits *dirtyBits) override {
		TF_UNUSED(dirtyBits);

		_ReprVector::iterator it = std::find_if(_reprs.begin(), _reprs.end(),
			_ReprComparator(reprToken));
		if (it == _reprs.end()) {
			_reprs.emplace_back(reprToken, pxr::HdReprSharedPtr());
		}
	};
};

class TinyRenderDelegate_RenderPass final : public pxr::HdRenderPass
{
public:
	TinyRenderDelegate_RenderPass(pxr::HdRenderIndex *index, pxr::HdRprimCollection const& collection) :pxr::HdRenderPass(index, collection) {  }

	void _Execute(pxr::HdRenderPassStateSharedPtr const &renderPassState, pxr::TfTokenVector const &renderTags) {
		std::cout << "(1) Generating image" << std::endl;
	};

};


class TinyRenderDelegate final : public pxr::HdRenderDelegate
{
public:

	TinyRenderDelegate() :pxr::HdRenderDelegate() { }

	pxr::TfTokenVector SUPPORTED_RPRIM_TYPES =
	{
		pxr::HdPrimTypeTokens->mesh
	};

	pxr::TfTokenVector SUPPORTED_SPRIM_TYPES =
	{
	};

	pxr::TfTokenVector SUPPORTED_BPRIM_TYPES =
	{
	};

	virtual pxr::TfTokenVector const &GetSupportedRprimTypes() const override { return SUPPORTED_RPRIM_TYPES; }
	virtual pxr::TfTokenVector const &GetSupportedBprimTypes() const override { return SUPPORTED_BPRIM_TYPES; }
	const pxr::TfTokenVector & GetSupportedSprimTypes() const override { return SUPPORTED_SPRIM_TYPES; }

	virtual pxr::HdRenderParam *GetRenderParam() const override { return nullptr; }
	virtual pxr::HdResourceRegistrySharedPtr GetResourceRegistry() const override { return nullptr; }

	virtual pxr::HdRenderPassSharedPtr CreateRenderPass(pxr::HdRenderIndex *index, pxr::HdRprimCollection const& collection) override {
		return pxr::HdRenderPassSharedPtr(new TinyRenderDelegate_RenderPass(index, collection));
	};

	virtual pxr::HdInstancer *CreateInstancer(pxr::HdSceneDelegate *delegate, pxr::SdfPath const& id) override { return nullptr; };
	virtual void DestroyInstancer(pxr::HdInstancer *instancer) override { delete instancer; };

	virtual pxr::HdRprim *CreateRprim(pxr::TfToken const& typeId, pxr::SdfPath const &rprimId) override {
		return  new TinyRenderDelegate_Mesh(typeId, rprimId);
	}
	virtual void DestroyRprim(pxr::HdRprim *rPrim) override { delete rPrim; };

	virtual pxr::HdSprim *CreateSprim(pxr::TfToken const& typeId, pxr::SdfPath const& sprimId) override { return nullptr; };
	virtual pxr::HdSprim *CreateFallbackSprim(pxr::TfToken const& typeId) override { return nullptr; };
	virtual void DestroySprim(pxr::HdSprim *sprim)override { delete sprim; };

	virtual pxr::HdBprim *CreateBprim(pxr::TfToken const& typeId, pxr::SdfPath const& bprimId)override { return nullptr; };
	virtual pxr::HdBprim *CreateFallbackBprim(pxr::TfToken const& typeId)override { return nullptr; };
	virtual void DestroyBprim(pxr::HdBprim *bprim)override { delete bprim; };

	virtual void CommitResources(pxr::HdChangeTracker *tracker)override { };

};


class TinyRenderTask final : public pxr::HdTask {
public:
	TinyRenderTask(pxr::HdRenderPassSharedPtr renderPass, pxr::HdRenderPassStateSharedPtr renderPassState, pxr::HdRenderIndex *index)
		:pxr::HdTask(pxr::SdfPath::AbsoluteRootPath()), _renderPass(renderPass), _renderPassState(renderPassState), _index(index) {};

	virtual void Sync(pxr::HdSceneDelegate* delegate, pxr::HdTaskContext* ctx, pxr::HdDirtyBits* dirtyBits) override {
		_renderPass->Sync();
	};

	virtual void Prepare(pxr::HdTaskContext* ctx, pxr::HdRenderIndex* renderIndex) override {
		// _renderPass->Prepare(pxr::TfTokenVector());
	};

	void Execute(pxr::HdTaskContext* ctx) override
	{
		_renderPass->Execute(_renderPassState, pxr::TfTokenVector());
	};

	pxr::HdRenderPassSharedPtr _renderPass;
	pxr::HdRenderPassStateSharedPtr _renderPassState;
	pxr::HdRenderIndex *_index;
};

class TinyColorCorrectionTask final : public pxr::HdTask {
public:
	TinyColorCorrectionTask() :pxr::HdTask(pxr::SdfPath::AbsoluteRootPath()) {};

	virtual void Sync(pxr::HdSceneDelegate* delegate, pxr::HdTaskContext* ctx, pxr::HdDirtyBits* dirtyBits) override {
	};

	virtual void Prepare(pxr::HdTaskContext* ctx, pxr::HdRenderIndex* renderIndex) override {
	};

	void Execute(pxr::HdTaskContext* ctx) override {
		std::cout << "(2) Color correcting the image" << std::endl;
	}
};

int main() {
	pxr::HdEngine z;
	TinyRenderDelegate renderDelegate;
	pxr::HdRenderIndex *renderIndex = pxr::HdRenderIndex::New(&renderDelegate, {});
	TinySceneDelegate sceneDelegate(renderIndex, pxr::SdfPath::AbsoluteRootPath());


	pxr::HdRprimCollection collection(pxr::TfToken("testCollection"), pxr::HdReprSelector(pxr::HdReprTokens->hull));
	pxr::HdRenderPassSharedPtr renderPass(renderDelegate.CreateRenderPass(renderIndex, collection));
	pxr::HdRenderPassStateSharedPtr renderPassState(renderDelegate.CreateRenderPassState());
	pxr::HdTaskSharedPtr taskRender(new TinyRenderTask(renderPass, renderPassState, renderIndex));
	pxr::HdTaskSharedPtr taskColorCorrect(new TinyColorCorrectionTask());
	pxr::HdTaskSharedPtrVector tasks = { taskRender,taskColorCorrect };


	sceneDelegate.Populate();
	z.Execute(renderIndex, &tasks);

	std::cout << "" << std::endl;

	sceneDelegate.setTime(2);
	z.Execute(renderIndex, &tasks);



	return 0;
}

