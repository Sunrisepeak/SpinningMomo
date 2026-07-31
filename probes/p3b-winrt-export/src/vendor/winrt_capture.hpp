// Mirrors src/vendor/windows/winrt/windows_graphics_capture.hpp verbatim in
// shape: a facade that pulls in windows.h then the WinRT projection header.
#pragma once

#include <windows.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Metadata.h>
#include <winrt/Windows.Graphics.Capture.h>
#include <winrt/Windows.Graphics.DirectX.h>
#include <winrt/Windows.Graphics.DirectX.Direct3D11.h>
