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
#ifndef PXR_USD_IMAGING_USD_IMAGING_GL_UNIT_TEST_GLDRAWING_H
#define PXR_USD_IMAGING_USD_IMAGING_GL_UNIT_TEST_GLDRAWING_H

#include "Args.h"
#include "pxr/pxr.h"
#include "pxr/base/gf/vec4d.h"
#include "pxr/base/vt/dictionary.h"
#include "pxr/base/tf/declarePtrs.h"
#include <pxr/usd/usd/stage.h>
#include "pxr/usdImaging/usdImagingGL/engine.h"

#include <string>
#include <vector>

PXR_NAMESPACE_OPEN_SCOPE

class UsdImagingGL_UnitTestWindow;

/// \class UsdImagingGL_UnitTestGLDrawing
///
/// A helper class for unit tests which need to perform GL drawing.
///
class UsdImagingGL_UnitTestGLDrawing
{
    Args args;
public:
    UsdImagingGL_UnitTestGLDrawing(const Args &args);
    ~UsdImagingGL_UnitTestGLDrawing();

    int GetWidth() const;
    int GetHeight() const;

    UsdImagingGLDrawMode GetDrawMode() const { return _drawMode; }
    std::vector<GfVec4d> const &GetClipPlanes() const { return _clipPlanes; }
    GfVec4f const &GetClearColor() const { return _clearColor; }
    GfVec3f const &GetTranslate() const { return _translate; }

    void InitTest();
    void DrawTest(bool offscreen);
    void ShutdownTest();

    void MousePress(int button, int x, int y, int modKeys);
    void MouseRelease(int button, int x, int y, int modKeys);
    void MouseMove(int x, int y, int modKeys);
    void KeyRelease(int key);

    bool WriteToFile(std::string const &attachment, std::string const &filename) const;

    void RunTest();

private:
    HdRenderIndex *_GetRenderIndex(UsdImagingGLEngine *engine)
    {
        return engine->_GetRenderIndex();
    }

    void _Render(UsdImagingGLEngine *engine,
                 const UsdImagingGLRenderParams &params)
    {
        SdfPathVector roots(1, SdfPath::AbsoluteRootPath());
        engine->RenderBatch(roots, params);
    }

private:
    UsdImagingGL_UnitTestWindow *_widget = nullptr;

    std::vector<GfVec4d> _clipPlanes;

    UsdImagingGLDrawMode _drawMode = UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH;

    GfVec4f _clearColor;
    GfVec3f _translate;

private:
    pxr::UsdStageRefPtr _stage;
    std::shared_ptr<pxr::UsdImagingGLEngine> _engine;
    pxr::GlfSimpleLightingContextRefPtr _lightingContext;

    float _rotate[2] = {0, 0};
    float __translate[3] = {0, 0, 0};
    int _mousePos[2] = {0, 0};
    bool _mouseButton[3] = {false, false, false};
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_IMAGING_USD_IMAGING_GL_UNIT_TEST_GLDRAWING_H