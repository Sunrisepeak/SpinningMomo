module;

#include "core/i18n/types.hpp"

export module core.i18n.state;

import std;

export namespace core::i18n {

struct I18nState {
  Language current_language = Language::EnUS;
  TextData texts;
  bool is_initialized = false;
};

}  // namespace core::i18n
