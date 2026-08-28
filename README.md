# Aven Browser prototype

Aven is a Windows-first full-browser foundation using C++20, Qt 6, QML, and Qt WebEngine. Version 0.2 adds adaptive tab lifecycle management and measurable manual memory cleanup. See [PROJECT.md](PROJECT.md) for the architecture and product principles.

Version 0.2.1 adds explicit omnibox edit-state synchronization, stable tab routing for WebEngine new-window requests, and interaction diagnostics. User-initiated `target="_blank"` and `window.open` requests are routed into the existing Aven tab strip; non-user-initiated popups remain blocked.

## Windows build prerequisites

Install the following x64 components:

1. **Windows 10 version 1809 or newer, or Windows 11.**
2. **Visual Studio 2022 Community (or higher edition)** with the **Desktop development with C++** workload. Include MSVC v143, C++ CMake tools for Windows, and a Windows 11 SDK. Qt WebEngine requires MSVC on Windows; do not select a MinGW Qt kit for this project.
3. **Qt 6.11.2, MSVC 2022 64-bit** (or a later compatible Qt 6 release), installed with the Qt Online Installer. The CMake project accepts Qt 6.5 or newer, but the commands below use the currently documented Qt 6.11.2 Windows kit. Under the chosen Qt version, select:
   - MSVC 2022 64-bit
   - Qt WebEngine
   - Qt Declarative (Qt Quick/QML; normally part of the base desktop package)
   - Qt WebChannel and Qt Positioning if the installer lists them as Qt WebEngine dependencies
4. **CMake 3.21 or newer.** The project minimum is 3.21. The CMake bundled with Visual Studio or Qt is suitable when it meets that version.
5. **Ninja**, recommended for the commands below. It is supplied by many Visual Studio and Qt installations. A Visual Studio CMake generator can be used instead.

The extra Node.js, Python, gperf, bison, and flex requirements in Qt's documentation apply when building the Qt WebEngine module itself from source. They are not needed when building Aven against Qt's prebuilt MSVC package.

## Configure and build

Open **x64 Native Tools Command Prompt for VS 2022**, change to this repository, and run the following. Replace `6.11.2` with the installed Qt version if necessary.

```bat
C:\Qt\6.11.2\msvc2022_64\bin\qt-cmake.bat -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

If Ninja is unavailable, use the Visual Studio generator:

```bat
C:\Qt\6.11.2\msvc2022_64\bin\qt-cmake.bat -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

## Run

For the Ninja build:

```bat
build\Aven.exe
```

For the Visual Studio generator build:

```bat
build\Release\Aven.exe
```

Running from the developer machine uses the Qt installation located during configuration. To create a relocatable folder for another Windows machine, first build Release and then run `windeployqt` against the executable; Qt WebEngine needs its helper process, resources, QML imports, and DLLs alongside the application.

## Search configuration

`AppSettings` persists the selected engine name and search URL template via `QSettings`. The current engine is `Google`. The template must contain `%1`, which is replaced with the percent-encoded query. Its default is:

```text
https://www.google.com/search?q=%1
```

There is deliberately no settings UI in this prototype. A later advanced settings surface can expose this property without adding another preference mechanism.

## Adaptive memory controls

The Home status line shows Aven's total process-tree working set and active/sleeping tab counts. Click it for current system pressure, memory availability, lifecycle diagnostics, and **Release RAM**. Tabs can be protected with **Keep tab active** or **Pin tab** in the tab context menu. Adaptive thresholds are persisted under the `performance/adaptive/*` `QSettings` keys; their defaults and lifecycle contract are documented in `PROJECT.md`.

## Tests

After building, run the URL/omnibox tests with:

```bat
ctest --test-dir build -C Release --output-on-failure
```

Debug builds log tab creation, activation and closure, navigation/URL changes,
omnibox editing transitions, and WebEngine new-window requests. Logging categories
include `aven.tabs` and `aven.navigation`.

## Useful shortcuts

- `Ctrl+T` — new tab
- `Ctrl+W` — close current tab
- `Ctrl+Shift+T` — restore the last closed tab
- `Ctrl+Tab` / `Ctrl+Shift+Tab` — next/previous tab
- `Ctrl+L` — focus the address/search bar
- `Escape` — leave the address bar and return focus to the page
- `Alt+Left` / `Alt+Right` — back/forward
- `Ctrl+R` or `F5` — reload

## References

- [Qt for Windows](https://doc.qt.io/qt-6/windows.html)
- [Building Qt projects with CMake on the command line](https://doc.qt.io/qt-6/cmake-build-on-cmdline.html)
- [Qt WebEngine platform notes](https://doc.qt.io/qt-6/qtwebengine-platform-notes.html)
- [Deploying Qt WebEngine applications](https://doc.qt.io/qt-6/qtwebengine-deploying.html)
