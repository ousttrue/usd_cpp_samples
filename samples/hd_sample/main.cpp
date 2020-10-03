#include "testHdxRenderer.h"
#include <pxr/base/tf/errorMark.h>
#include <iostream>

int main(int argc, char *argv[])
{
    pxr::TfErrorMark mark;

    RunTest(argc, argv);

    if (mark.IsClean())
    {
        std::cout << "OK" << std::endl;
        return EXIT_SUCCESS;
    }
    else
    {
        std::cout << "FAILED" << std::endl;
        return EXIT_FAILURE;
    }
}
