export module sm.extensions.infinity_nikki.game_directory;

import std;
import sm.extensions.infinity_nikki.types;

export namespace extensions::infinity_nikki::game_directory {

auto get_game_directory() -> std::expected<InfinityNikkiGameDirResult, std::string>;

}  // namespace extensions::infinity_nikki::game_directory
