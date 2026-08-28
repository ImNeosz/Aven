#include "MemoryPolicy.h"

#include <algorithm>
#include <utility>

MemoryPolicy::MemoryPolicy(AdaptivePolicyConfig config) : m_config(std::move(config)) {}

MemoryPressure MemoryPolicy::pressureForLoad(int memoryLoadPercent) const
{
    if (memoryLoadPercent >= m_config.highPressurePercent) return MemoryPressure::High;
    if (memoryLoadPercent >= m_config.mediumPressurePercent) return MemoryPressure::Medium;
    return MemoryPressure::Low;
}

LifecycleDecision MemoryPolicy::automaticDecision(const TabLifecycleContext &tab,
                                                    MemoryPressure pressure) const
{
    if (isProtected(tab) || tab.state == PolicyTabState::Discarded) return LifecycleDecision::Keep;

    quint64 freezeAfter = m_config.lowFreezeSeconds;
    quint64 discardAfter = m_config.lowDiscardSeconds;
    if (pressure == MemoryPressure::Medium) {
        freezeAfter = m_config.mediumFreezeSeconds;
        discardAfter = m_config.mediumDiscardSeconds;
    } else if (pressure == MemoryPressure::High) {
        freezeAfter = m_config.highFreezeSeconds;
        discardAfter = m_config.highDiscardSeconds;
    }

    if (!tab.windowVisible) {
        freezeAfter = static_cast<quint64>(freezeAfter * m_config.hiddenWindowMultiplier);
        discardAfter = static_cast<quint64>(discardAfter * m_config.hiddenWindowMultiplier);
    }

    // Qt WebEngine requires Active -> Frozen -> Discarded. Never skip the
    // intermediate engine state, even under high pressure.
    if (tab.inactiveSeconds >= discardAfter && tab.state == PolicyTabState::Frozen) {
        return LifecycleDecision::Discard;
    }
    if (tab.state == PolicyTabState::Background && tab.inactiveSeconds >= freezeAfter) {
        return LifecycleDecision::Freeze;
    }
    return LifecycleDecision::Keep;
}

LifecycleDecision MemoryPolicy::releaseRamDecision(const TabLifecycleContext &tab) const
{
    if (isProtected(tab) || tab.state == PolicyTabState::Discarded) return LifecycleDecision::Keep;
    return LifecycleDecision::Discard;
}

const AdaptivePolicyConfig &MemoryPolicy::config() const { return m_config; }

bool MemoryPolicy::isProtected(const TabLifecycleContext &tab)
{
    return tab.state == PolicyTabState::Active || tab.isVisible || tab.isPlayingAudio ||
           tab.isPinned || tab.keepAlive;
}
