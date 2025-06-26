#include "Utils/PerformanceProfiler.hpp"
#include <algorithm>
#include <numeric>
#include <spdlog/spdlog.h>

namespace Core {

PerformanceProfiler::PerformanceProfiler()
    : frameTimes_(FRAME_TIME_BUFFER_SIZE, 0.0f), frameTimeIndex_(0),
      frameStartTime_(std::chrono::steady_clock::now()), currentFPS_(0.0f),
      averageFPS_(0.0f) {}

PerformanceProfiler::~PerformanceProfiler() = default;

void PerformanceProfiler::BeginFrame() {
  frameStartTime_ = std::chrono::steady_clock::now();
}

void PerformanceProfiler::EndFrame() {
  auto frameEndTime = std::chrono::steady_clock::now();
  auto frameDuration = std::chrono::duration_cast<std::chrono::duration<float>>(
                           frameEndTime - frameStartTime_)
                           .count();

  LogFrameTime(frameDuration);
}

void PerformanceProfiler::BeginSection(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);

  activeSections_[name] = SectionTimer{std::chrono::steady_clock::now(), true};
}

void PerformanceProfiler::EndSection(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = activeSections_.find(name);
  if (it == activeSections_.end() || !it->second.active) {
    spdlog::warn("Ending section '{}' that was not started", name);
    return;
  }

  auto endTime = std::chrono::steady_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(
                      endTime - it->second.startTime)
                      .count();

  auto &data = sectionData_[name];
  data.minTime = std::min(data.minTime, duration);
  data.maxTime = std::max(data.maxTime, duration);
  data.averageTime =
      (data.averageTime * data.sampleCount + duration) / (data.sampleCount + 1);
  data.sampleCount++;

  it->second.active = false;
}

void PerformanceProfiler::LogFrameTime(float deltaTime) {
  frameTimes_[frameTimeIndex_] = deltaTime;
  frameTimeIndex_ = (frameTimeIndex_ + 1) % FRAME_TIME_BUFFER_SIZE;

  currentFPS_ = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;

  float sum = std::accumulate(frameTimes_.begin(), frameTimes_.end(), 0.0f);
  float avgFrameTime = sum / FRAME_TIME_BUFFER_SIZE;
  averageFPS_ = avgFrameTime > 0.0f ? 1.0f / avgFrameTime : 0.0f;
}

void PerformanceProfiler::LogMemoryUsage(std::size_t bytes) {
  currentMemoryUsage_ = bytes;

  std::size_t expected = peakMemoryUsage_.load();
  while (expected < bytes &&
         !peakMemoryUsage_.compare_exchange_weak(expected, bytes)) {
    // Retry until successful
  }
}

float PerformanceProfiler::GetAverageFPS() const { return averageFPS_; }

float PerformanceProfiler::GetCurrentFPS() const { return currentFPS_; }

PerformanceProfiler::ProfileData
PerformanceProfiler::GetSectionData(const std::string &name) const {
  std::lock_guard<std::mutex> lock(mutex_);

  auto it = sectionData_.find(name);
  if (it != sectionData_.end()) {
    return it->second;
  }

  return ProfileData{};
}

void PerformanceProfiler::GenerateReport() const {
  std::lock_guard<std::mutex> lock(mutex_);

  spdlog::info("=== Performance Report ===");
  spdlog::info("Average FPS: {:.1f}", averageFPS_);
  spdlog::info("Current FPS: {:.1f}", currentFPS_);
  spdlog::info("Memory Usage: {:.2f} MB (Peak: {:.2f} MB)",
               currentMemoryUsage_.load() / (1024.0 * 1024.0),
               peakMemoryUsage_.load() / (1024.0 * 1024.0));

  if (!sectionData_.empty()) {
    spdlog::info("--- Section Timings ---");
    for (const auto &[name, data] : sectionData_) {
      spdlog::info("{}: Avg={:.3f}ms, Min={:.3f}ms, Max={:.3f}ms ({}samples)",
                   name, data.averageTime * 1000.0, data.minTime * 1000.0,
                   data.maxTime * 1000.0, data.sampleCount);
    }
  }
}

void PerformanceProfiler::Reset() {
  std::lock_guard<std::mutex> lock(mutex_);

  sectionData_.clear();
  activeSections_.clear();
  std::fill(frameTimes_.begin(), frameTimes_.end(), 0.0f);
  frameTimeIndex_ = 0;
  currentFPS_ = 0.0f;
  averageFPS_ = 0.0f;
}

} // namespace Core