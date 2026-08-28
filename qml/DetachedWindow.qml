import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

ApplicationWindow {
    id: detached
    required property url initialUrl
    signal newTabRequested(var request)

    width: 1100
    height: 760
    minimumWidth: 640
    minimumHeight: 420
    title: view.title ? view.title + " — Aven" : "Aven"
    color: "#f7f7f5"

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            color: "#f7f7f5"
            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 12
                spacing: 5
                ToolButton { text: "‹"; enabled: view.canGoBack; onClicked: view.goBack() }
                ToolButton { text: "›"; enabled: view.canGoForward; onClicked: view.goForward() }
                ToolButton { text: view.loading ? "…" : "↻"; onClicked: view.reload() }
                TextField {
                    id: address
                    Layout.fillWidth: true
                    text: ""
                    selectByMouse: true
                    onAccepted: {
                        view.url = tabManager.resolveInput(text)
                        view.forceActiveFocus()
                    }
                }
            }
        }

        WebEngineView {
            id: view
            Layout.fillWidth: true
            Layout.fillHeight: true
            url: detached.initialUrl
            settings.javascriptCanOpenWindows: true
            Component.onCompleted: address.text = url.toString()
            onUrlChanged: if (!address.activeFocus) address.text = url.toString()
            onNewWindowRequested: request => detached.newTabRequested(request)
            onWindowCloseRequested: detached.close()
        }
    }
}
