#pragma once

class Drawing
{
    class My_TestGLDrawing *_impl = nullptr;

public:
    Drawing();
    ~Drawing();
    void Init(int width, int height);
    void Uninit();
    void Draw(int width, int height);
    void MousePress(int button, int x, int y, int modKeys);
    void MouseRelease(int button, int x, int y, int modKeys);
    void MouseMove(int x, int y, int modKeys);
    void KeyRelease(int key);
};
