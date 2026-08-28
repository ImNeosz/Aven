#include "WindowsMemoryMetrics.h"

#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#endif

SystemMemorySnapshot WindowsMemoryMetrics::systemMemory()
{
#ifdef Q_OS_WIN
    MEMORYSTATUSEX status{sizeof(MEMORYSTATUSEX)};
    if (GlobalMemoryStatusEx(&status)) {
        return {status.ullTotalPhys, status.ullAvailPhys, static_cast<int>(status.dwMemoryLoad)};
    }
#endif
    return {};
}

quint64 WindowsMemoryMetrics::processWorkingSet(quint32 processId)
{
#ifdef Q_OS_WIN
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                                 FALSE, static_cast<DWORD>(processId));
    if (!process) return 0;
    PROCESS_MEMORY_COUNTERS counters{};
    const bool ok = GetProcessMemoryInfo(process, &counters, sizeof(counters));
    CloseHandle(process);
    return ok ? static_cast<quint64>(counters.WorkingSetSize) : 0;
#else
    Q_UNUSED(processId)
    return 0;
#endif
}

QSet<quint32> WindowsMemoryMetrics::avenProcessTreeIds()
{
    QSet<quint32> family{static_cast<quint32>(QCoreApplication::applicationPid())};
#ifdef Q_OS_WIN
    bool added = true;
    while (added) {
        added = false;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) break;
        PROCESSENTRY32 entry{sizeof(PROCESSENTRY32)};
        if (Process32First(snapshot, &entry)) {
            do {
                if (family.contains(entry.th32ParentProcessID) &&
                    !family.contains(entry.th32ProcessID)) {
                    family.insert(entry.th32ProcessID);
                    added = true;
                }
            } while (Process32Next(snapshot, &entry));
        }
        CloseHandle(snapshot);
    }
#endif
    return family;
}

quint64 WindowsMemoryMetrics::avenProcessTreeWorkingSet()
{
    quint64 total = 0;
    for (const quint32 processId : avenProcessTreeIds()) total += processWorkingSet(processId);
    return total;
}

int WindowsMemoryMetrics::trimWorkingSets(const QSet<quint32> &processIds)
{
    int trimmed = 0;
#ifdef Q_OS_WIN
    for (const quint32 processId : processIds) {
        HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_QUOTA,
                                     FALSE, static_cast<DWORD>(processId));
        if (!process) continue;
        if (EmptyWorkingSet(process)) ++trimmed;
        CloseHandle(process);
    }
#else
    Q_UNUSED(processIds)
#endif
    return trimmed;
}
