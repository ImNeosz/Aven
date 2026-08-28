#include "AppSettings.h"

#include <QSettings>
#include <QtGlobal>

namespace {
constexpr auto kDefaultSearchUrl = "https://www.google.com/search?q=%1";
constexpr auto kSearchUrlKey = "search/defaultUrl";
constexpr auto kSearchEngineNameKey = "search/engineName";
constexpr auto kBackground = "qrc:/resources/aven-nordic-home.png";
}

AppSettings::AppSettings(QObject *parent)
    : QObject(parent)
{
    QSettings settings;
    m_defaultSearchUrl = settings.value(kSearchUrlKey, kDefaultSearchUrl).toString();
    m_searchEngineName = settings.value(kSearchEngineNameKey, QStringLiteral("Google")).toString().trimmed();
    if (m_searchEngineName.isEmpty()) m_searchEngineName = QStringLiteral("Google");
    if (!m_defaultSearchUrl.contains(QStringLiteral("%1"))) {
        m_defaultSearchUrl = QString::fromLatin1(kDefaultSearchUrl);
    }
    m_themeMode = static_cast<ThemeMode>(qBound(0, settings.value("appearance/theme", Ambient).toInt(), 2));
    m_backgroundImage = settings.value("appearance/backgroundImage", kBackground).toString();
    m_backgroundStrength = qBound(0.0, settings.value("appearance/backgroundStrength", 0.30).toDouble(), 1.0);
    m_showClock = settings.value("home/showClock", true).toBool();
    m_showDate = settings.value("home/showDate", true).toBool();
    m_showCollections = settings.value("home/showCollections", true).toBool();
    m_showPerformanceStatus = settings.value("home/showPerformanceStatus", true).toBool();
    m_performanceProfile = static_cast<PerformanceProfile>(
        qBound(0, settings.value("performance/profile", Adaptive).toInt(), 4));
}

QString AppSettings::searchEngineName() const { return m_searchEngineName; }

void AppSettings::setSearchEngineName(const QString &name)
{
    const QString normalized = name.trimmed();
    if (normalized.isEmpty() || normalized == m_searchEngineName) return;
    m_searchEngineName = normalized;
    QSettings().setValue(kSearchEngineNameKey, normalized);
    emit searchEngineNameChanged();
}

AppSettings::ThemeMode AppSettings::themeMode() const { return m_themeMode; }
void AppSettings::setThemeMode(ThemeMode value) { if (m_themeMode == value) return; m_themeMode = value; QSettings().setValue("appearance/theme", value); emit themeModeChanged(); }
QString AppSettings::backgroundImage() const { return m_backgroundImage; }
void AppSettings::setBackgroundImage(const QString &value) { if (m_backgroundImage == value) return; m_backgroundImage = value; QSettings().setValue("appearance/backgroundImage", value); emit backgroundImageChanged(); }
double AppSettings::backgroundStrength() const { return m_backgroundStrength; }
void AppSettings::setBackgroundStrength(double value) { value = qBound(0.0, value, 1.0); if (qFuzzyCompare(m_backgroundStrength, value)) return; m_backgroundStrength = value; QSettings().setValue("appearance/backgroundStrength", value); emit backgroundStrengthChanged(); }
bool AppSettings::showClock() const { return m_showClock; }
void AppSettings::setShowClock(bool value) { if (m_showClock == value) return; m_showClock = value; QSettings().setValue("home/showClock", value); emit showClockChanged(); }
bool AppSettings::showDate() const { return m_showDate; }
void AppSettings::setShowDate(bool value) { if (m_showDate == value) return; m_showDate = value; QSettings().setValue("home/showDate", value); emit showDateChanged(); }
bool AppSettings::showCollections() const { return m_showCollections; }
void AppSettings::setShowCollections(bool value) { if (m_showCollections == value) return; m_showCollections = value; QSettings().setValue("home/showCollections", value); emit showCollectionsChanged(); }
bool AppSettings::showPerformanceStatus() const { return m_showPerformanceStatus; }
void AppSettings::setShowPerformanceStatus(bool value) { if (m_showPerformanceStatus == value) return; m_showPerformanceStatus = value; QSettings().setValue("home/showPerformanceStatus", value); emit showPerformanceStatusChanged(); }
QString AppSettings::profileLabel() const { return QStringLiteral("Personal"); }
AppSettings::PerformanceProfile AppSettings::performanceProfile() const { return m_performanceProfile; }
void AppSettings::setPerformanceProfile(PerformanceProfile value)
{
    if (m_performanceProfile == value) return;
    m_performanceProfile = value;
    QSettings().setValue("performance/profile", value);
    emit performanceProfileChanged();
}
QString AppSettings::performanceProfileName() const
{
    switch (m_performanceProfile) {
    case Adaptive: return QStringLiteral("Adaptive");
    case Balanced: return QStringLiteral("Balanced");
    case Aggressive: return QStringLiteral("Aggressive");
    case Manual: return QStringLiteral("Manual");
    case Gaming: return QStringLiteral("Gaming");
    }
    return {};
}

QString AppSettings::defaultSearchUrl() const
{
    return m_defaultSearchUrl;
}

void AppSettings::setDefaultSearchUrl(const QString &urlTemplate)
{
    const QString normalized = urlTemplate.trimmed();
    if (normalized == m_defaultSearchUrl || !normalized.contains(QStringLiteral("%1"))) {
        return;
    }

    m_defaultSearchUrl = normalized;
    QSettings().setValue(kSearchUrlKey, m_defaultSearchUrl);
    emit defaultSearchUrlChanged();
}
