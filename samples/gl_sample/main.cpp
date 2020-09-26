#include <iostream>
#include "unitTestGLDrawing.h"

int main(int argc, char *argv[])
{
    {
        pxr::UsdImagingGL_UnitTestGLDrawing driver;
        driver.RunTest(argc, argv);
    }

    std::cout << "OK" << std::endl;
}
