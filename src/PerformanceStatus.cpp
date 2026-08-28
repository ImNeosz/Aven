#include "PerformanceStatus.h"

#include "WindowsMemoryMetrics.h"

#include <QTimer>

namespace {
QString formatRam(quint64 bytes)
{
    if (!bytes) return QStringLiteral("RAM —");
    constexpr double gib = 1024.0 * 1024.0 * 1024.0;
    if (bytes >= static_cast<quint64>(gib))
        return QStringLiteral("RAM %1 GB").arg(bytes / gib, 0, 'f', 1);
    return QStringLiteral("RAM %1 MB").arg(bytes / (1024 * 1024));
}
}

PerformanceStatus::PerformanceStatus(QObject *parent) : QObject(parent)
{
    auto *timer = new QTimer(this);
    timer->setInterval(2000);
    connect(timer, &QTimer::timeout, this, &PerformanceStatus::refresh);
    timer->start();
    refresh();
}

QString PerformanceStatus::currentRamText() const { return m_currentRamText; }
quint64 PerformanceStatus::currentRamBytes() const { return m_currentRamBytes; }
QString PerformanceStatus::releasedMemoryText() const { return QStringLiteral("0 MB saved"); }
QString PerformanceStatus::gamingModeStatus() const { return QStringLiteral("Gaming mode off"); }

void PerformanceStatus::refresh()
{
    const quint64 bytes = WindowsMemoryMetrics::avenProcessTreeWorkingSet();
    const QString text = formatRam(bytes);
    if (text == m_currentRamText && bytes == m_currentRamBytes) return;
    m_currentRamBytes = bytes;
    m_currentRamText = text;
    emit currentRamTextChanged();
}
