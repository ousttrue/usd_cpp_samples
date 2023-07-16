#include <pxr/usd/usd/stage.h>
#include <pxr/usd/sdf/path.h>
#include <pxr/usd/usdGeom/sphere.h>

int main()
{
    auto stage = pxr::UsdStage::CreateInMemory();
    // auto sphere = pxr::UsdGeomSphere::Define(stage, pxr::SdfPath("/TestSphere"));
    // auto root = stage->GetRootLayer();
    // root->Export("test_sphere.usda");

    return 0;
}
