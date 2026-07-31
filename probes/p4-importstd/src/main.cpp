import std;
import utils.throttle;

int main() {
  // Exercise both the variadic and the void specialisation across the module
  // boundary — if the explicit specialisations did not survive export, this
  // fails to compile rather than silently doing the wrong thing.
  auto s = utils::throttle::create<void>(std::chrono::milliseconds{50});
  int hits = 0;
  const bool first = utils::throttle::call(*s, [&] { ++hits; });
  const bool second = utils::throttle::call(*s, [&] { ++hits; });

  auto sa = utils::throttle::create<int>(std::chrono::milliseconds{50});
  int last = -1;
  utils::throttle::call(*sa, [&](int v) { last = v; }, 7);

  std::println("p4: first={} second={} hits={} last={}", first, second, hits, last);
  if (!first || second || hits != 1 || last != 7) {
    std::println("p4: FAIL — throttle semantics did not survive modularisation");
    return 1;
  }
  std::println("p4: real source modularised, import std ok");
  return 0;
}
