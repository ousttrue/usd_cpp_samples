#include <iostream>
#include "Args.h"
#include "UnitTestWindow.h"
#include "GLDrawing.h"

int main(int argc, char *argv[])
{
    Args args;
    args.Parse(argc, argv);

    GLDrawing driver(args);

    // create GL Context / window
    Input input;
    input.OnMousePress = [&driver](int button, int x, int y, int mod) {
        driver.MousePress(button, x, y, mod);
    };
    input.OnMouseRelease = [&driver](int button, int x, int y, int mod) {
        driver.MouseRelease(button, x, y, mod);
    };
    input.OnMouseMove = [&driver](int button, int x, int y) {
        driver.MouseMove(button, x, y);
    };
    input.OnKeyRelease = [&driver](int key) {
        driver.KeyRelease(key);
    };
    auto context = UnitTestWindow(
        640, 480,
        [&driver](int w, int h) {},
        [&driver](int w, int h) {
            return driver.Draw(w, h);
        },
        [&driver]() { driver.Shutdown(); },
        input);

    context.Init();

    // RUN
    if (args.complexities.size() > 0)
    {
        // OFFSCREEN
        std::string imageFilePath = args.GetOutputFilePath();
        for (auto &compIt : args.complexities)
        {
            args._complexity = compIt;
            if (!imageFilePath.empty())
            {
                std::stringstream suffix;
                suffix << "_" << args._complexity << ".png";
                args._outputFilePath = pxr::TfStringReplace(imageFilePath, ".png", suffix.str());
            }
            driver.Draw(context.GetWidth(), context.GetHeight());
        }
    }
    else if (args.offscreen)
    {
        // OFFSCREEN
        driver.Draw(context.GetWidth(), context.GetHeight());
    }
    else
    {
        // WINDOW main loop
        context.Run();
    }

    std::cout << "OK" << std::endl;
}
