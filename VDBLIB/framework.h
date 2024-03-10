#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
//priyadsh: these interfere with std::min,max during build...
#undef min
#undef max
