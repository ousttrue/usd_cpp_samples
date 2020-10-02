#pragma once
#include <string>
#include <vector>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/dictionary.h>

struct Args
{
    std::string _stageFilePath;

    pxr::VtDictionary _renderSettings;

    std::string const &GetStageFilePath() const { return _stageFilePath; }

    pxr::VtDictionary const &GetRenderSettings() const { return _renderSettings; }

    void Parse(int argc, char *argv[]);
};
