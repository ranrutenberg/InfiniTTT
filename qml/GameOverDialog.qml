import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    anchors.centerIn: Overlay.overlay
    width: Math.min(parent.width - 48, 300)
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.accent: Material.Blue

    property string winner: ""
    property bool   isDraw: false

    signal newGameRequested()

    background: Rectangle {
        radius: 12
        color: "#ffffff"
    }

    contentItem: ColumnLayout {
        spacing: 16

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.isDraw ? "🤝" : (root.winner === "X" ? "✕" : "○")
            font.pixelSize: 56
            color: root.isDraw ? "#555555"
                               : (root.winner === "X" ? "#dc3737" : "#3737dc")
        }

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            text: root.isDraw ? "Draw!" : "Player " + root.winner + " wins!"
            font { pixelSize: 22; bold: true }
        }

        Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            color: "#777777"
            font.pixelSize: 13
            text: root.isDraw ? "Maximum moves reached." : "5 in a row!"
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Button {
                Layout.fillWidth: true
                text: "Close"
                flat: true
                onClicked: root.close()
            }

            Button {
                Layout.fillWidth: true
                text: "New Game"
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: {
                    root.close()
                    root.newGameRequested()
                }
            }
        }
    }
}
