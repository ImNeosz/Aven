#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QUrl>
#include <QVector>

class AppSettings;

class TabManager final : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int activeTabCount READ activeTabCount NOTIFY tabCountsChanged)
    Q_PROPERTY(int sleepingTabCount READ sleepingTabCount NOTIFY tabCountsChanged)
    Q_PROPERTY(bool canRestoreClosedTab READ canRestoreClosedTab NOTIFY canRestoreClosedTabChanged)
    Q_PROPERTY(bool windowVisible READ windowVisible NOTIFY windowVisibleChanged)

public:
    enum LifecycleState { Active, Background, Frozen, Discarded };
    Q_ENUM(LifecycleState)

    enum Roles {
        TabIdRole = Qt::UserRole + 1,
        TitleRole,
        UrlRole,
        IconRole,
        LoadingRole,
        DomainRole,
        SleepingRole,
        LifecycleStateRole,
        LifecycleNameRole,
        LastActiveRole,
        IsVisibleRole,
        IsPlayingAudioRole,
        IsPinnedRole,
        KeepAliveRole,
        EstimatedMemoryUsageRole,
        LifecycleReasonRole
    };

    struct LifecycleSnapshot {
        quint64 id = 0;
        LifecycleState state = Background;
        QDateTime lastActive;
        bool isVisible = false;
        bool isPlayingAudio = false;
        bool isPinned = false;
        bool keepAlive = false;
        bool windowVisible = true;
        quint64 estimatedMemoryUsage = 0;
        quint32 renderProcessId = 0;
        QString title;
    };

    explicit TabManager(AppSettings *settings, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

    [[nodiscard]] int currentIndex() const;
    [[nodiscard]] int activeTabCount() const;
    [[nodiscard]] int sleepingTabCount() const;
    [[nodiscard]] bool canRestoreClosedTab() const;
    [[nodiscard]] bool windowVisible() const;
    [[nodiscard]] QVector<LifecycleSnapshot> lifecycleSnapshot() const;
    void setCurrentIndex(int index);
    bool setLifecycleStateById(quint64 id, LifecycleState state, const QString &reason);
    void refreshEstimatedMemoryUsage();

    Q_INVOKABLE void newTab();
    Q_INVOKABLE int createTab(bool activate);
    Q_INVOKABLE int createTabWithUrl(const QUrl &url, bool activate);
    Q_INVOKABLE void closeTab(int index);
    Q_INVOKABLE void closeOtherTabs(int index);
    Q_INVOKABLE void closeTabsToRight(int index);
    Q_INVOKABLE void closeTabsToLeft(int index);
    Q_INVOKABLE void closeTabsFromDomain(int index);
    Q_INVOKABLE void duplicateTab(int index);
    Q_INVOKABLE void restoreLastClosedTab();
    Q_INVOKABLE void selectRelativeTab(int offset);
    Q_INVOKABLE void updateTab(int index, const QString &title, const QUrl &url,
                               const QUrl &iconUrl, bool loading);
    Q_INVOKABLE void updateTabRuntime(int index, bool isPlayingAudio, qint64 renderProcessId);
    Q_INVOKABLE void setWindowVisible(bool visible);
    Q_INVOKABLE void setKeepAlive(int index, bool keepAlive);
    Q_INVOKABLE void setPinned(int index, bool pinned);
    Q_INVOKABLE QUrl resolveInput(const QString &input) const;

signals:
    void currentIndexChanged();
    void countChanged();
    void tabCountsChanged();
    void canRestoreClosedTabChanged();
    void windowVisibleChanged();
    void lifecycleChanged(quint64 tabId, int state, const QString &reason);

private:
    struct TabState {
        quint64 id = 0;
        QString title;
        QUrl url;
        QUrl iconUrl;
        bool loading = false;
        QString domain;
        LifecycleState lifecycleState = Background;
        QDateTime lastActive;
        bool isVisible = false;
        bool isPlayingAudio = false;
        bool isPinned = false;
        bool keepAlive = false;
        quint64 estimatedMemoryUsage = 0;
        quint32 renderProcessId = 0;
        QString lifecycleReason;
    };

    static QUrl startPageUrl();
    static QString lifecycleName(LifecycleState state);
    int indexForId(quint64 id) const;
    bool setLifecycleStateAt(int index, LifecycleState state, const QString &reason);
    void emitLifecycleDataChanged(int index);

    AppSettings *m_settings;
    QVector<TabState> m_tabs;
    QVector<TabState> m_closedTabs;
    int m_currentIndex = -1;
    quint64 m_nextId = 1;
    bool m_windowVisible = true;
};
