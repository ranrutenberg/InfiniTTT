import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import QtQuick.Layouts

Popup {
    id: root
    anchors.centerIn: Overlay.overlay
    width:  Math.min(parent.width  - 32, 380)
    height: Math.min(parent.height - 64, 560)
    modal: true
    closePolicy: Popup.NoAutoClose
    Material.accent: Material.Blue

    // Emitted when user taps Start Game
    signal gameStarted(bool p1Human, int p1AI, int p1Lvl,
                       bool p2Human, int p2AI, int p2Lvl)

    property bool gameActive: false

    // Persisted selections (survive dialog close/reopen)
    QtObject {
        id: saved
        property int p1Type:  0   // 0=Human, 1=AI
        property int p1AI:    3   // 0=SmartRandom, 1=Hybrid, 2=HybridV2, 3=HybridV3
        property int p1Level: 2
        property int p2Type:  1
        property int p2AI:    3
        property int p2Level: 2
    }

    background: Rectangle {
        radius: 8
        color: "#ffffff"
        layer.enabled: true
        layer.effect: null
    }

    contentItem: ColumnLayout {
        spacing: 0

        // Title
        Rectangle {
            Layout.fillWidth: true
            height: 52
            color: Material.accentColor
            radius: 8
            // flat bottom
            Rectangle { anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
                        height: 8; color: parent.color }

            Label {
                anchors.centerIn: parent
                text: "New Game"
                color: "white"
                font { pixelSize: 20; bold: true }
            }
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout {
                width: root.width - 32
                spacing: 8

                Item { height: 8 }

                PlayerPanel {
                    id: p1Panel
                    Layout.fillWidth: true
                    playerLabel: "Player 1  (✕)"
                    accentColor: "#dc3737"
                    typeIndex:  saved.p1Type
                    aiIndex:    saved.p1AI
                    levelIndex: saved.p1Level
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: "#e0e0e0" }

                PlayerPanel {
                    id: p2Panel
                    Layout.fillWidth: true
                    playerLabel: "Player 2  (○)"
                    accentColor: "#3737dc"
                    typeIndex:  saved.p2Type
                    aiIndex:    saved.p2AI
                    levelIndex: saved.p2Level
                }

                Item { height: 8 }
            }
        }

        // Start button
        Button {
            Layout.fillWidth: true
            Layout.margins: 12
            text: "Start Game"
            Material.background: Material.Blue
            Material.foreground: "white"
            font { pixelSize: 16; bold: true }
            height: 48
            onClicked: {
                saved.p1Type  = p1Panel.typeIndex
                saved.p1AI    = p1Panel.aiIndex
                saved.p1Level = p1Panel.levelIndex
                saved.p2Type  = p2Panel.typeIndex
                saved.p2AI    = p2Panel.aiIndex
                saved.p2Level = p2Panel.levelIndex
                root.close()
                root.gameStarted(
                    p1Panel.typeIndex === 0, p1Panel.aiIndex, p1Panel.levelIndex,
                    p2Panel.typeIndex === 0, p2Panel.aiIndex, p2Panel.levelIndex
                )
            }
        }

        Button {
            Layout.fillWidth: true
            Layout.leftMargin: 12; Layout.rightMargin: 12; Layout.bottomMargin: 12
            visible: root.gameActive
            text: "Cancel"
            flat: true
            onClicked: root.close()
        }
    }

    // ── Inline PlayerPanel component ─────────────────────────────────────────
    component PlayerPanel: ColumnLayout {
        required property string playerLabel
        required property color  accentColor
        property int typeIndex:  0
        property int aiIndex:    1
        property int levelIndex: 2

        spacing: 6
        Layout.leftMargin: 16; Layout.rightMargin: 16

        Label {
            text: playerLabel
            font { pixelSize: 15; bold: true }
            color: accentColor
        }

        RowLayout {
            Layout.fillWidth: true
            Label { text: "Type:"; font.pixelSize: 13; Layout.preferredWidth: 56 }
            ComboBox {
                id: typeCombo
                Layout.fillWidth: true
                model: ["Human", "AI"]
                currentIndex: parent.parent.typeIndex
                onCurrentIndexChanged: parent.parent.typeIndex = currentIndex
                font.pixelSize: 13
            }
        }

        RowLayout {
            Layout.fillWidth: true
            visible: typeCombo.currentIndex === 1   // AI selected
            Label { text: "AI:"; font.pixelSize: 13; Layout.preferredWidth: 56 }
            ComboBox {
                id: aiCombo
                Layout.fillWidth: true
                model: ["Smart Random", "Hybrid Evaluator", "Hybrid v2 (Minimax)", "Hybrid v3 (Open-4)"]
                currentIndex: parent.parent.aiIndex
                onCurrentIndexChanged: parent.parent.aiIndex = currentIndex
                font.pixelSize: 13
            }
        }

        RowLayout {
            Layout.fillWidth: true
            // Level only meaningful for Smart Random
            visible: typeCombo.currentIndex === 1 && aiCombo.currentIndex === 0
            Label { text: "Level:"; font.pixelSize: 13; Layout.preferredWidth: 56 }
            ComboBox {
                Layout.fillWidth: true
                model: [
                    "0 – Pure random",
                    "1 – Win detection",
                    "2 – Win + Block",
                    "3 – Win + Block + Double threat",
                    "4 – Full strategic"
                ]
                currentIndex: parent.parent.levelIndex
                onCurrentIndexChanged: parent.parent.levelIndex = currentIndex
                font.pixelSize: 13
            }
        }

        Item { height: 4 }
    }
}
