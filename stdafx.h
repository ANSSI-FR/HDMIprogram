#pragma once

// Si vous incluez SDKDDKVer.h, cela définit la dernière plateforme Windows disponible.
// Si vous souhaitez générer votre application pour une plateforme Windows précédente, incluez WinSDKVer.h et
// définissez la macro _WIN32_WINNT à la plateforme que vous souhaitez prendre en charge avant d'inclure SDKDDKVer.h.
#define WINVER _WIN32_WINNT_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#include <SDKDDKVer.h>


#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN             // Exclure les en-têtes Windows rarement utilisés
// Fichiers d'en-tête Windows :
#include <windows.h>
#include <ShellScalingApi.h>
#include <Shlwapi.h>
#include <ShlObj.h>
// Fichiers d'en-tête C RunTime
#include <cstdint>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <math.h>
#include <locale.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")
#include "nfd.h"
#define __WINDOWS_DS__