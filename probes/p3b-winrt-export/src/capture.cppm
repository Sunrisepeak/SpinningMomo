// P3b: THE critical unknown for the whole migration.
//
// C++/WinRT projection headers are template- and consteval-heavy and are only
// supported on MSVC / clang-cl. The question is whether they survive being
// pulled into a named module's global module fragment — because 16 files in
// this project use them and Windows.Graphics.Capture is the core screenshot
// and recording path.
//
// This probe models the REALISTIC migration shape: WinRT types are used INSIDE
// the module and never cross the module boundary (the exported surface is
// plain std types). Re-exporting winrt:: types is a strictly harder question
// and is deliberately not conflated with this one.
module;

#include "vendor/winrt_capture.hpp"

export module capture;

import std;

export namespace capture {

// Touches ApiInformation (Foundation.Metadata), the WGC feature gate the real
// code uses, and the DirectX pixel-format enum — three separate projection
// headers, all consumed inside the module purview.
auto probe() -> std::string {
  bool supported = false;
  try {
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    supported = winrt::Windows::Graphics::Capture::GraphicsCaptureSession::IsSupported();
  } catch (...) {
    // A headless runner may have no capture stack; compiling and linking is
    // the assertion, running is a bonus.
    return "winrt: compiled+linked, runtime unavailable";
  }

  const auto fmt = winrt::Windows::Graphics::DirectX::DirectXPixelFormat::B8G8R8A8UIntNormalized;
  return std::format("winrt: compiled+linked, WGC supported={} fmt={}",
                     supported, static_cast<int>(fmt));
}

}  // namespace capture
