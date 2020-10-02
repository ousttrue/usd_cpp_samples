#pragma once
#include <string>
#include <vector>
#include <pxr/base/tf/token.h>
#include <pxr/base/vt/dictionary.h>

struct Args
{
    std::string _stageFilePath;
    std::string const &GetStageFilePath() const { return _stageFilePath; }

    void Parse(int argc, char *argv[]);
};
