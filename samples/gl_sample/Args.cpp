#include "Args.h"
#include <pxr/usdImaging/usdImagingGL/renderParams.h>

static void Die(const char *fmt, ...) ARCH_PRINTF_FUNCTION(1, 2);
static void Die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fflush(stderr);
    exit(1);
}

static void
ParseError(const char *pname, const char *fmt, ...) ARCH_PRINTF_FUNCTION(2, 3);
static void
ParseError(const char *pname, const char *fmt, ...)
{
    fprintf(stderr, "%s: ", pxr::TfGetBaseName(pname).c_str());
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, ".  Try '%s -' for help.\n", pxr::TfGetBaseName(pname).c_str());
    fflush(stderr);
    exit(1);
}

static void Usage(int argc, char *argv[])
{
    static const char usage[] =
        "%s [-stage filePath] [-write filePath]\n"
        "                           [-offscreen] [-lighting] [-idRender]\n"
        "                           [-camera pathToCamera]\n"
        "                           [-complexity complexity]\n"
        "                           [-renderer rendererName]\n"
        "                           [-shading [flat|smooth|wire|wireOnSurface]]\n"
        "                           [-frameAll]\n"
        "                           [-clipPlane clipPlane1 ... clipPlane4]\n"
        "                           [-complexities complexities1 complexities2 ...]\n"
        "                           [-times times1 times2 ...] [-cullBackfaces]\n"
        "                           [-clear r g b a] [-clearOnce] [-translate x y z]\n"
        "                           [-renderSetting name type value]\n"
        "                           [-rendererAov name]\n"
        "                           [-perfStatsFile path]\n"
        "                           [-traceFile path] [...]\n"
        "\n"
        "  usdImaging basic drawing test\n"
        "\n"
        "options:\n"
        "  -stage filePath     name of usd stage to open []\n"
        "  -write filePath     name of image file to write (suffix determines type) []\n"
        "  -offscreen          execute without mapping a window\n"
        "  -lighting           use simple lighting override shader\n"
        "  -sceneLights        use in combination with -lighting to utilize the lights \n"
        "                      defined in the scene\n"
        "  -camLight           use a single camera light\n"
        "  -idRender           ID rendering\n"
        "  -complexity complexity\n"
        "                      Set the fallback complexity [1]\n"
        "  -renderer rendererName\n"
        "                      use the specified renderer plugin []\n"
        "  -shading [flat|smooth|wire|wireOnSurface]\n"
        "                      force specific type of shading\n"
        "                      [flat|smooth|wire|wireOnSurface] []\n"
        "  -frameAll           set the view to frame all root prims on the stage\n"
        "  -clipPlane clipPlane1 ... clipPlane4\n"
        "                      set an additional camera clipping plane [()]\n"
        "  -complexities complexities1 complexities2 ...\n"
        "                      One or more complexities, each complexity will\n"
        "                      produce an image [()]\n"
        "  -times times1 times2 ...\n"
        "                      One or more time samples, each time will produce\n"
        "                      an image [()]\n"
        "  -cullBackfaces      enable backface culling\n"
        "  -clear r g b a      clear color\n"
        "  -clearOnce          Clear the framebuffer only once at the start \n"
        "                      instead of before each render.\n"
        "  -translate x y z    default camera translation\n"
        "  -rendererAov name   Name of AOV to display or write out\n"
        "  -perfStatsFile path Path to file performance stats are written to\n"
        "  -traceFile path     Path to trace file to write\n"
        "  -renderSetting name type value\n"
        "                      Specifies a setting with given name, type (such as\n"
        "                      float) and value passed to renderer. -renderSetting\n"
        "                      can be given multiple times to specify different\n"
        "                      settings\n"
        "  -guidesPurpose [show|hide]\n"
        "                      force prims of purpose 'guide' to be shown or hidden\n"
        "  -renderPurpose [show|hide]\n"
        "                      force prims of purpose 'render' to be shown or hidden\n"
        "  -proxyPurpose [show|hide]\n"
        "                      force prims of purpose 'proxy' to be shown or hidden\n";

    Die(usage, pxr::TfGetBaseName(argv[0]).c_str());
}

static void CheckForMissingArguments(int i, int n, int argc, char *argv[])
{
    if (i + n >= argc)
    {
        if (n == 1)
        {
            ParseError(argv[0], "missing parameter for '%s'", argv[i]);
        }
        else
        {
            ParseError(argv[0], "argument '%s' requires %d values", argv[i], n);
        }
    }
}

static double ParseDouble(int &i, int argc, char *argv[],
                          bool *invalid = nullptr)
{
    if (i + 1 == argc)
    {
        if (invalid)
        {
            *invalid = true;
            return 0.0;
        }
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
    }
    char *end;
    double result = strtod(argv[i + 1], &end);
    if (end == argv[i + 1] || *end != '\0')
    {
        if (invalid)
        {
            *invalid = true;
            return 0.0;
        }
        ParseError(argv[0], "invalid parameter for '%s': %s",
                   argv[i], argv[i + 1]);
    }
    ++i;
    if (invalid)
    {
        *invalid = false;
    }
    return result;
}

static bool ParseShowHide(int &i, int argc, char *argv[],
                          bool *result)
{
    if (i + 1 == argc)
    {
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
        return false;
    }
    if (strcmp(argv[i + 1], "show") == 0)
    {
        *result = true;
    }
    else if (strcmp(argv[i + 1], "hide") == 0)
    {
        *result = false;
    }
    else
    {
        ParseError(argv[0], "invalid parameter for '%s': %s. Must be either "
                            "'show' or 'hide'",
                   argv[i], argv[i + 1]);
        return false;
    }

    ++i;
    return true;
}

static const char *ParseString(int &i, int argc, char *argv[],
                               bool *invalid = nullptr)
{
    if (i + 1 == argc)
    {
        if (invalid)
        {
            *invalid = true;
            return nullptr;
        }
        ParseError(argv[0], "missing parameter for '%s'", argv[i]);
    }
    const char *const result = argv[i + 1];
    ++i;
    if (invalid)
    {
        *invalid = false;
    }
    return result;
}

static void
ParseDoubleVector(
    int &i, int argc, char *argv[],
    std::vector<double> *result)
{
    bool invalid = false;
    while (i != argc)
    {
        const double value = ParseDouble(i, argc, argv, &invalid);
        if (invalid)
        {
            break;
        }
        result->push_back(value);
    }
}

static pxr::VtValue ParseVtValue(int &i, int argc, char *argv[])
{
    const char *const typeString = ParseString(i, argc, argv);

    if (strcmp(typeString, "float") == 0)
    {
        CheckForMissingArguments(i, 1, argc, argv);
        return pxr::VtValue(float(ParseDouble(i, argc, argv)));
    }
    else
    {
        ParseError(argv[0], "unknown type '%s'", typeString);
        return pxr::VtValue();
    }
}

void Args::Parse(int argc, char *argv[])
{
    for (int i = 1; i != argc; ++i)
    {
        if (strcmp(argv[i], "-") == 0)
        {
            Usage(argc, argv);
        }
        else if (strcmp(argv[i], "-stage") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _stageFilePath = argv[++i];
        }
        else if (strcmp(argv[i], "-write") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _outputFilePath = argv[++i];
        }
        else if (strcmp(argv[i], "-renderer") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _renderer = pxr::TfToken(argv[++i]);
        }
        else if (strcmp(argv[i], "-rendererAov") == 0)
        {
            CheckForMissingArguments(i, 1, argc, argv);
            _rendererAov = pxr::TfToken(argv[++i]);
        }
        else if (strcmp(argv[i], "-renderSetting") == 0)
        {
            CheckForMissingArguments(i, 2, argc, argv);
            const char *const key = ParseString(i, argc, argv);
            _renderSettings[key] = ParseVtValue(i, argc, argv);
        }
        else
        {
            ParseError(argv[0], "unknown argument %s", argv[i]);
        }
    }
}
