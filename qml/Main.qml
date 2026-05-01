import QtQuick
import QtQuick.Controls.Material
import QtQuick.Controls
import InfiniTTT 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 420
    height: 780
    title: "InfiniTTT"

    Material.theme: Material.Light
    Material.accent: Material.Blue

    // ── Game state ───────────────────────────────────────────────────────────
    property string currentPlayer: "X"
    property int    moveCount:     0
    property bool   gameActive:    false
    property bool   aiThinking:    false
    property bool   humanVsHuman:  false
    property bool   logPanelVisible: false

    // Previous move coords — used by "jump to my move" button after opponent auto-centers
    property int prevMoveX: 0
    property int prevMoveY: 0
    property int lastMoveX: 0
    property int lastMoveY: 0

    // ── AI log model ─────────────────────────────────────────────────────────
    ListModel { id: aiLogModel }

    // ── Board ────────────────────────────────────────────────────────────────
    BoardItem {
        id: board
        anchors.fill: parent
        interactive: root.gameActive && !root.aiThinking
        onCellClicked: (x, y) => gameController.handleCellClick(x, y)
    }

    // ── AI log panel (slide-up overlay above status bar) ─────────────────────
    Rectangle {
        id: aiLogPanel
        anchors { left: parent.left; right: parent.right; bottom: statusBar.top }
        height: root.logPanelVisible ? 200 : 0
        clip: true
        color: "#1a1a2e"

        Behavior on height {
            NumberAnimation { duration: 200; easing.type: Easing.InOutQuad }
        }

        // Header bar
        Rectangle {
            id: logHeader
            anchors { top: parent.top; left: parent.left; right: parent.right }
            height: 36
            color: "#0d0d1e"

            Label {
                anchors { left: parent.left; verticalCenter: parent.verticalCenter; leftMargin: 12 }
                text: "AI Messages"
                color: "white"
                font { pixelSize: 13; bold: true }
            }

            Button {
                anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 4 }
                text: "Clear"
                flat: true
                font.pixelSize: 11
                Material.foreground: "#aaaacc"
                padding: 4
                onClicked: aiLogModel.clear()
            }
        }

        // Scrollable log list
        ListView {
            id: aiLogView
            anchors { top: logHeader.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
            model: aiLogModel
            clip: true
            ScrollBar.vertical: ScrollBar {}

            delegate: Item {
                width: aiLogView.width
                height: entryCol.implicitHeight + 6

                Column {
                    id: entryCol
                    x: 8
                    width: parent.width - 16
                    topPadding: 4

                    Label {
                        width: parent.width
                        text: model.playerText
                        color: model.playerText.indexOf("X") !== -1 ? "#ff6b6b" : "#6b9fff"
                        font { pixelSize: 11; bold: true }
                    }
                    Label {
                        width: parent.width
                        text: model.body
                        color: "#cccccc"
                        font { pixelSize: 10; family: "Monospace" }
                        wrapMode: Text.WrapAnywhere
                    }
                }
            }

            onCountChanged: positionViewAtEnd()
        }
    }

    // ── Status bar (bottom overlay) ──────────────────────────────────────────
    Rectangle {
        id: statusBar
        anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
        height: 52
        color: "#CC1a1a2e"

        Row {
            anchors.centerIn: parent
            spacing: 20

            Label {
                color: "white"
                font.pixelSize: 15
                text: {
                    if (!root.gameActive)   return "Tap ☰ to start"
                    if (root.aiThinking)    return "AI thinking…"
                    return "Turn: " + (root.currentPlayer === "X" ? "✕" : "○")
                           + "  (Player " + root.currentPlayer + ")"
                }
            }

            Label {
                color: "#aaaacc"
                font.pixelSize: 13
                text: root.gameActive ? "Move " + root.moveCount : ""
            }
        }

        // Jump to previous move (only visible after the 2nd move, so there's a "before" to go back to)
        RoundButton {
            anchors { right: logToggleBtn.left; verticalCenter: parent.verticalCenter; rightMargin: 2 }
            width: 36; height: 36
            text: "📍"
            font.pixelSize: 16
            flat: true
            visible: root.moveCount >= 2
            Material.foreground: "white"
            onClicked: board.centerOn(root.prevMoveX, root.prevMoveY)
        }

        // Log panel toggle
        RoundButton {
            id: logToggleBtn
            anchors { right: parent.right; verticalCenter: parent.verticalCenter; rightMargin: 8 }
            width: 36; height: 36
            text: "💬"
            font.pixelSize: 16
            flat: true
            Material.foreground: root.logPanelVisible ? Material.accent : "white"
            onClicked: root.logPanelVisible = !root.logPanelVisible
        }
    }

    // ── Menu button (top-right) ──────────────────────────────────────────────
    RoundButton {
        anchors { top: parent.top; right: parent.right; margins: 12 }
        width: 44; height: 44
        text: "☰"
        font.pixelSize: 18
        Material.background: Material.Blue
        Material.foreground: "white"
        onClicked: drawer.open()
    }

    // ── Side drawer ──────────────────────────────────────────────────────────
    Drawer {
        id: drawer
        width: Math.min(parent.width * 0.72, 260)
        height: parent.height

        background: Rectangle { color: "#1a1a2e" }

        Column {
            anchors { top: parent.top; left: parent.left; right: parent.right; topMargin: 48 }

            Label {
                leftPadding: 20
                text: "InfiniTTT"
                color: "white"
                font { pixelSize: 22; bold: true }
                bottomPadding: 24
            }

            DrawerItem { text: "New Game";       symbol: "🎮"; onClicked: { drawer.close(); setupDialog.open() } }
            DrawerItem { text: "Reset View";      symbol: "🔍"; onClicked: { drawer.close(); board.resetView() } }
            DrawerItem { text: "Undo Last Move";  symbol: "↩";  visible: root.humanVsHuman && root.gameActive
                         onClicked: { drawer.close(); gameController.undoMove() } }
            DrawerItem { text: "AI Log";          symbol: "💬"; onClicked: { drawer.close(); root.logPanelVisible = !root.logPanelVisible } }
            DrawerItem { text: "About";           symbol: "ℹ";  onClicked: { drawer.close(); aboutDialog.open() } }
        }
    }

    // ── Dialogs ──────────────────────────────────────────────────────────────
    GameSetupDialog {
        id: setupDialog
        onGameStarted: (p1Human, p1AI, p1Lvl, p2Human, p2AI, p2Lvl) => {
            aiLogModel.clear()
            board.clearBoard()
            root.moveCount    = 0
            root.lastMoveX    = 0
            root.lastMoveY    = 0
            root.prevMoveX    = 0
            root.prevMoveY    = 0
            root.gameActive   = true
            root.aiThinking   = false
            root.humanVsHuman = p1Human && p2Human
            gameController.startNewGameQML(p1Human, p1AI, p1Lvl, p2Human, p2AI, p2Lvl)
        }
    }

    GameOverDialog {
        id: gameOverDialog
        onNewGameRequested: setupDialog.open()
    }

    Dialog {
        id: aboutDialog
        anchors.centerIn: Overlay.overlay
        title: "About InfiniTTT"
        standardButtons: Dialog.Ok
        Material.accent: Material.Blue

        Label {
            width: aboutDialog.availableWidth
            wrapMode: Text.Wrap
            text: "Infinite Tic-Tac-Toe\n\n" +
                  "Get 5 in a row to win!\n\n" +
                  "Controls:\n" +
                  "• Tap to place your mark\n" +
                  "• Pinch to zoom\n" +
                  "• Drag with two fingers to pan\n" +
                  "• On desktop: scroll wheel to zoom,\n" +
                  "  right-drag to pan"
        }
    }

    // ── Controller connections ────────────────────────────────────────────────
    Connections {
        target: gameController

        function onMoveExecuted(x, y, mark) {
            board.placeMark(x, y, mark)
            root.prevMoveX = root.lastMoveX
            root.prevMoveY = root.lastMoveY
            root.lastMoveX = x
            root.lastMoveY = y
            if (!board.isCellVisible(x, y)) board.centerOn(x, y)
            root.moveCount++
        }

        function onTurnChanged(player) {
            root.currentPlayer = String.fromCharCode(player)
        }

        function onGameOver(winner) {
            root.gameActive = false
            gameOverDialog.winner = winner === 68 ? "" : String.fromCharCode(winner)
            gameOverDialog.isDraw = (winner === 68) // 'D'
            gameOverTimer.start()
        }

        function onAiThinking(thinking) {
            root.aiThinking = thinking
        }

        function onMoveUndone() {
            board.rebuildMarks(gameController.getMoveHistoryQML())
            root.moveCount = gameController.getMoveHistoryQML().length
        }

        function onAiMessage(playerMark, message) {
            var letter = String.fromCharCode(playerMark)
            aiLogModel.append({ playerText: "Player " + letter + ":", body: message })
        }
    }

    // Delay game-over dialog so the winning position is visible before it covers the board
    Timer {
        id: gameOverTimer
        interval: 1500
        repeat: false
        onTriggered: gameOverDialog.open()
    }

    // ── Auto-open setup on launch ─────────────────────────────────────────────
    Component.onCompleted: setupDialog.open()
}
