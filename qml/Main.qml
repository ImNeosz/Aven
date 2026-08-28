import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: window
    width: 1280
    height: 800
    minimumWidth: 760
    minimumHeight: 520
    visible: true
    title: currentBrowser && currentBrowser.title ? currentBrowser.title + " — Aven" : "Aven"
    color: "#ecece9"

    readonly property color chrome: "#eeeeeb"
    readonly property color toolbar: "#f7f7f5"
    readonly property color ink: "#252725"
    readonly property color quietInk: "#727671"
    property var currentBrowser: null
    property bool omniboxEditing: false
    property var detachedWindows: []

    function displayUrl(browser) {
        return browser && !browser.isHome ? browser.url.toString() : ""
    }

    function syncOmnibox() {
        if (!omniboxEditing)
            addressField.text = displayUrl(currentBrowser)
    }

    function startOmniboxEditing(selectEverything) {
        if (!omniboxEditing) {
            omniboxEditing = true
            console.debug("Aven omnibox editing started")
        }
        addressField.forceActiveFocus()
        if (selectEverything)
            addressField.selectAll()
    }

    function stopOmniboxEditing(restoreCurrentUrl) {
        if (omniboxEditing)
            console.debug("Aven omnibox editing stopped")
        omniboxEditing = false
        if (restoreCurrentUrl)
            addressField.text = displayUrl(currentBrowser)
    }

    function refreshCurrentBrowser() {
        Qt.callLater(function() {
            currentBrowser = tabManager.currentIndex >= 0
                    ? browserRepeater.itemAt(tabManager.currentIndex) : null
            stopOmniboxEditing(true)
            if (currentBrowser && currentBrowser.isHome) {
                startOmniboxEditing(true)
            } else if (currentBrowser) {
                currentBrowser.focusPage()
            }
        })
    }

    function openWebWindow(request) {
        const background = request.destination === WebEngineNewWindowRequest.InNewBackgroundTab
        console.debug("Aven routing WebEngine request to tab; destination=" + request.destination
                      + ", background=" + background)
        const index = tabManager.createTab(!background)
        const browser = browserRepeater.itemAt(index)
        if (browser)
            browser.acceptWindowRequest(request)
        else
            console.warn("Aven could not create a WebEngineView for requested tab " + index)
    }

    function openUrlTab(destination, activate) {
        tabManager.createTabWithUrl(destination, activate)
    }

    function openExplicitWindow(destination) {
        const detached = detachedWindowComponent.createObject(null, { initialUrl: destination })
        if (!detached) {
            console.warn("Aven could not create an explicit browser window")
            return
        }
        detached.newTabRequested.connect(function(request) { window.openWebWindow(request) })
        detachedWindows = detachedWindows.concat([detached])
        detached.show()
    }

    function updateWindowVisibility() {
        tabManager.setWindowVisible(window.visible && window.visibility !== Window.Minimized
                                    && window.visibility !== Window.Hidden)
    }

    Component.onCompleted: { refreshCurrentBrowser(); updateWindowVisibility() }
    onVisibleChanged: updateWindowVisibility()
    onVisibilityChanged: updateWindowVisibility()
    Connections {
        target: tabManager
        function onCurrentIndexChanged() { window.refreshCurrentBrowser() }
    }
    Connections {
        target: window.currentBrowser
        function onUrlChanged() { window.syncOmnibox() }
        function onLoadingChanged() { if (!window.currentBrowser.loading) window.syncOmnibox() }
    }

    Component { id: detachedWindowComponent; DetachedWindow {} }

    Menu {
        id: tabContextMenu
        property int tabIndex: -1
        property bool tabKeepAlive: false
        property bool tabPinned: false
        property string tabDomain: ""

        MenuItem { text: "New tab"; onTriggered: tabManager.newTab() }
        MenuItem {
            text: "Reload"
            onTriggered: {
                const browser = browserRepeater.itemAt(tabContextMenu.tabIndex)
                if (browser) browser.reload()
            }
        }
        MenuItem { text: "Duplicate"; onTriggered: tabManager.duplicateTab(tabContextMenu.tabIndex) }
        MenuItem { text: "Keep tab active"; checkable: true; checked: tabContextMenu.tabKeepAlive; onTriggered: tabManager.setKeepAlive(tabContextMenu.tabIndex, checked) }
        MenuItem { text: "Pin tab"; checkable: true; checked: tabContextMenu.tabPinned; onTriggered: tabManager.setPinned(tabContextMenu.tabIndex, checked) }
        MenuSeparator {}
        MenuItem { text: "Close tab"; onTriggered: tabManager.closeTab(tabContextMenu.tabIndex) }
        MenuItem { text: "Close other tabs"; enabled: tabManager.count > 1; onTriggered: tabManager.closeOtherTabs(tabContextMenu.tabIndex) }
        MenuItem { text: "Close tabs to the left"; enabled: tabContextMenu.tabIndex > 0; onTriggered: tabManager.closeTabsToLeft(tabContextMenu.tabIndex) }
        MenuItem { text: "Close tabs to the right"; enabled: tabContextMenu.tabIndex >= 0 && tabContextMenu.tabIndex < tabManager.count - 1; onTriggered: tabManager.closeTabsToRight(tabContextMenu.tabIndex) }
        MenuItem {
            text: "Close all tabs from " + (tabContextMenu.tabDomain.length > 0 ? tabContextMenu.tabDomain : "this site")
            enabled: tabContextMenu.tabDomain.length > 0
            onTriggered: tabManager.closeTabsFromDomain(tabContextMenu.tabIndex)
        }
        MenuSeparator {}
        MenuItem { text: "Reopen closed tab"; enabled: tabManager.canRestoreClosedTab; onTriggered: tabManager.restoreLastClosedTab() }
    }

    Shortcut { sequence: "Ctrl+T"; onActivated: tabManager.newTab() }
    Shortcut { sequence: "Ctrl+W"; onActivated: tabManager.closeTab(tabManager.currentIndex) }
    Shortcut { sequence: "Ctrl+Shift+T"; onActivated: tabManager.restoreLastClosedTab() }
    Shortcut { sequence: "Ctrl+Tab"; onActivated: tabManager.selectRelativeTab(1) }
    Shortcut { sequence: "Ctrl+Shift+Tab"; onActivated: tabManager.selectRelativeTab(-1) }
    Shortcut { sequence: "Alt+Left"; onActivated: if (currentBrowser) currentBrowser.goBack() }
    Shortcut { sequence: "Alt+Right"; onActivated: if (currentBrowser) currentBrowser.goForward() }
    Shortcut { sequence: "Ctrl+R"; onActivated: if (currentBrowser) currentBrowser.reload() }
    Shortcut { sequence: "F5"; onActivated: if (currentBrowser) currentBrowser.reload() }
    Shortcut {
        sequence: "Ctrl+L"
        onActivated: window.startOmniboxEditing(true)
    }

    component QuietToolButton: ToolButton {
        id: control
        implicitWidth: 30
        implicitHeight: 30
        font.pixelSize: 18
        opacity: enabled ? (hovered ? 1 : 0.72) : 0.28
        background: Rectangle { radius: 7; color: control.hovered ? "#e6e7e3" : "transparent" }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 36
            color: window.chrome

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 8
                anchors.rightMargin: 10
                spacing: 6

                Flickable {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    contentWidth: tabRow.implicitWidth
                    contentHeight: height
                    clip: true

                    Row {
                        id: tabRow
                        height: parent.height
                        spacing: 3

                        Repeater {
                            model: tabManager
                            delegate: Rectangle {
                                id: tabDelegate
                                required property int index
                                required property string title
                                required property url iconUrl
                                required property bool loading
                                required property bool sleeping
                                required property bool keepAlive
                                required property bool isPinned
                                required property string domain
                                width: Math.max(112, Math.min(178, (window.width - 170) / Math.max(1, tabManager.count)))
                                height: 30
                                anchors.verticalCenter: parent.verticalCenter
                                radius: 7
                                color: index === tabManager.currentIndex ? "#fafaf8" : (tabMouse.hovered ? "#e7e7e4" : "transparent")

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.leftMargin: 10
                                    anchors.rightMargin: 4
                                    spacing: 6
                                    Item {
                                        Layout.preferredWidth: 14
                                        Layout.preferredHeight: 14
                                        Image { anchors.fill: parent; source: iconUrl; fillMode: Image.PreserveAspectFit; visible: iconUrl.toString().length > 0 && !loading }
                                        Rectangle { anchors.centerIn: parent; width: 6; height: 6; radius: 3; color: "#8a9c94"; visible: loading }
                                        Rectangle { anchors.right: parent.right; anchors.bottom: parent.bottom; width: 5; height: 5; radius: 3; color: "#8ea39a"; visible: sleeping }
                                    }
                                    Label { Layout.fillWidth: true; text: title; elide: Text.ElideRight; font.pixelSize: 12; color: index === tabManager.currentIndex ? window.ink : window.quietInk }
                                    ToolButton {
                                        Layout.preferredWidth: 24; Layout.preferredHeight: 24
                                        text: "×"; flat: true; font.pixelSize: 15
                                        opacity: index === tabManager.currentIndex || hovered ? 0.72 : 0
                                        onClicked: tabManager.closeTab(index)
                                        Accessible.name: "Close tab"
                                    }
                                }
                                HoverHandler { id: tabMouse }
                                TapHandler { acceptedButtons: Qt.LeftButton; onTapped: tabManager.currentIndex = index }
                                TapHandler { acceptedButtons: Qt.MiddleButton; onTapped: tabManager.closeTab(index) }
                                TapHandler {
                                    acceptedButtons: Qt.RightButton
                                    onTapped: {
                                        tabContextMenu.tabIndex = index
                                        tabContextMenu.tabKeepAlive = keepAlive
                                        tabContextMenu.tabPinned = isPinned
                                        tabContextMenu.tabDomain = domain
                                        tabContextMenu.popup()
                                    }
                                }
                            }
                        }

                        QuietToolButton { width: 30; height: 30; anchors.verticalCenter: parent.verticalCenter; text: "+"; onClicked: tabManager.newTab(); Accessible.name: "New tab" }
                    }
                }

                Label { text: appSettings.profileLabel; font.pixelSize: 11; color: window.quietInk; leftPadding: 8 }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 46
            color: window.toolbar

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 12
                spacing: 4

                QuietToolButton { text: "‹"; enabled: currentBrowser ? currentBrowser.canGoBack : false; onClicked: currentBrowser.goBack(); Accessible.name: "Back" }
                QuietToolButton { text: "›"; enabled: currentBrowser ? currentBrowser.canGoForward : false; onClicked: currentBrowser.goForward(); Accessible.name: "Forward" }
                QuietToolButton { text: currentBrowser && currentBrowser.loading ? "…" : "↻"; onClicked: if (currentBrowser) currentBrowser.reload(); Accessible.name: "Reload" }

                TextField {
                    id: addressField
                    Layout.fillWidth: true
                    Layout.maximumWidth: 820
                    Layout.alignment: Qt.AlignHCenter
                    implicitHeight: 32
                    leftPadding: 13; rightPadding: 13
                    placeholderText: "Search or enter address"
                    selectByMouse: true
                    font.pixelSize: 13
                    color: window.ink
                    background: Rectangle {
                        radius: 9
                        color: addressField.activeFocus ? "#ffffff" : "#ecece9"
                        border.width: addressField.activeFocus ? 1 : 0
                        border.color: "#c6c9c3"
                    }
                    onAccepted: {
                        const destination = tabManager.resolveInput(text)
                        if (currentBrowser) {
                            console.debug("Aven navigation requested: " + text + " -> " + destination)
                            stopOmniboxEditing(false)
                            currentBrowser.navigate(destination)
                            currentBrowser.focusPage()
                        }
                    }
                    Keys.onEscapePressed: event => {
                        stopOmniboxEditing(true)
                        if (currentBrowser) currentBrowser.focusPage()
                        event.accepted = true
                    }
                    onActiveFocusChanged: {
                        if (activeFocus) {
                            if (!omniboxEditing) {
                                omniboxEditing = true
                                console.debug("Aven omnibox editing started")
                            }
                        } else if (omniboxEditing) {
                            stopOmniboxEditing(true)
                        }
                    }
                }
                Item { Layout.preferredWidth: 30 }
            }
        }

        Rectangle { Layout.fillWidth: true; Layout.preferredHeight: 1; color: "#dedfdb" }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Repeater {
                id: browserRepeater
                model: tabManager
                onItemAdded: (index, item) => {
                    if (index === tabManager.currentIndex) window.refreshCurrentBrowser()
                }
                onItemRemoved: (index, item) => {
                    if (item === window.currentBrowser) window.currentBrowser = null
                    window.refreshCurrentBrowser()
                }
                BrowserView {
                    required property int index
                    required property url tabUrl
                    required property int lifecycleState
                    anchors.fill: parent
                    tabIndex: index
                    initialUrl: tabUrl
                    requestedLifecycleState: lifecycleState
                    visible: index === tabManager.currentIndex
                    onNewTabRequested: request => window.openWebWindow(request)
                    onUrlTabRequested: (destination, activate) => window.openUrlTab(destination, activate)
                    onNewWindowUrlRequested: destination => window.openExplicitWindow(destination)
                    onCloseRequested: tabManager.closeTab(index)
                }
            }
        }
    }
}
