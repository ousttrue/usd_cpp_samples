#pragma once
#include <string>
#include <vector>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/dictionary.h>

struct Args
{
    std::string unresolvedStageFilePath;

    std::string shading;

    float clearColor[4] = {1.0f, 0.5f, 0.1f, 1.0f};
    float translate[3] = {0.0f, -1000.0f, -2500.0f};

    bool _shouldFrameAll = false;
    bool _cullBackfaces = false;
    bool _testLighting = false;
    bool _sceneLights = false;
    bool _cameraLight = false;
    std::string _cameraPath;
    bool _testIdRender = false;
    std::string _stageFilePath;
    std::string _outputFilePath;
    float _complexity = 1.0f;
    pxr::TfToken _renderer;
    pxr::TfToken _rendererAov;
    std::string _perfStatsFile;
    std::string _traceFile;

    pxr::VtDictionary _renderSettings;
    bool _showGuides;
    bool _showRender;
    bool _showProxy;
    bool _clearOnce = false;

    Args();

    std::string const &GetStageFilePath() const { return _stageFilePath; }
    std::string const &GetOutputFilePath() const { return _outputFilePath; }

    bool IsEnabledTestLighting() const { return _testLighting; }
    bool IsEnabledSceneLights() const { return _sceneLights; }
    bool IsEnabledCameraLight() const { return _cameraLight; }
    bool IsEnabledCullBackfaces() const { return _cullBackfaces; }
    bool IsEnabledIdRender() const { return _testIdRender; }

    bool IsShowGuides() const { return _showGuides; }
    bool IsShowRender() const { return _showRender; }
    bool IsShowProxy() const { return _showProxy; }
    bool ShouldClearOnce() const { return _clearOnce; }

    float _GetComplexity() const { return _complexity; }
    bool _ShouldFrameAll() const { return _shouldFrameAll; }
    pxr::TfToken _GetRenderer() const { return _renderer; }
    pxr::VtDictionary const &GetRenderSettings() const { return _renderSettings; }
    std::string const &GetCameraPath() const { return _cameraPath; }
    pxr::TfToken const &GetRendererAov() const { return _rendererAov; }
    std::string const &GetPerfStatsFile() const { return _perfStatsFile; }

    void Parse(int argc, char *argv[]);
};
