#pragma once
#include <stdint.h>
#include <pxr/usd/usd/timeCode.h>

class GLDrawing
{
    class Impl *_impl = nullptr;

public:
    GLDrawing(const struct Args &args);
    ~GLDrawing();

    uint32_t Draw(int w, int h, const pxr::UsdTimeCode &time);
    void Shutdown();
    void MousePress(int button, int x, int y, int modKeys);
    void MouseRelease(int button, int x, int y, int modKeys);
    void MouseMove(int x, int y, int modKeys);
    void KeyRelease(int key);
};
