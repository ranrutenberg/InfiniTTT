import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls

ItemDelegate {
    id: root
    required property string symbol
    width: parent ? parent.width : 0
    height: 52

    contentItem: Row {
        spacing: 16
        leftPadding: 16
        anchors.verticalCenter: parent.verticalCenter
        Label { text: root.symbol; color: "white"; font.pixelSize: 18; width: 28 }
        Label { text: root.text;   color: "white"; font.pixelSize: 16 }
    }

    background: Rectangle {
        color: root.hovered ? "#2a2a4e" : "transparent"
    }
}
