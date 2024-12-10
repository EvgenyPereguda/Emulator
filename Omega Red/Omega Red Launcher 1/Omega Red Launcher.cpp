// Omega Red Launcher.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

using namespace System;
#using "../MediaSignStudio/bin/Release/MediaSignStudio.exe"

[System::STAThread] int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                                          _In_opt_ HINSTANCE hPrevInstance,
                                          _In_ LPWSTR lpCmdLine,
                                          _In_ int nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MediaSignStudio::Program::Main(nullptr);

    return 0;
}

// Nvidia OpenGL drivers >= v302 will check if the application exports a global
// variable named NvOptimusEnablement to know if it should run the app in high
// performance graphics mode or using the IGP.
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 1;
}

// Also on AMD PowerExpress: https://community.amd.com/thread/169965
extern "C" {
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
