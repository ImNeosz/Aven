# Aven Browser

Aven is a Windows-first, performance-first browser built with C++20, Qt 6, Qt Quick/QML, and Qt WebEngine. The product direction is a full Chromium-class browser comparable in capability to Chrome or Opera, with calmer UI, transparent diagnostics, and more intelligent resource management. This repository remains an early technical foundation rather than a production browser.

**Aven does not minimize resource usage. Aven minimizes resource waste.** Available RAM should improve browsing responsiveness; memory pressure, inactivity, and explicit user intent should reclaim waste without sacrificing the active experience.

## Prototype architecture

The first iteration has four boundaries:

- **Browser UI** — `qml/Main.qml` contains the compact window chrome, horizontal tab strip, navigation controls, and sole address/search field. `qml/HomeView.qml` is the local new-tab surface. QML is responsible for presentation and interaction, not persistence or URL policy.
- **Tab/session management** — `TabManager` owns lightweight tab identity, selection, metadata, and the requested Active/Background/Frozen/Discarded state. It never owns QML objects.
- **Web engine integration** — `qml/BrowserView.qml` maps requested tab state onto Qt WebEngine lifecycle state and reports title, URL, favicon, loading, renderer PID, and audible state back to the model.
- **Settings** — `AppSettings` owns persistent application preferences through `QSettings`: search URL, Ambient/Light/Dark mode, background source and strength, and Home visibility toggles. The default search URL is configurable as a template containing `%1`; the initial value is `https://www.google.com/search?q=%1`.
- **Omnibox policy** — `UrlResolver` classifies explicit URLs, domains, local hosts, IP addresses, and search phrases without depending on QML. Google is the named default engine and its URL remains a replaceable settings template, ready for a later engine list without adding settings UI now.
- **Adaptive memory policy** — `MemoryPolicy` is a deterministic, unit-tested policy with configurable thresholds. `MemoryController` samples Windows memory pressure, applies that policy to tab snapshots, performs manual Release RAM, and owns diagnostics. No resource policy is embedded in QML.
- **Windows metrics** — `WindowsMemoryMetrics` reads physical-memory load and working sets through Windows APIs. Renderer working set is an estimate because Chromium may share a renderer process between tabs.
- **Performance status** — `PerformanceStatus` samples Aven's complete Windows process tree at low frequency and publishes the compact aggregate RAM figure used by Home.

Home is native QML over an embedded Nordic background. `resources/start.html` remains the internal history URL, while Home itself performs no network request and adds no web UI framework.

## Visual identity

The primary Aven mark is an open circular horizon whose lower ribbon forms a restrained A-shaped wave. The production icon uses satin silver on charcoal and appears in the Windows executable, window chrome, taskbar, and local Home tab. The full concept sheet is retained under `design/`; product UI should use the mark sparingly rather than turning the browser into a branded dashboard.

## Current behavior and deliberate limits

The prototype creates one tab at startup. Every additional tab receives one `WebEngineView`, and closing the final tab immediately creates a fresh one. The omnibox recognizes explicit URLs, domains and paths, localhost, IP addresses, and ports; ordinary words and phrases are percent-encoded and sent to the configured search engine. Closed-tab restoration is intentionally memory-only and bounded to 20 entries.

Tab metadata is currently in-memory only. It includes last active time, visibility, audible state, pinned/keep-alive flags, renderer PID, estimated renderer working set, lifecycle state, and the reason for its last transition. Session persistence, downloads, permission UX, history, bookmarks, crash recovery, and production security policy are not implemented. Gaming behavior, Work Mode, tab groups, ad blocking, extensions, accounts, sync, and advanced privacy remain outside this milestone.

Interaction state is explicit rather than binding-driven. While the omnibox is being edited, redirects and URL notifications cannot replace user input; Escape cancels the edit, Enter commits it, and tab activation synchronizes directly from that tab's live WebEngine URL. WebEngine requests for user-initiated new tabs/windows are routed by destination, with ordinary `target="_blank"` and `window.open` behavior kept inside the main tab strip. An explicit context-menu command can create a separate single-page Aven window; full multi-window session and lifecycle ownership remains future architecture.

## Adaptive Memory Foundation

The implemented `Adaptive` profile classifies system memory load as low (below 70%), medium (70–84%), or high (85% and above). At low pressure, background tabs are frozen after 30 minutes and only discarded after 24 hours. Medium pressure uses 10 and 45 minutes; high pressure uses 1 and 10 minutes. A hidden/minimized window halves these thresholds. Every value is loaded from `QSettings` under `performance/adaptive/*` and can later be exposed by an advanced settings surface.

The current/visible tab is always Active. Audible, pinned, and explicit keep-alive tabs are protected from automatic and manual cleanup. Release RAM discards every other eligible background tab, waits for Chromium's transition, trims only affected renderer working sets, and records Aven process-tree working set plus system availability before and after. The prepared profile model also names Balanced, Aggressive, Manual, and Gaming, but only Adaptive currently runs an automatic policy.

## Long-term principles

### Performance is the highest priority

Features must earn their CPU time, memory, disk I/O, startup cost, and visual footprint. Measure before adding layers. Prefer direct Qt and platform facilities to dependencies whose cost or lifecycle Aven cannot control.

### Startup is lazy

Startup must never eagerly restore and load every tab from the previous session. A future session store should restore lightweight tab records first, create the selected web view on demand, and defer or avoid navigation for background tabs.

### Inactive tabs release wasted resources

Background tabs move through explicit Active, Background, Frozen, and Discarded states. State transitions are observable and conservative around visible pages, audio, pinned tabs, and user keep-alive intent. A discarded page reloads when selected again, matching Qt WebEngine lifecycle semantics.

### The interface is Scandinavian, calm, and minimal

The UI should use clear hierarchy, restrained color, generous space, and predictable motion. Minimal does not mean feature-poor; it means the default surface contains only what helps with the current task.

### Power stays below the surface

Advanced configuration may be extensive, but it must stay out of the default UI. Expert settings should be searchable and intentionally entered, without making everyday browsing feel like operating a control panel.

### Functionality appears in context

Browser functionality should appear when needed instead of constantly occupying screen space. Permissions, page actions, downloads, media controls, and specialized modes should surface in response to relevant state and recede cleanly afterward.

## Direction for the next architectural increment

Next, add lazy `WebEngineView` creation and a lightweight persistent session record. Restored background tabs should begin as unloaded/discarded records, so startup time and RAM scale with what the user actually opens rather than the size of their previous session.
