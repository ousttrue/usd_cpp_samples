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

    bool _testLighting = false;
    bool _sceneLights = false;
    bool _cameraLight = false;
    std::string _cameraPath;
    std::string _stageFilePath;
    std::string _outputFilePath;
    pxr::TfToken _renderer;
    pxr::TfToken _rendererAov;

    pxr::VtDictionary _renderSettings;

    std::string const &GetStageFilePath() const { return _stageFilePath; }
    std::string const &GetOutputFilePath() const { return _outputFilePath; }

    bool IsEnabledTestLighting() const { return _testLighting; }
    bool IsEnabledSceneLights() const { return _sceneLights; }
    bool IsEnabledCameraLight() const { return _cameraLight; }
    pxr::TfToken _GetRenderer() const { return _renderer; }
    pxr::VtDictionary const &GetRenderSettings() const { return _renderSettings; }
    std::string const &GetCameraPath() const { return _cameraPath; }
    pxr::TfToken const &GetRendererAov() const { return _rendererAov; }

    void Parse(int argc, char *argv[]);
};
