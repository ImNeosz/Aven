#pragma once

#include <QtGlobal>

enum class MemoryPressure { Low, Medium, High };
enum class PolicyTabState { Active, Background, Frozen, Discarded };
enum class LifecycleDecision { Keep, Freeze, Discard };

struct AdaptivePolicyConfig {
    quint64 lowFreezeSeconds = 30 * 60;
    quint64 lowDiscardSeconds = 24 * 60 * 60;
    quint64 mediumFreezeSeconds = 10 * 60;
    quint64 mediumDiscardSeconds = 45 * 60;
    quint64 highFreezeSeconds = 60;
    quint64 highDiscardSeconds = 10 * 60;
    int mediumPressurePercent = 70;
    int highPressurePercent = 85;
    double hiddenWindowMultiplier = 0.5;
};

struct TabLifecycleContext {
    PolicyTabState state = PolicyTabState::Background;
    quint64 inactiveSeconds = 0;
    bool isVisible = false;
    bool isPlayingAudio = false;
    bool isPinned = false;
    bool keepAlive = false;
    bool windowVisible = true;
};

class MemoryPolicy final
{
public:
    explicit MemoryPolicy(AdaptivePolicyConfig config = {});

    [[nodiscard]] MemoryPressure pressureForLoad(int memoryLoadPercent) const;
    [[nodiscard]] LifecycleDecision automaticDecision(const TabLifecycleContext &tab,
                                                       MemoryPressure pressure) const;
    [[nodiscard]] LifecycleDecision releaseRamDecision(const TabLifecycleContext &tab) const;
    [[nodiscard]] const AdaptivePolicyConfig &config() const;

private:
    [[nodiscard]] static bool isProtected(const TabLifecycleContext &tab);
    AdaptivePolicyConfig m_config;
};
