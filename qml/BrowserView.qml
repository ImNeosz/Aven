import QtQuick
import QtQuick.Controls
import QtWebEngine

Item {
    id: root
    required property int tabIndex
    required property url initialUrl
    required property int requestedLifecycleState
    signal newTabRequested(var request)
    signal urlTabRequested(url destination, bool activate)
    signal newWindowUrlRequested(url destination)
    signal closeRequested()

    property url contextLinkUrl
    property bool contextEditable: false
    property bool contextHasSelection: false
    property bool contextIsImage: false
    property url contextMediaUrl

    readonly property alias url: webView.url
    readonly property alias title: webView.title
    readonly property alias canGoBack: webView.canGoBack
    readonly property alias canGoForward: webView.canGoForward
    readonly property alias loading: webView.loading
    readonly property bool isHome: webView.url.toString() === "qrc:/resources/start.html"

    function navigate(destination) {
        if (destination.toString().length > 0) {
            console.debug("Aven WebEngine navigation requested for tab " + root.tabIndex + ": " + destination)
            webView.url = destination
        }
    }
    function goBack() { webView.goBack() }
    function goForward() { webView.goForward() }
    function reload() { webView.reload() }
    function focusPage() { webView.forceActiveFocus() }
    function acceptWindowRequest(request) { webView.acceptAsNewWindow(request) }
    function syncTab() {
        const tabIcon = root.isHome ? "qrc:/resources/aven-app-icon.png" : webView.icon
        tabManager.updateTab(root.tabIndex, root.isHome ? "New tab" : webView.title,
                             webView.url, tabIcon, webView.loading)
    }
    function applyLifecycle() {
        let desired = WebEngineView.LifecycleState.Active
        if (root.requestedLifecycleState === 2)
            desired = WebEngineView.LifecycleState.Frozen
        else if (root.requestedLifecycleState === 3)
            desired = WebEngineView.LifecycleState.Discarded
        if (webView.lifecycleState === WebEngineView.LifecycleState.Discarded
                && desired === WebEngineView.LifecycleState.Frozen)
            desired = WebEngineView.LifecycleState.Active
        if (webView.lifecycleState !== desired)
            webView.lifecycleState = desired
    }
    onRequestedLifecycleStateChanged: Qt.callLater(applyLifecycle)
    Component.onCompleted: Qt.callLater(applyLifecycle)

    Menu {
        id: pageContextMenu

        MenuItem {
            text: "Open link in new tab"
            visible: root.contextLinkUrl.toString().length > 0
            onTriggered: root.urlTabRequested(root.contextLinkUrl, true)
        }
        MenuItem {
            text: "Open link in new window"
            visible: root.contextLinkUrl.toString().length > 0
            onTriggered: root.newWindowUrlRequested(root.contextLinkUrl)
        }
        MenuItem {
            text: "Copy link address"
            visible: root.contextLinkUrl.toString().length > 0
            onTriggered: webView.triggerWebAction(WebEngineView.CopyLinkToClipboard)
        }
        MenuItem {
            text: "Open image in new tab"
            visible: root.contextIsImage && root.contextMediaUrl.toString().length > 0
            onTriggered: root.urlTabRequested(root.contextMediaUrl, false)
        }
        MenuItem {
            text: "Copy image"
            visible: root.contextIsImage
            onTriggered: webView.triggerWebAction(WebEngineView.CopyImageToClipboard)
        }
        MenuItem {
            text: "Copy image address"
            visible: root.contextIsImage
            onTriggered: webView.triggerWebAction(WebEngineView.CopyImageUrlToClipboard)
        }
        MenuSeparator { visible: root.contextLinkUrl.toString().length > 0 || root.contextIsImage }

        MenuItem { text: "Back"; enabled: webView.canGoBack; onTriggered: webView.goBack() }
        MenuItem { text: "Forward"; enabled: webView.canGoForward; onTriggered: webView.goForward() }
        MenuItem { text: "Reload"; onTriggered: webView.reload() }
        MenuSeparator {}

        MenuItem { text: "Undo"; visible: root.contextEditable; enabled: webView.action(WebEngineView.Undo).enabled; onTriggered: webView.triggerWebAction(WebEngineView.Undo) }
        MenuItem { text: "Redo"; visible: root.contextEditable; enabled: webView.action(WebEngineView.Redo).enabled; onTriggered: webView.triggerWebAction(WebEngineView.Redo) }
        MenuItem { text: "Cut"; visible: root.contextEditable; enabled: webView.action(WebEngineView.Cut).enabled; onTriggered: webView.triggerWebAction(WebEngineView.Cut) }
        MenuItem { text: "Copy"; enabled: root.contextHasSelection; onTriggered: webView.triggerWebAction(WebEngineView.Copy) }
        MenuItem { text: "Paste"; visible: root.contextEditable; enabled: webView.action(WebEngineView.Paste).enabled; onTriggered: webView.triggerWebAction(WebEngineView.Paste) }
        MenuItem { text: "Select all"; onTriggered: webView.triggerWebAction(WebEngineView.SelectAll) }
        MenuSeparator {}
        MenuItem { text: "View page source"; onTriggered: webView.triggerWebAction(WebEngineView.ViewSource) }
    }

    WebEngineView {
        id: webView
        anchors.fill: parent
        url: root.initialUrl
        visible: !root.isHome
        settings.javascriptCanOpenWindows: true
        onTitleChanged: root.syncTab()
        onUrlChanged: {
            console.debug("Aven URL changed for tab " + root.tabIndex + ": " + url)
            root.syncTab()
        }
        onIconChanged: root.syncTab()
        onLoadingChanged: root.syncTab()
        onRecentlyAudibleChanged: tabManager.updateTabRuntime(root.tabIndex, recentlyAudible, renderProcessPid)
        onRenderProcessPidChanged: tabManager.updateTabRuntime(root.tabIndex, recentlyAudible, renderProcessPid)
        onNewWindowRequested: request => {
            console.debug("Aven WebEngine request for new window/tab; destination="
                          + request.destination + ", URL=" + request.requestedUrl)
            if (request.userInitiated)
                root.newTabRequested(request)
            else
                console.warn("Aven blocked a non-user-initiated popup: " + request.requestedUrl)
        }
        onWindowCloseRequested: root.closeRequested()
        onContextMenuRequested: request => {
            request.accepted = true
            root.contextLinkUrl = request.linkUrl
            root.contextEditable = request.isContentEditable
            root.contextHasSelection = request.selectedText.length > 0
            root.contextIsImage = request.mediaType === ContextMenuRequest.MediaTypeImage
            root.contextMediaUrl = request.mediaUrl
            pageContextMenu.x = Math.max(0, Math.min(request.position.x, root.width - pageContextMenu.implicitWidth))
            pageContextMenu.y = Math.max(0, Math.min(request.position.y, root.height - pageContextMenu.implicitHeight))
            pageContextMenu.open()
        }
    }

    HomeView { anchors.fill: parent; visible: root.isHome; onNavigateRequested: destination => root.navigate(destination) }
}
