#include "TabManager.h"

#include "AppSettings.h"
#include "UrlResolver.h"
#include "WindowsMemoryMetrics.h"

#include <algorithm>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(avenTabs, "aven.tabs")
Q_LOGGING_CATEGORY(avenNavigation, "aven.navigation")

TabManager::TabManager(AppSettings *settings, QObject *parent)
    : QAbstractListModel(parent), m_settings(settings)
{
    newTab();
}

int TabManager::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(m_tabs.size());
}

QVariant TabManager::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tabs.size()) return {};
    const auto &tab = m_tabs.at(index.row());
    switch (role) {
    case TabIdRole: return QVariant::fromValue(tab.id);
    case TitleRole: return tab.title;
    case UrlRole: return tab.url;
    case IconRole: return tab.iconUrl;
    case LoadingRole: return tab.loading;
    case DomainRole: return tab.domain;
    case SleepingRole: return tab.lifecycleState == Frozen || tab.lifecycleState == Discarded;
    case LifecycleStateRole: return tab.lifecycleState;
    case LifecycleNameRole: return lifecycleName(tab.lifecycleState);
    case LastActiveRole: return tab.lastActive;
    case IsVisibleRole: return tab.isVisible;
    case IsPlayingAudioRole: return tab.isPlayingAudio;
    case IsPinnedRole: return tab.isPinned;
    case KeepAliveRole: return tab.keepAlive;
    case EstimatedMemoryUsageRole: return QVariant::fromValue(tab.estimatedMemoryUsage);
    case LifecycleReasonRole: return tab.lifecycleReason;
    default: return {};
    }
}

QHash<int, QByteArray> TabManager::roleNames() const
{
    return {
        {TabIdRole, "tabId"}, {TitleRole, "title"}, {UrlRole, "tabUrl"},
        {IconRole, "iconUrl"}, {LoadingRole, "loading"}, {DomainRole, "domain"},
        {SleepingRole, "sleeping"}, {LifecycleStateRole, "lifecycleState"},
        {LifecycleNameRole, "lifecycleName"}, {LastActiveRole, "lastActive"},
        {IsVisibleRole, "tabVisible"}, {IsPlayingAudioRole, "isPlayingAudio"},
        {IsPinnedRole, "isPinned"}, {KeepAliveRole, "keepAlive"},
        {EstimatedMemoryUsageRole, "estimatedMemoryUsage"},
        {LifecycleReasonRole, "lifecycleReason"}
    };
}

int TabManager::activeTabCount() const { return rowCount() - sleepingTabCount(); }
int TabManager::sleepingTabCount() const
{
    return static_cast<int>(std::count_if(m_tabs.cbegin(), m_tabs.cend(), [](const TabState &tab) {
        return tab.lifecycleState == Frozen || tab.lifecycleState == Discarded;
    }));
}
bool TabManager::canRestoreClosedTab() const { return !m_closedTabs.isEmpty(); }
bool TabManager::windowVisible() const { return m_windowVisible; }
int TabManager::currentIndex() const { return m_currentIndex; }

QVector<TabManager::LifecycleSnapshot> TabManager::lifecycleSnapshot() const
{
    QVector<LifecycleSnapshot> result;
    result.reserve(m_tabs.size());
    for (const auto &tab : m_tabs) {
        result.append({tab.id, tab.lifecycleState, tab.lastActive, tab.isVisible,
                       tab.isPlayingAudio, tab.isPinned, tab.keepAlive, m_windowVisible,
                       tab.estimatedMemoryUsage, tab.renderProcessId, tab.title});
    }
    return result;
}

void TabManager::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_tabs.size() || index == m_currentIndex) return;
    const int previous = m_currentIndex;
    if (previous >= 0 && previous < m_tabs.size()) {
        auto &oldTab = m_tabs[previous];
        oldTab.isVisible = false;
        if (oldTab.lifecycleState == Active) {
            oldTab.lifecycleState = Background;
            oldTab.lifecycleReason = QStringLiteral("Tab moved to background");
        }
        emitLifecycleDataChanged(previous);
    }

    m_currentIndex = index;
    auto &tab = m_tabs[index];
    tab.lifecycleState = Active;
    tab.isVisible = m_windowVisible;
    tab.lastActive = QDateTime::currentDateTimeUtc();
    tab.lifecycleReason = QStringLiteral("Selected by user");
    emitLifecycleDataChanged(index);
    emit currentIndexChanged();
    emit tabCountsChanged();
    emit lifecycleChanged(tab.id, tab.lifecycleState, tab.lifecycleReason);
    qCDebug(avenTabs).noquote() << "tab activated" << tab.id << "index" << index
                                << "url" << tab.url.toString();
}

void TabManager::newTab() { createTab(true); }
int TabManager::createTab(bool activate) { return createTabWithUrl(startPageUrl(), activate); }

int TabManager::createTabWithUrl(const QUrl &url, bool activate)
{
    TabState tab;
    tab.id = m_nextId++;
    tab.title = QStringLiteral("New tab");
    tab.url = url;
    tab.domain = url.host();
    tab.lastActive = QDateTime::currentDateTimeUtc();
    tab.lifecycleReason = QStringLiteral("Created in background");
    if (url == startPageUrl()) tab.iconUrl = QUrl(QStringLiteral("qrc:/resources/aven-app-icon.png"));

    const int index = m_tabs.size();
    beginInsertRows({}, index, index);
    m_tabs.append(std::move(tab));
    endInsertRows();
    emit countChanged();
    emit tabCountsChanged();
    if (activate) setCurrentIndex(index);
    qCDebug(avenTabs).noquote() << "tab created" << m_tabs.at(index).id << "index" << index
                                << "activate" << activate << "url" << url.toString();
    return index;
}

void TabManager::closeTab(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    const int previousCurrent = m_currentIndex;
    const quint64 closedId = m_tabs.at(index).id;
    const QUrl closedUrl = m_tabs.at(index).url;
    m_closedTabs.append(m_tabs.at(index));
    constexpr qsizetype kMaximumClosedTabs = 20;
    if (m_closedTabs.size() > kMaximumClosedTabs) m_closedTabs.removeFirst();
    emit canRestoreClosedTabChanged();

    beginRemoveRows({}, index, index);
    m_tabs.removeAt(index);
    endRemoveRows();
    emit countChanged();
    emit tabCountsChanged();

    if (m_tabs.isEmpty()) {
        m_currentIndex = -1;
        emit currentIndexChanged();
        newTab();
    } else if (index < previousCurrent) {
        m_currentIndex = previousCurrent - 1;
        emit currentIndexChanged();
    } else if (index == previousCurrent) {
        m_currentIndex = -1;
        setCurrentIndex(qMin(index, static_cast<int>(m_tabs.size()) - 1));
    }
    qCDebug(avenTabs).noquote() << "tab closed" << closedId << "former index" << index
                                << "url" << closedUrl.toString()
                                << "new active index" << m_currentIndex;
}

void TabManager::closeOtherTabs(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    setCurrentIndex(index);
    for (int row = static_cast<int>(m_tabs.size()) - 1; row >= 0; --row) {
        if (row != index) closeTab(row);
    }
}

void TabManager::closeTabsToRight(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    for (int row = static_cast<int>(m_tabs.size()) - 1; row > index; --row) closeTab(row);
}

void TabManager::closeTabsToLeft(int index)
{
    if (index <= 0 || index >= m_tabs.size()) return;
    for (int row = index - 1; row >= 0; --row) closeTab(row);
}

void TabManager::closeTabsFromDomain(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    const QString domain = m_tabs.at(index).domain;
    if (domain.isEmpty()) return;
    for (int row = static_cast<int>(m_tabs.size()) - 1; row >= 0; --row) {
        if (m_tabs.at(row).domain.compare(domain, Qt::CaseInsensitive) == 0)
            closeTab(row);
    }
}

void TabManager::duplicateTab(int index)
{
    if (index < 0 || index >= m_tabs.size()) return;
    TabState duplicate = m_tabs.at(index);
    duplicate.id = m_nextId++;
    duplicate.loading = false;
    duplicate.lifecycleState = Background;
    duplicate.isVisible = false;
    duplicate.isPlayingAudio = false;
    duplicate.lastActive = QDateTime::currentDateTimeUtc();
    duplicate.lifecycleReason = QStringLiteral("Duplicated in background");
    const int destination = index + 1;
    beginInsertRows({}, destination, destination);
    m_tabs.insert(destination, std::move(duplicate));
    endInsertRows();
    emit countChanged();
    emit tabCountsChanged();
    setCurrentIndex(destination);
}

void TabManager::restoreLastClosedTab()
{
    if (m_closedTabs.isEmpty()) return;
    TabState restored = m_closedTabs.takeLast();
    emit canRestoreClosedTabChanged();
    restored.id = m_nextId++;
    restored.loading = false;
    restored.lifecycleState = Background;
    restored.isVisible = false;
    restored.isPlayingAudio = false;
    restored.lastActive = QDateTime::currentDateTimeUtc();
    restored.lifecycleReason = QStringLiteral("Restored from closed tabs");
    const int index = m_tabs.size();
    beginInsertRows({}, index, index);
    m_tabs.append(std::move(restored));
    endInsertRows();
    emit countChanged();
    emit tabCountsChanged();
    setCurrentIndex(index);
}

void TabManager::selectRelativeTab(int offset)
{
    if (m_tabs.size() < 2 || offset == 0) return;
    const int count = static_cast<int>(m_tabs.size());
    setCurrentIndex((m_currentIndex + offset % count + count) % count);
}

void TabManager::updateTab(int index, const QString &title, const QUrl &url,
                           const QUrl &iconUrl, bool loading)
{
    if (index < 0 || index >= m_tabs.size()) return;
    auto &tab = m_tabs[index];
    const QString displayTitle = title.trimmed().isEmpty() ? QStringLiteral("New tab") : title.trimmed();
    if (tab.title == displayTitle && tab.url == url && tab.iconUrl == iconUrl && tab.loading == loading) return;
    const bool urlChanged = tab.url != url;
    tab.title = displayTitle;
    tab.url = url;
    tab.iconUrl = iconUrl;
    tab.loading = loading;
    tab.domain = url.host();
    emit dataChanged(this->index(index), this->index(index),
                     {TitleRole, UrlRole, IconRole, LoadingRole, DomainRole});
    if (urlChanged)
        qCDebug(avenNavigation).noquote() << "URL changed for tab" << tab.id
                                          << url.toString();
}

void TabManager::updateTabRuntime(int index, bool isPlayingAudio, qint64 renderProcessId)
{
    if (index < 0 || index >= m_tabs.size()) return;
    auto &tab = m_tabs[index];
    const quint32 pid = renderProcessId > 0 ? static_cast<quint32>(renderProcessId) : 0;
    bool changed = false;
    if (tab.isPlayingAudio != isPlayingAudio) { tab.isPlayingAudio = isPlayingAudio; changed = true; }
    if (tab.renderProcessId != pid) {
        tab.renderProcessId = pid;
        tab.estimatedMemoryUsage = pid ? WindowsMemoryMetrics::processWorkingSet(pid) : 0;
        changed = true;
    }
    if (changed) emit dataChanged(this->index(index), this->index(index),
                                  {IsPlayingAudioRole, EstimatedMemoryUsageRole});
}

void TabManager::setWindowVisible(bool visible)
{
    if (m_windowVisible == visible) return;
    m_windowVisible = visible;
    if (m_currentIndex >= 0) {
        m_tabs[m_currentIndex].isVisible = visible;
        emit dataChanged(index(m_currentIndex), index(m_currentIndex), {IsVisibleRole});
    }
    emit windowVisibleChanged();
}

void TabManager::setKeepAlive(int index, bool keepAlive)
{
    if (index < 0 || index >= m_tabs.size() || m_tabs[index].keepAlive == keepAlive) return;
    m_tabs[index].keepAlive = keepAlive;
    emit dataChanged(this->index(index), this->index(index), {KeepAliveRole});
}

void TabManager::setPinned(int index, bool pinned)
{
    if (index < 0 || index >= m_tabs.size() || m_tabs[index].isPinned == pinned) return;
    m_tabs[index].isPinned = pinned;
    emit dataChanged(this->index(index), this->index(index), {IsPinnedRole});
}

bool TabManager::setLifecycleStateById(quint64 id, LifecycleState state, const QString &reason)
{
    return setLifecycleStateAt(indexForId(id), state, reason);
}

bool TabManager::setLifecycleStateAt(int index, LifecycleState state, const QString &reason)
{
    if (index < 0 || index >= m_tabs.size()) return false;
    auto &tab = m_tabs[index];
    if ((state == Frozen || state == Discarded) &&
        (index == m_currentIndex || tab.isVisible || tab.isPlayingAudio || tab.isPinned || tab.keepAlive)) return false;
    if (tab.lifecycleState == Discarded && state == Frozen) return false;
    if (tab.lifecycleState == state) return false;
    tab.lifecycleState = state;
    tab.lifecycleReason = reason;
    emitLifecycleDataChanged(index);
    emit tabCountsChanged();
    emit lifecycleChanged(tab.id, state, reason);
    return true;
}

void TabManager::refreshEstimatedMemoryUsage()
{
    for (int row = 0; row < m_tabs.size(); ++row) {
        auto &tab = m_tabs[row];
        const quint64 bytes = tab.renderProcessId ? WindowsMemoryMetrics::processWorkingSet(tab.renderProcessId) : 0;
        if (bytes != tab.estimatedMemoryUsage) {
            tab.estimatedMemoryUsage = bytes;
            emit dataChanged(index(row), index(row), {EstimatedMemoryUsageRole});
        }
    }
}

QUrl TabManager::resolveInput(const QString &input) const
{
    const QUrl destination = UrlResolver::resolve(input, m_settings->defaultSearchUrl());
    qCDebug(avenNavigation).noquote() << "navigation requested" << input
                                      << "resolved to" << destination.toString();
    return destination;
}

QUrl TabManager::startPageUrl() { return QUrl(QStringLiteral("qrc:/resources/start.html")); }

QString TabManager::lifecycleName(LifecycleState state)
{
    switch (state) {
    case Active: return QStringLiteral("Active");
    case Background: return QStringLiteral("Background");
    case Frozen: return QStringLiteral("Frozen");
    case Discarded: return QStringLiteral("Discarded");
    }
    return QStringLiteral("Unknown");
}

int TabManager::indexForId(quint64 id) const
{
    for (int row = 0; row < m_tabs.size(); ++row) if (m_tabs.at(row).id == id) return row;
    return -1;
}

void TabManager::emitLifecycleDataChanged(int index)
{
    emit dataChanged(this->index(index), this->index(index),
                     {SleepingRole, LifecycleStateRole, LifecycleNameRole, LastActiveRole,
                      IsVisibleRole, LifecycleReasonRole});
}
