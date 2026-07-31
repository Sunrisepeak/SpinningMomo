module;

#include "core/notifications/types.hpp"

export module core.notifications.events;

import std;

export namespace core::notifications::events {

struct NotificationRequestEvent {
  core::notifications::NotificationOptions options;
};

}  // namespace core::notifications::events
