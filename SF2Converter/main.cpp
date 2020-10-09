#include <SDL.h>
#include <iostream>
#include <iomanip>
#include <vector>

#ifdef _SF2_WINDOWS
#include "foundation/platform/sdl/platform_sdl_windows.h"
#elif _SF2_LINUX
#include "foundation/platform/sdl/platform_sdl_linux.h"
#else
#include "foundation/platform/sdl/platform_sdl_macos.h"
#endif
#include "converter/misc.h"
#include "converter/test.h"
#include "converter/sf2_interface.h"
#include "converter/source_utils.h"

// @todo Add command-line option for forcing a specific file format (i.e. bypassing detection)

using namespace SF2;
using namespace Foundation;
using namespace Converter;

//#define DEBUG_ARGS
//#define DEBUG_CLASS

int main(int argc, char* argv[])
{
    std::cout
        << "SF2Converter by Jens-Christian Huus - build " << __DATE__ << ".\n"
        << "Converts other source files to SF2 for use by SID Factory II.\n\n";

    int leaveout = 4, transpose = 0;
    std::string source_filename, destination_filename = "";

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg.front() == '-' || arg.front() == '/')
        {
            // Option specified
            if (arg.length() < 2) argc = 0;
            else
            {
                switch (arg.at(1))
                {
                case 'l':
                    if (arg.length() != 4 || arg.at(2) != ':')
                        argc = 0;
                    else
                    {
                        leaveout = arg.at(3) - '0';
                        if (leaveout < 1 || leaveout > 4) argc = 0;
                    }
                    break;
                case 't':
                    if (arg.length() < 4 || arg.length() > 6 || arg.at(2) != ':')
                        argc = 0;
                    else
                    {
                        transpose = stoi(arg.substr(3));
                        if (transpose < 0 || transpose > 128) argc = 0;
                    }
                    break;
                }
            }
        }
        else
        {
            if (source_filename == "")
                source_filename = arg;
            else
                // Get rid of any specified extension and add our own proper
                destination_filename = arg.substr(0, arg.find_last_of(".")) + ".sf2";
        }
    }

    if (destination_filename == "")
        // Destination file uses the source name plus our own extension
        destination_filename = source_filename.substr(0, source_filename.find_last_of(".")) + ".sf2";

#ifndef DEBUG_CLASS
    if (argc < 2 || source_filename == "")
    {
        std::cout
            << "  Currently converts from: MOD, SNG, CT\n\n"
            << "Usage: SF2Converter [options] <source file> [destination file]\n\n"
            << "  /l:<number>    Leave out MOD channel <number> (1-4). Default is 4.\n"
            << "  /t:<number>    Halfstep transpose <number> added to MOD notes. Default is 0.\n";
        return 0;
    }
#endif // !DEBUG_CLASS

#ifdef DEBUG_ARGS
    std::cout
            << "leaveout:     " << leaveout << std::endl
            << "transpose:    " << transpose << std::endl
            << "source:       " << source_filename << std::endl
            << "destination:  " << destination_filename << std::endl;
        return 0;
#endif // DEBUG_ARGS

#ifdef _SF2_WINDOWS
        PlatformSDLWindows platform;
#elif _SF2_LINUX
        PlatformSDLLinux platform;
#else
        PlatformSDLMacOS platform;
#endif 

    Interface sf2(&platform);
    Misc output(&sf2, true);

    bool driver_loaded = sf2.LoadFile("drivers//sf2driver11_02.prg");
    if (!driver_loaded)
    {
        std::cerr << "ERROR: Driver failed to load." << std::endl;
        return 0;
    }

#ifdef DEBUG_CLASS
        // Unit testing
        Test test(&sf2, &output);
        test.TestAll();
        //test.TestSimpleAndSave();
        return 0;
#endif // DEBUG_CLASS

    SourceUtils source(&sf2, &output);
    source.LoadFile(source_filename);
    source.Convert(leaveout, transpose);
    source.SaveFile(destination_filename);

    return 0;
}
