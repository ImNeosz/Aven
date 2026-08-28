#include "AppSettings.h"
#include "TabManager.h"

#include <QTest>

class TabManagerTests final : public QObject
{
    Q_OBJECT

private slots:
    void appendsAndActivatesNewTabs();
    void backgroundTabsDoNotStealFocus();
    void closingActiveTabSelectsNeighbor();
    void closesTabsToLeft();
    void closesAllTabsFromDomain();
};

void TabManagerTests::appendsAndActivatesNewTabs()
{
    AppSettings settings;
    TabManager tabs(&settings);
    QCOMPARE(tabs.rowCount(), 1);
    QCOMPARE(tabs.currentIndex(), 0);
    QCOMPARE(tabs.createTab(true), 1);
    QCOMPARE(tabs.createTab(true), 2);
    QCOMPARE(tabs.rowCount(), 3);
    QCOMPARE(tabs.currentIndex(), 2);
}

void TabManagerTests::backgroundTabsDoNotStealFocus()
{
    AppSettings settings;
    TabManager tabs(&settings);
    const int original = tabs.currentIndex();
    QCOMPARE(tabs.createTabWithUrl(QUrl("https://example.com"), false), 1);
    QCOMPARE(tabs.currentIndex(), original);
    QCOMPARE(tabs.rowCount(), 2);
}

void TabManagerTests::closingActiveTabSelectsNeighbor()
{
    AppSettings settings;
    TabManager tabs(&settings);
    tabs.createTabWithUrl(QUrl("https://one.example"), true);
    tabs.createTabWithUrl(QUrl("https://two.example"), true);
    QCOMPARE(tabs.currentIndex(), 2);
    tabs.closeTab(2);
    QCOMPARE(tabs.currentIndex(), 1);

    tabs.createTabWithUrl(QUrl("https://right.example"), true);
    tabs.setCurrentIndex(1);
    tabs.closeTab(1);
    QCOMPARE(tabs.currentIndex(), 1);
}

void TabManagerTests::closesTabsToLeft()
{
    AppSettings settings;
    TabManager tabs(&settings);
    tabs.createTabWithUrl(QUrl("https://one.example"), true);
    tabs.createTabWithUrl(QUrl("https://two.example"), true);
    tabs.createTabWithUrl(QUrl("https://three.example"), true);
    tabs.closeTabsToLeft(2);
    QCOMPARE(tabs.rowCount(), 2);
    QCOMPARE(tabs.currentIndex(), 1);
}

void TabManagerTests::closesAllTabsFromDomain()
{
    AppSettings settings;
    TabManager tabs(&settings);
    tabs.createTabWithUrl(QUrl("https://example.com/one"), true);
    tabs.createTabWithUrl(QUrl("https://other.example"), false);
    tabs.createTabWithUrl(QUrl("https://example.com/two"), false);
    tabs.closeTabsFromDomain(1);
    QCOMPARE(tabs.rowCount(), 2);
    QCOMPARE(tabs.currentIndex(), 1);
}

QTEST_GUILESS_MAIN(TabManagerTests)
#include "TabManagerTests.moc"
