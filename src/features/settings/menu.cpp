module sm.features.settings.menu;

import std;
import sm.core.state.app_state;

import sm.features.settings.menu_types;
import sm.features.settings.state;

namespace features::settings::menu {

auto get_ratios(const core::AppState& app_state) -> const std::vector<RatioPreset>& {
  static const std::vector<RatioPreset> kEmpty;
  if (!app_state.settings) {
    return kEmpty;
  }
  const auto& settings = *app_state.settings;
  return settings.computed.aspect_ratios;
}

auto get_resolutions(const core::AppState& app_state) -> const std::vector<ResolutionPreset>& {
  static const std::vector<ResolutionPreset> kEmpty;
  if (!app_state.settings) {
    return kEmpty;
  }
  const auto& settings = *app_state.settings;
  return settings.computed.resolutions;
}

}  // namespace features::settings::menu
