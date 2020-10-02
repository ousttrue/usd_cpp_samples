#pragma once
#include <string>
#include <vector>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/dictionary.h>

struct Args
{
    std::string _stageFilePath;
    std::string _outputFilePath;
    pxr::TfToken _renderer;
    pxr::TfToken _rendererAov;

    pxr::VtDictionary _renderSettings;

    std::string const &GetStageFilePath() const { return _stageFilePath; }
    std::string const &GetOutputFilePath() const { return _outputFilePath; }

    pxr::TfToken _GetRenderer() const { return _renderer; }
    pxr::VtDictionary const &GetRenderSettings() const { return _renderSettings; }
    pxr::TfToken const &GetRendererAov() const { return _rendererAov; }

    void Parse(int argc, char *argv[]);
};
