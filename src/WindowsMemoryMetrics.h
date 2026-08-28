#pragma once

#include <QSet>
#include <QtGlobal>

struct SystemMemorySnapshot {
    quint64 totalPhysicalBytes = 0;
    quint64 availablePhysicalBytes = 0;
    int loadPercent = 0;
};

class WindowsMemoryMetrics final
{
public:
    [[nodiscard]] static SystemMemorySnapshot systemMemory();
    [[nodiscard]] static quint64 processWorkingSet(quint32 processId);
    [[nodiscard]] static quint64 avenProcessTreeWorkingSet();
    [[nodiscard]] static QSet<quint32> avenProcessTreeIds();
    static int trimWorkingSets(const QSet<quint32> &processIds);
};
