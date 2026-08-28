#include "UrlResolver.h"

#include <QTest>

class UrlResolverTests final : public QObject
{
    Q_OBJECT

private slots:
    void resolvesInput_data();
    void resolvesInput();
};

void UrlResolverTests::resolvesInput_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("expected");

    QTest::newRow("https URL") << "https://www.qt.io/path?q=1" << "https://www.qt.io/path?q=1";
    QTest::newRow("domain") << "youtube.com" << "https://youtube.com";
    QTest::newRow("domain path") << "github.com/openai" << "https://github.com/openai";
    QTest::newRow("domain port") << "example.test:8443/path" << "https://example.test:8443/path";
    QTest::newRow("localhost") << "localhost" << "https://localhost";
    QTest::newRow("localhost port") << "localhost:3000/app" << "https://localhost:3000/app";
    QTest::newRow("IPv4") << "127.0.0.1:8080" << "https://127.0.0.1:8080";
    QTest::newRow("IPv6") << "[::1]:8080" << "https://[::1]:8080";
    QTest::newRow("single word") << "youtube" << "https://www.google.com/search?q=youtube";
    QTest::newRow("phrase") << "warcraft logs" << "https://www.google.com/search?q=warcraft%20logs";
    QTest::newRow("long phrase") << "best gaming mouse" << "https://www.google.com/search?q=best%20gaming%20mouse";
}

void UrlResolverTests::resolvesInput()
{
    QFETCH(QString, input);
    QFETCH(QString, expected);
    QCOMPARE(UrlResolver::resolve(input, QStringLiteral("https://www.google.com/search?q=%1"))
                 .toString(QUrl::FullyEncoded),
             expected);
}

QTEST_GUILESS_MAIN(UrlResolverTests)
#include "UrlResolverTests.moc"
