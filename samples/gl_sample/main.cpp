#include <iostream>
#include "Args.h"
#include "UnitTestWindow.h"
#include "unitTestGLDrawing.h"

#include <pxr/base/tf/stringUtils.h>


int main(int argc, char *argv[])
{
    Args args;
    args.Parse(argc, argv);

    {
        pxr::UnitTestGLDrawing driver(args);

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

        // create window
        auto _widget = new UnitTestWindow(
            640, 480,
            [&driver](int w, int h) { },
            [&driver](int w, int h) {
                return driver.DrawTest(w, h);
            },
            [&driver]() { driver.ShutdownTest(); },
            input);

        _widget->Init();

        if (args.complexities.size() > 0)
        {
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

                driver.DrawTest(_widget->GetWidth(), _widget->GetHeight());
            }
        }
        else if (args.offscreen)
        {
            driver.DrawTest(_widget->GetWidth(), _widget->GetHeight());
        }
        else
        {
            _widget->Run();
        }
    }

    std::cout << "OK" << std::endl;
}
