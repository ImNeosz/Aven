#pragma once

#include "MemoryPolicy.h"

#include <QObject>
#include <QSet>
#include <QString>

class AppSettings;
class PerformanceStatus;
class TabManager;
class QTimer;

class MemoryController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString pressureText READ pressureText NOTIFY statusChanged)
    Q_PROPERTY(QString systemMemoryText READ systemMemoryText NOTIFY statusChanged)
    Q_PROPERTY(QString profileText READ profileText NOTIFY statusChanged)
    Q_PROPERTY(QString lastReleaseSummary READ lastReleaseSummary NOTIFY statusChanged)
    Q_PROPERTY(QString lastPolicyAction READ lastPolicyAction NOTIFY statusChanged)
    Q_PROPERTY(bool releaseInProgress READ releaseInProgress NOTIFY releaseInProgressChanged)

public:
    MemoryController(TabManager *tabs, AppSettings *settings,
                     PerformanceStatus *performanceStatus, QObject *parent = nullptr);

    [[nodiscard]] QString pressureText() const;
    [[nodiscard]] QString systemMemoryText() const;
    [[nodiscard]] QString profileText() const;
    [[nodiscard]] QString lastReleaseSummary() const;
    [[nodiscard]] QString lastPolicyAction() const;
    [[nodiscard]] bool releaseInProgress() const;

    Q_INVOKABLE void releaseRam();

signals:
    void statusChanged();
    void releaseInProgressChanged();

private:
    void evaluate();
    void finishRelease(quint64 beforeBytes, quint64 beforeAvailableBytes,
                       const QSet<quint32> &rendererIds, int affectedTabs);
    static QString formatBytes(quint64 bytes);

    TabManager *m_tabs;
    AppSettings *m_settings;
    PerformanceStatus *m_performanceStatus;
    MemoryPolicy m_policy;
    QString m_pressureText = QStringLiteral("Low pressure");
    QString m_systemMemoryText = QStringLiteral("System memory —");
    QString m_lastReleaseSummary = QStringLiteral("No manual cleanup yet");
    QString m_lastPolicyAction = QStringLiteral("No lifecycle actions yet");
    bool m_releaseInProgress = false;
};
