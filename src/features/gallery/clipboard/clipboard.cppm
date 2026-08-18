export module sm.features.gallery.clipboard.clipboard;

import std;
import sm.core.state.app_state;
import sm.features.gallery.types;

export namespace features::gallery::clipboard {

// 将选中资产解析为真实文件并写入系统剪贴板。
auto copy_assets(core::AppState& app_state, const std::vector<std::int64_t>& ids)
    -> std::expected<OperationResult, std::string>;

// 将剪贴板文件或截图无覆盖地写入指定图库目录，再同步建立资产索引。
auto paste_to_folder(core::AppState& app_state, std::int64_t folder_id)
    -> std::expected<OperationResult, std::string>;

}  // namespace features::gallery::clipboard
