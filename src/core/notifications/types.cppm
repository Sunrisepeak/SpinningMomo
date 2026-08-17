export module sm.core.notifications.types;

import std;

export namespace core::notifications {

// A notification action closes over what it needs; it does NOT receive the
// application root.
//
// Taking `core::AppState&` here made this — a TYPES module in core — import the
// application root, and that closed a cycle the header world had hidden:
//
//   app_state -> ui.notification_window.state -> ui.notification_window.types
//             -> core.notifications.types -> app_state
//
// Modules do not allow it, and the rule they enforce is the right one: a core
// types module has no business depending on the composition root. Callers bind
// what they need at construction; the root outlives every notification (it is
// main's object), and the existing practice of capturing only STABLE keys and
// re-querying at click time is unchanged — that concern was about a watcher's
// lifetime, not AppState's.
using NotificationActionCallback = std::function<void()>;

struct NotificationAction {
  std::wstring label;
  NotificationActionCallback callback = nullptr;
};

struct NotificationOptions {
  std::wstring title;
  std::wstring message;
  std::optional<NotificationAction> action;
  std::chrono::milliseconds duration = std::chrono::milliseconds(3000);
};

}  // namespace core::notifications
