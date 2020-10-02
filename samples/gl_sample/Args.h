#pragma once
#include <string>
#include <vector>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/dictionary.h>

struct Args
{
    std::string _stageFilePath;
    pxr::TfToken _renderer;
    pxr::TfToken _rendererAov;

    pxr::VtDictionary _renderSettings;

    std::string const &GetStageFilePath() const { return _stageFilePath; }

    pxr::TfToken _GetRenderer() const { return _renderer; }
    pxr::VtDictionary const &GetRenderSettings() const { return _renderSettings; }
    pxr::TfToken const &GetRendererAov() const { return _rendererAov; }

    void Parse(int argc, char *argv[]);
};
