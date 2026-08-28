#pragma once

#include <QObject>
#include <QString>

class AppSettings final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString defaultSearchUrl READ defaultSearchUrl WRITE setDefaultSearchUrl NOTIFY defaultSearchUrlChanged)
    Q_PROPERTY(QString searchEngineName READ searchEngineName WRITE setSearchEngineName NOTIFY searchEngineNameChanged)
    Q_PROPERTY(ThemeMode themeMode READ themeMode WRITE setThemeMode NOTIFY themeModeChanged)
    Q_PROPERTY(QString backgroundImage READ backgroundImage WRITE setBackgroundImage NOTIFY backgroundImageChanged)
    Q_PROPERTY(double backgroundStrength READ backgroundStrength WRITE setBackgroundStrength NOTIFY backgroundStrengthChanged)
    Q_PROPERTY(bool showClock READ showClock WRITE setShowClock NOTIFY showClockChanged)
    Q_PROPERTY(bool showDate READ showDate WRITE setShowDate NOTIFY showDateChanged)
    Q_PROPERTY(bool showCollections READ showCollections WRITE setShowCollections NOTIFY showCollectionsChanged)
    Q_PROPERTY(bool showPerformanceStatus READ showPerformanceStatus WRITE setShowPerformanceStatus NOTIFY showPerformanceStatusChanged)
    Q_PROPERTY(QString profileLabel READ profileLabel CONSTANT)
    Q_PROPERTY(PerformanceProfile performanceProfile READ performanceProfile WRITE setPerformanceProfile NOTIFY performanceProfileChanged)
    Q_PROPERTY(QString performanceProfileName READ performanceProfileName NOTIFY performanceProfileChanged)

public:
    enum ThemeMode { Ambient, Light, Dark };
    Q_ENUM(ThemeMode)
    enum PerformanceProfile { Adaptive, Balanced, Aggressive, Manual, Gaming };
    Q_ENUM(PerformanceProfile)

    explicit AppSettings(QObject *parent = nullptr);

    [[nodiscard]] QString defaultSearchUrl() const;
    void setDefaultSearchUrl(const QString &urlTemplate);
    [[nodiscard]] QString searchEngineName() const;
    void setSearchEngineName(const QString &name);
    [[nodiscard]] ThemeMode themeMode() const;
    void setThemeMode(ThemeMode mode);
    [[nodiscard]] QString backgroundImage() const;
    void setBackgroundImage(const QString &path);
    [[nodiscard]] double backgroundStrength() const;
    void setBackgroundStrength(double strength);
    [[nodiscard]] bool showClock() const;
    void setShowClock(bool visible);
    [[nodiscard]] bool showDate() const;
    void setShowDate(bool visible);
    [[nodiscard]] bool showCollections() const;
    void setShowCollections(bool visible);
    [[nodiscard]] bool showPerformanceStatus() const;
    void setShowPerformanceStatus(bool visible);
    [[nodiscard]] QString profileLabel() const;
    [[nodiscard]] PerformanceProfile performanceProfile() const;
    void setPerformanceProfile(PerformanceProfile profile);
    [[nodiscard]] QString performanceProfileName() const;

signals:
    void defaultSearchUrlChanged();
    void searchEngineNameChanged();
    void themeModeChanged();
    void backgroundImageChanged();
    void backgroundStrengthChanged();
    void showClockChanged();
    void showDateChanged();
    void showCollectionsChanged();
    void showPerformanceStatusChanged();
    void performanceProfileChanged();

private:
    QString m_defaultSearchUrl;
    QString m_searchEngineName;
    ThemeMode m_themeMode = Ambient;
    QString m_backgroundImage;
    double m_backgroundStrength = 0.30;
    bool m_showClock = true;
    bool m_showDate = true;
    bool m_showCollections = true;
    bool m_showPerformanceStatus = true;
    PerformanceProfile m_performanceProfile = Adaptive;
};
