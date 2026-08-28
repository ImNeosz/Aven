#pragma once

#include <QObject>
#include <QString>

class QTimer;

class PerformanceStatus final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentRamText READ currentRamText NOTIFY currentRamTextChanged)
    Q_PROPERTY(quint64 currentRamBytes READ currentRamBytes NOTIFY currentRamTextChanged)
    Q_PROPERTY(QString releasedMemoryText READ releasedMemoryText CONSTANT)
    Q_PROPERTY(QString gamingModeStatus READ gamingModeStatus CONSTANT)

public:
    explicit PerformanceStatus(QObject *parent = nullptr);
    [[nodiscard]] QString currentRamText() const;
    [[nodiscard]] quint64 currentRamBytes() const;
    [[nodiscard]] QString releasedMemoryText() const;
    [[nodiscard]] QString gamingModeStatus() const;

signals:
    void currentRamTextChanged();

public slots:
    void refresh();

private:
    QString m_currentRamText = QStringLiteral("RAM —");
    quint64 m_currentRamBytes = 0;
};
