export module sm.features.gallery.importer.importer;

import std;
import sm.core.state.app_state;
import sm.features.gallery.types;

export namespace features::gallery::importer {

// 将外部普通媒体文件复制到指定图库文件夹，并同步建立索引。
auto import_files_to_folder(core::AppState& app_state, std::int64_t folder_id,
                            const std::vector<std::filesystem::path>& source_paths)
    -> std::expected<OperationResult, std::string>;

}  // namespace features::gallery::importer
