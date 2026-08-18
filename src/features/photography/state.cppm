export module sm.features.photography.state;

import std;

export namespace features::photography {

struct PhotographyState {
  std::atomic<bool> enabled{false};
  // Demo 阶段按采样帧数控制长曝光；0 表示关闭。
  std::atomic<int> shutter_frames{0};
};

}  // namespace features::photography
