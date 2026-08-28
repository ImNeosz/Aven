import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: home
    signal navigateRequested(url destination)
    property date now: new Date()
    property int expandedCollection: -1

    readonly property var collections: [
        { name: "Social", links: [{name: "Reddit", url: "https://reddit.com"}, {name: "Discord", url: "https://discord.com/app"}] },
        { name: "Gaming", links: [{name: "Steam", url: "https://store.steampowered.com"}, {name: "Twitch", url: "https://twitch.tv"}] },
        { name: "Media", links: [{name: "YouTube", url: "https://youtube.com"}, {name: "Spotify", url: "https://open.spotify.com"}] },
        { name: "Good stuff", links: [{name: "GitHub", url: "https://github.com"}, {name: "Wikipedia", url: "https://wikipedia.org"}] }
    ]

    Image { anchors.fill: parent; source: appSettings.backgroundImage; fillMode: Image.PreserveAspectCrop; cache: true }
    Rectangle { anchors.fill: parent; color: appSettings.themeMode === 2 ? "#111614" : "#26302d"; opacity: appSettings.backgroundStrength }

    Timer { interval: 1000; running: home.visible; repeat: true; onTriggered: home.now = new Date() }

    ColumnLayout {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -22
        width: Math.min(parent.width - 48, 680)
        spacing: 8

        Label { visible: appSettings.showClock; Layout.alignment: Qt.AlignHCenter; text: Qt.formatTime(home.now, "HH:mm"); font.pixelSize: 76; font.weight: Font.Light; color: "#f7f7f2"; style: Text.Raised; styleColor: "#33000000" }
        Label { visible: appSettings.showDate; Layout.alignment: Qt.AlignHCenter; text: Qt.formatDate(home.now, "dddd, d MMMM"); font.pixelSize: 14; font.letterSpacing: 0.5; color: "#e9ebe6" }
        Item { Layout.preferredHeight: 28 }

        RowLayout {
            visible: appSettings.showCollections
            Layout.alignment: Qt.AlignHCenter
            spacing: 10
            Repeater {
                model: home.collections
                delegate: ColumnLayout {
                    id: collection
                    required property int index
                    required property var modelData
                    readonly property bool expanded: home.expandedCollection === collection.index
                    spacing: 7

                    Timer {
                        id: collapseTimer
                        interval: 110
                        onTriggered: {
                            if (home.expandedCollection === collection.index)
                                home.expandedCollection = -1
                        }
                    }

                    HoverHandler {
                        onHoveredChanged: {
                            if (hovered) {
                                collapseTimer.stop()
                                home.expandedCollection = collection.index
                            } else {
                                collapseTimer.restart()
                            }
                        }
                    }
                    Button {
                        id: collectionButton
                        Layout.alignment: Qt.AlignHCenter
                        text: collection.modelData.name + (collection.expanded ? "  −" : "  +")
                        font.pixelSize: 12
                        leftPadding: 13; rightPadding: 13
                        implicitHeight: 32
                        onClicked: home.expandedCollection = collection.index
                        contentItem: Text { text: collectionButton.text; font: collectionButton.font; color: "#f4f5f1"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                        background: Rectangle {
                            radius: 8
                            color: collectionButton.hovered ? "#35ffffff" : "#1fffffff"
                            border.width: 1
                            border.color: collectionButton.hovered ? "#4affffff" : "#35ffffff"

                            Behavior on color { ColorAnimation { duration: 220; easing.type: Easing.OutCubic } }
                            Behavior on border.color { ColorAnimation { duration: 220; easing.type: Easing.OutCubic } }
                        }
                    }

                    Item {
                        id: linksReveal
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredWidth: linksColumn.implicitWidth
                        Layout.preferredHeight: linksColumn.implicitHeight * revealProgress
                        opacity: revealProgress
                        clip: true

                        property real revealProgress: collection.expanded ? 1 : 0
                        Behavior on revealProgress {
                            NumberAnimation {
                                duration: collection.expanded ? 260 : 220
                                easing.type: collection.expanded ? Easing.OutCubic : Easing.InOutCubic
                            }
                        }

                        ColumnLayout {
                            id: linksColumn
                            anchors.top: parent.top
                            anchors.horizontalCenter: parent.horizontalCenter
                            spacing: 2
                            transform: Translate { y: 5 * (1 - linksReveal.revealProgress) }

                            Repeater {
                                model: collection.modelData.links
                                delegate: Button {
                                    id: linkButton
                                    required property var modelData
                                    Layout.alignment: Qt.AlignHCenter
                                    text: modelData.name
                                    flat: true
                                    font.pixelSize: 12
                                    contentItem: Text {
                                        text: linkButton.text
                                        font: linkButton.font
                                        color: linkButton.hovered ? "#ffffff" : "#dfe3dd"
                                        horizontalAlignment: Text.AlignHCenter
                                        verticalAlignment: Text.AlignVCenter

                                        Behavior on color { ColorAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    }
                                    background: Rectangle {
                                        radius: 6
                                        color: linkButton.hovered ? "#20ffffff" : "transparent"
                                        Behavior on color { ColorAnimation { duration: 180; easing.type: Easing.OutCubic } }
                                    }
                                    onClicked: home.navigateRequested(modelData.url)
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Popup {
        id: performancePanel
        width: Math.min(390, home.width - 32)
        x: (home.width - width) / 2
        y: home.height - height - 46
        padding: 18
        modal: false
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Rectangle { color: "#ed191d1b"; radius: 10; border.width: 1; border.color: "#35ffffff" }
        contentItem: ColumnLayout {
            spacing: 8
            Label { text: "Performance · " + memoryController.profileText; color: "#ffffff"; font.pixelSize: 14; font.weight: Font.Medium }
            Label { text: memoryController.pressureText; color: "#e1e5df"; font.pixelSize: 12 }
            Label { text: memoryController.systemMemoryText; color: "#cbd1ca"; font.pixelSize: 11 }
            Label { text: memoryController.lastReleaseSummary; color: "#cbd1ca"; font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Label { text: memoryController.lastPolicyAction; color: "#aeb7af"; font.pixelSize: 10; wrapMode: Text.WordWrap; Layout.fillWidth: true }
            Button {
                text: memoryController.releaseInProgress ? "Releasing…" : "Release RAM"
                enabled: !memoryController.releaseInProgress
                onClicked: memoryController.releaseRam()
            }
        }
    }

    Label {
        id: statusLine
        visible: appSettings.showPerformanceStatus
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16
        text: performanceStatus.currentRamText + "  ·  " + tabManager.activeTabCount
              + " active  ·  " + tabManager.sleepingTabCount + " sleeping"
        color: statusTap.hovered ? "#ffffff" : "#e1e5df"
        font.pixelSize: 11
        HoverHandler { id: statusTap }
        TapHandler { onTapped: performancePanel.open() }
    }
}
