#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>

namespace Core {

class PerformanceProfiler {
public:
    struct ProfileData {
        double averageTime = 0.0;
        double minTime = std::numeric_limits<double>::max();
        double maxTime = 0.0;
        std::size_t sampleCount = 0;
    };
    
    PerformanceProfiler();
    ~PerformanceProfiler();
    
    void BeginFrame();
    void EndFrame();
    
    void BeginSection(const std::string& name);
    void EndSection(const std::string& name);
    
    void LogFrameTime(float deltaTime);
    void LogMemoryUsage(std::size_t bytes);
    
    [[nodiscard]] float GetAverageFPS() const;
    [[nodiscard]] float GetCurrentFPS() const;
    [[nodiscard]] ProfileData GetSectionData(const std::string& name) const;
    
    void GenerateReport() const;
    void Reset();
    
private:
    struct SectionTimer {
        std::chrono::steady_clock::time_point startTime;
        bool active = false;
    };
    
    mutable std::mutex mutex_;
    
    std::unordered_map<std::string, SectionTimer> activeSections_;
    std::unordered_map<std::string, ProfileData> sectionData_;
    
    std::vector<float> frameTimes_;
    std::size_t frameTimeIndex_ = 0;
    static constexpr std::size_t FRAME_TIME_BUFFER_SIZE = 120;
    
    std::chrono::steady_clock::time_point frameStartTime_;
    std::atomic<std::size_t> currentMemoryUsage_{0};
    std::atomic<std::size_t> peakMemoryUsage_{0};
    
    float currentFPS_ = 0.0f;
    float averageFPS_ = 0.0f;
};

class ScopedProfiler {
public:
    ScopedProfiler(PerformanceProfiler& profiler, const std::string& sectionName)
        : profiler_(profiler), sectionName_(sectionName) {
        profiler_.BeginSection(sectionName_);
    }
    
    ~ScopedProfiler() {
        profiler_.EndSection(sectionName_);
    }
    
private:
    PerformanceProfiler& profiler_;
    std::string sectionName_;
};

#define PROFILE_SCOPE(profiler, name) Core::ScopedProfiler _scoped_profiler_##__LINE__(profiler, name)

} // namespace Core