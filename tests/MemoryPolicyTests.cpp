#include "MemoryPolicy.h"

#include <QTest>

class MemoryPolicyTests final : public QObject
{
    Q_OBJECT

private slots:
    void classifiesPressure();
    void protectsImportantTabs();
    void adaptsToPressure();
    void prioritizesHiddenWindows();
    void releaseRamDiscardsOnlyEligibleTabs();
};

void MemoryPolicyTests::classifiesPressure()
{
    MemoryPolicy policy;
    QCOMPARE(policy.pressureForLoad(69), MemoryPressure::Low);
    QCOMPARE(policy.pressureForLoad(70), MemoryPressure::Medium);
    QCOMPARE(policy.pressureForLoad(85), MemoryPressure::High);
}

void MemoryPolicyTests::protectsImportantTabs()
{
    MemoryPolicy policy;
    TabLifecycleContext tab{PolicyTabState::Background, 100000};
    tab.isPlayingAudio = true;
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::High), LifecycleDecision::Keep);
    tab.isPlayingAudio = false;
    tab.keepAlive = true;
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::High), LifecycleDecision::Keep);
    tab.keepAlive = false;
    tab.isVisible = true;
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::High), LifecycleDecision::Keep);
}

void MemoryPolicyTests::adaptsToPressure()
{
    MemoryPolicy policy;
    TabLifecycleContext tab{PolicyTabState::Background, 11 * 60};
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::Low), LifecycleDecision::Keep);
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::Medium), LifecycleDecision::Freeze);
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::High), LifecycleDecision::Freeze);
    tab.state = PolicyTabState::Frozen;
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::High), LifecycleDecision::Discard);
}

void MemoryPolicyTests::prioritizesHiddenWindows()
{
    MemoryPolicy policy;
    TabLifecycleContext tab{PolicyTabState::Background, 5 * 60};
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::Medium), LifecycleDecision::Keep);
    tab.windowVisible = false;
    QCOMPARE(policy.automaticDecision(tab, MemoryPressure::Medium), LifecycleDecision::Freeze);
}

void MemoryPolicyTests::releaseRamDiscardsOnlyEligibleTabs()
{
    MemoryPolicy policy;
    TabLifecycleContext tab{PolicyTabState::Background, 1};
    QCOMPARE(policy.releaseRamDecision(tab), LifecycleDecision::Discard);
    tab.isPinned = true;
    QCOMPARE(policy.releaseRamDecision(tab), LifecycleDecision::Keep);
    tab.isPinned = false;
    tab.state = PolicyTabState::Active;
    QCOMPARE(policy.releaseRamDecision(tab), LifecycleDecision::Keep);
}

QTEST_GUILESS_MAIN(MemoryPolicyTests)
#include "MemoryPolicyTests.moc"
