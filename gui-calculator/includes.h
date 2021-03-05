#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <iostream>
#include <limits.h>

#include <string>

#include <dwmapi.h>
#pragma comment (lib, "dwmapi.lib")

#include <d3d9.h>
#pragma comment (lib, "d3d9.lib")

#include "DXWF/DXWF.h"
#pragma comment (lib, "DXWF/DXWF.lib")

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"