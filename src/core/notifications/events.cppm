export module sm.core.notifications.events;

import std;
import sm.core.notifications.types;

export namespace core::notifications::events {

struct NotificationRequestEvent {
  core::notifications::NotificationOptions options;
};

}  // namespace core::notifications::events
