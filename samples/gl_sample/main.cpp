#include <iostream>
#include "Args.h"
#include "unitTestGLDrawing.h"
#include <pxr/base/tf/stringUtils.h>

int main(int argc, char *argv[])
{
    Args args;
    args.Parse(argc, argv);

    {
        pxr::UsdImagingGL_UnitTestGLDrawing driver(args);
        driver.RunTest();
    }

    std::cout << "OK" << std::endl;
}
