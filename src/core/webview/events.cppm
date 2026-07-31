module;

export module core.webview.events;

import std;

export namespace core::webview::events {

// WebView响应事件
struct WebViewResponseEvent {
  std::string response;

  std::chrono::steady_clock::time_point timestamp = std::chrono::steady_clock::now();
};

}  // namespace core::webview::events
