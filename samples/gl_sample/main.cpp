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
        pxr::UsdImagingGL_UnitTestGLDrawing driver(args);

        // create window
        auto _widget = new UsdImagingGL_UnitTestWindow(
            640, 480,
            [&driver]() { driver.InitTest(); },
            [&driver](bool offscreen, int w, int h) {
                driver.DrawTest(offscreen, w, h);
            });

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

                _widget->DrawOffscreen();
            }
        }
        else if (args.offscreen)
        {
            _widget->DrawOffscreen();
        }
        else
        {
            _widget->Run();
        }
    }

    std::cout << "OK" << std::endl;
}
