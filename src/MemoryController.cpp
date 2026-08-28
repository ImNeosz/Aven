#include "MemoryController.h"

#include "AppSettings.h"
#include "PerformanceStatus.h"
#include "TabManager.h"
#include "WindowsMemoryMetrics.h"

#include <QDateTime>
#include <QDebug>
#include <QSettings>
#include <QTime>
#include <QTimer>

namespace {
AdaptivePolicyConfig loadConfig()
{
    QSettings settings;
    AdaptivePolicyConfig c;
    c.lowFreezeSeconds = settings.value("performance/adaptive/lowFreezeSeconds", c.lowFreezeSeconds).toULongLong();
    c.lowDiscardSeconds = settings.value("performance/adaptive/lowDiscardSeconds", c.lowDiscardSeconds).toULongLong();
    c.mediumFreezeSeconds = settings.value("performance/adaptive/mediumFreezeSeconds", c.mediumFreezeSeconds).toULongLong();
    c.mediumDiscardSeconds = settings.value("performance/adaptive/mediumDiscardSeconds", c.mediumDiscardSeconds).toULongLong();
    c.highFreezeSeconds = settings.value("performance/adaptive/highFreezeSeconds", c.highFreezeSeconds).toULongLong();
    c.highDiscardSeconds = settings.value("performance/adaptive/highDiscardSeconds", c.highDiscardSeconds).toULongLong();
    c.mediumPressurePercent = settings.value("performance/adaptive/mediumPressurePercent", c.mediumPressurePercent).toInt();
    c.highPressurePercent = settings.value("performance/adaptive/highPressurePercent", c.highPressurePercent).toInt();
    c.hiddenWindowMultiplier = settings.value("performance/adaptive/hiddenWindowMultiplier", c.hiddenWindowMultiplier).toDouble();
    return c;
}

PolicyTabState policyState(TabManager::LifecycleState state)
{
    switch (state) {
    case TabManager::Active: return PolicyTabState::Active;
    case TabManager::Background: return PolicyTabState::Background;
    case TabManager::Frozen: return PolicyTabState::Frozen;
    case TabManager::Discarded: return PolicyTabState::Discarded;
    }
    return PolicyTabState::Background;
}

TabLifecycleContext contextFor(const TabManager::LifecycleSnapshot &tab)
{
    const qint64 seconds = tab.lastActive.secsTo(QDateTime::currentDateTimeUtc());
    return {policyState(tab.state), static_cast<quint64>(qMax<qint64>(0, seconds)),
            tab.isVisible, tab.isPlayingAudio, tab.isPinned, tab.keepAlive, tab.windowVisible};
}
}

MemoryController::MemoryController(TabManager *tabs, AppSettings *settings,
                                   PerformanceStatus *performanceStatus, QObject *parent)
    : QObject(parent), m_tabs(tabs), m_settings(settings),
      m_performanceStatus(performanceStatus), m_policy(loadConfig())
{
    auto *timer = new QTimer(this);
    timer->setInterval(15000);
    connect(timer, &QTimer::timeout, this, &MemoryController::evaluate);
    timer->start();
    QTimer::singleShot(3000, this, &MemoryController::evaluate);
    connect(settings, &AppSettings::performanceProfileChanged, this, &MemoryController::statusChanged);
}

QString MemoryController::pressureText() const { return m_pressureText; }
QString MemoryController::systemMemoryText() const { return m_systemMemoryText; }
QString MemoryController::profileText() const { return m_settings->performanceProfileName(); }
QString MemoryController::lastReleaseSummary() const { return m_lastReleaseSummary; }
QString MemoryController::lastPolicyAction() const { return m_lastPolicyAction; }
bool MemoryController::releaseInProgress() const { return m_releaseInProgress; }

void MemoryController::evaluate()
{
    const auto system = WindowsMemoryMetrics::systemMemory();
    const MemoryPressure pressure = m_policy.pressureForLoad(system.loadPercent);
    const QString pressureName = pressure == MemoryPressure::High ? QStringLiteral("High pressure")
        : pressure == MemoryPressure::Medium ? QStringLiteral("Medium pressure") : QStringLiteral("Low pressure");
    m_pressureText = QStringLiteral("%1 (%2%)").arg(pressureName).arg(system.loadPercent);
    m_systemMemoryText = QStringLiteral("%1 available of %2")
        .arg(formatBytes(system.availablePhysicalBytes), formatBytes(system.totalPhysicalBytes));

    if (m_settings->performanceProfile() == AppSettings::Adaptive) {
        for (const auto &tab : m_tabs->lifecycleSnapshot()) {
            const auto decision = m_policy.automaticDecision(contextFor(tab), pressure);
            TabManager::LifecycleState target;
            if (decision == LifecycleDecision::Freeze) target = TabManager::Frozen;
            else if (decision == LifecycleDecision::Discard) target = TabManager::Discarded;
            else continue;
            const QString action = target == TabManager::Frozen ? QStringLiteral("froze") : QStringLiteral("discarded");
            const QString reason = QStringLiteral("Adaptive policy: %1 at %2% memory load").arg(action).arg(system.loadPercent);
            if (m_tabs->setLifecycleStateById(tab.id, target, reason)) {
                m_lastPolicyAction = QStringLiteral("%1: %2 %3").arg(QTime::currentTime().toString("HH:mm:ss"), action, tab.title);
                qDebug().noquote() << "Aven lifecycle" << tab.id << reason;
            }
        }
    }
    m_tabs->refreshEstimatedMemoryUsage();
    emit statusChanged();
}

void MemoryController::releaseRam()
{
    if (m_releaseInProgress) return;
    m_releaseInProgress = true;
    emit releaseInProgressChanged();
    const quint64 before = WindowsMemoryMetrics::avenProcessTreeWorkingSet();
    const quint64 beforeAvailable = WindowsMemoryMetrics::systemMemory().availablePhysicalBytes;
    QSet<quint32> affectedRenderers;
    QSet<quint32> protectedRenderers;
    QVector<quint64> eligibleTabs;
    int affectedTabs = 0;
    const auto snapshots = m_tabs->lifecycleSnapshot();
    for (const auto &tab : snapshots) {
        if (m_policy.releaseRamDecision(contextFor(tab)) == LifecycleDecision::Discard) continue;
        if (tab.renderProcessId) protectedRenderers.insert(tab.renderProcessId);
    }
    for (const auto &tab : snapshots) {
        if (m_policy.releaseRamDecision(contextFor(tab)) != LifecycleDecision::Discard) continue;
        eligibleTabs.append(tab.id);
        ++affectedTabs;
        if (tab.renderProcessId) affectedRenderers.insert(tab.renderProcessId);
        if (tab.state == TabManager::Background)
            m_tabs->setLifecycleStateById(tab.id, TabManager::Frozen,
                                          QStringLiteral("Release RAM: freeze before discard"));
    }
    affectedRenderers.subtract(protectedRenderers);
    QTimer::singleShot(500, this, [this, before, beforeAvailable, affectedRenderers, eligibleTabs, affectedTabs] {
        for (const quint64 tabId : eligibleTabs)
            m_tabs->setLifecycleStateById(tabId, TabManager::Discarded,
                                          QStringLiteral("Release RAM manual cleanup"));
        QTimer::singleShot(1300, this, [this, before, beforeAvailable, affectedRenderers, affectedTabs] {
            finishRelease(before, beforeAvailable, affectedRenderers, affectedTabs);
        });
    });
}

void MemoryController::finishRelease(quint64 beforeBytes, quint64 beforeAvailableBytes,
                                     const QSet<quint32> &rendererIds, int affectedTabs)
{
    const int trimmed = WindowsMemoryMetrics::trimWorkingSets(rendererIds);
    QTimer::singleShot(400, this, [this, beforeBytes, beforeAvailableBytes, affectedTabs, trimmed] {
        const quint64 after = WindowsMemoryMetrics::avenProcessTreeWorkingSet();
        const quint64 afterAvailable = WindowsMemoryMetrics::systemMemory().availablePhysicalBytes;
        const quint64 released = beforeBytes > after ? beforeBytes - after : 0;
        const qint64 systemDelta = static_cast<qint64>(afterAvailable) - static_cast<qint64>(beforeAvailableBytes);
        const QString deltaPrefix = systemDelta >= 0 ? QStringLiteral("+") : QStringLiteral("−");
        m_lastReleaseSummary = QStringLiteral("Released %1 · %2 tab(s) discarded · system available %3%4")
            .arg(formatBytes(released)).arg(affectedTabs, 0, 10)
            .arg(deltaPrefix, formatBytes(static_cast<quint64>(qAbs(systemDelta))));
        qInfo().noquote() << "Aven Release RAM: before" << formatBytes(beforeBytes)
                          << "after" << formatBytes(after) << "released" << formatBytes(released)
                          << "tabs" << affectedTabs << "trimmed renderers" << trimmed;
        m_releaseInProgress = false;
        m_performanceStatus->refresh();
        emit releaseInProgressChanged();
        emit statusChanged();
    });
}

QString MemoryController::formatBytes(quint64 bytes)
{
    constexpr double gib = 1024.0 * 1024.0 * 1024.0;
    if (bytes >= static_cast<quint64>(gib)) return QStringLiteral("%1 GB").arg(bytes / gib, 0, 'f', 1);
    return QStringLiteral("%1 MB").arg(bytes / (1024 * 1024));
}
