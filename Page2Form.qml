import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

Page {
    property alias messageHistory: msgHistory.text
    property alias messageInput: lineInput.text
    property alias msgHistory: msgHistory

    id: pageTwo
    width: 240
    height: 1440
    background: Rectangle {
        color: "#000000"
    }

    Component.onCompleted: {

    }

    header: Label {
        id: labelHeader
        y: 5
        x: 0
        text: qsTr("Messaging")
        font.pointSize: 10
        padding: 0
        color: eClass.mainColor
    }

    Button {
        id: msgMenuButton
        anchors.right: parent.right
        anchors.rightMargin: 10
        anchors.bottom: textFlow.top
        anchors.bottomMargin: 6
        width: 60
        height: 15
        text: "Command"
        font.pointSize: 6
        checkable: false
        onClicked: {
            commandButtonsFrame.visible = !commandButtonsFrame.visible
            eClass.registerTouch()
        }
        contentItem: Text {
            text: parent.text
            font: parent.font
            opacity: enabled ? 1.0 : 0.3
            color: eClass.mainColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
        background: Rectangle {
            anchors.fill: parent
            color: parent.down ? eClass.highColor : "#000"
            opacity: enabled ? 1 : 0.3
            border.color: eClass.mainColor
            radius: 2
        }
    }

    Rectangle {
        id: textFlow
        y: 10
        radius: 0
        color: "#000000"
        width: 240
        height: 280
        border.color: eClass.mainColor
        border.width: 1

        Rectangle {
            id: backgroundLogoImage
            border.width: 0
            border.color: "#ffffff"
            anchors.centerIn: parent
            anchors.fill: parent
            color: "transparent"

            Item {
                anchors.centerIn: parent
                Image {
                    id: callSignInsigniaBackground
                    anchors.centerIn: parent
                    width: 160
                    height: 160
                    source: eClass.callSignInsigniaImage
                    opacity: 0.3
                }
            }

            Flickable {
                id: textScrollView
                anchors.fill: parent
                width: 240
                clip: true
                flickableDirection: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar {
                    id: flickScroll
                }
                boundsBehavior: Flickable.DragAndOvershootBounds

                TextArea.flickable: TextArea {
                    id: msgHistory
                    textFormat: TextEdit.RichText
                    width: 240
                    text: eClass.textMsgDisplay
                    font.family: "DejaVu"
                    font.pointSize: 8
                    color: eClass.mainColor
                    readOnly: true
                    wrapMode: Text.WordWrap
                    onContentSizeChanged: {
                        msgHistory.cursorPosition = msgHistory.length - 1
                    }
                }
            }
        } // rectangle
    }

    // Text input field
    TextField {
        id: lineInput
        x: 0
        anchors.top: textFlow.bottom
        width: parent.width

        Component.onCompleted: {

        }
        height: 24
        padding: 2
        font.pointSize: 8
        color: eClass.mainColor
        placeholderTextColor: eClass.dimColor
        placeholderText: "Write here, <enter> to send"
        focus: true
        onAccepted: {
            eClass.on_LineEdit_returnPressed(lineInput.text)
            lineInput.text = ""
            lineInput.placeholderText = ""
            eClass.registerTouch()
        }
        /* Limit msg entry to 190 chars */
        onTextChanged: {
            eClass.registerTouch()
            msgLenLabel.text = length + "/190"
            if (length > 190)
                remove(190, length)
        }
        background: Rectangle {
            radius: 0
            color: "#000000"
            anchors.fill: parent
            height: 24
            border.color: eClass.mainColor
            border.width: 1
        }
    }
    Label {
        id: msgLenLabel
        y: 270
        anchors.right: parent.right
        anchors.rightMargin: 5
        text: qsTr("0/190")
        font.pointSize: 8
        padding: 0
        color: eClass.mainColor
    }

    Frame {
        id: commandButtonsFrame
        property bool stateVisible: true
        visible: false
        width: 70
        height: textFlow.height
        anchors.right: textFlow.right
        anchors.rightMargin: 5
        anchors.verticalCenter: textFlow.verticalCenter
        background: Rectangle {
            color: "transparent"
            border.color: eClass.mainColor
            border.width: 0
            radius: 2
        }

        states: [
            State {
                name: "visible"
                when: commandButtonsFrame.visible
                PropertyChanges {
                    target: commandButtonsFrame
                    opacity: 1.0
                }
            },
            State {
                name: "invisible"
                when: !commandButtonsFrame.visible
                PropertyChanges {
                    target: commandButtonsFrame
                    opacity: 0.0
                }
            }
        ]
        transitions: Transition {
            NumberAnimation {
                property: "opacity"
                duration: 300
                easing.type: Easing.InOutQuad
            }
        }

        Button {
            id: msgPingButton
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "Check"
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(1)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        Button {
            id: msgRedLedOn
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "Red On"
            anchors.top: msgPingButton.bottom
            anchors.topMargin: 5
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(2)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        Button {
            id: msgRedLedOff
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "Red Off"
            anchors.top: msgRedLedOn.bottom
            anchors.topMargin: 5
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(3)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        Button {
            id: msgGreenLedOn
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "Green On"
            anchors.top: msgRedLedOff.bottom
            anchors.topMargin: 5
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(4)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        Button {
            id: msgGreenLedOff
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "Green Off"
            anchors.top: msgGreenLedOn.bottom
            anchors.topMargin: 5
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(5)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        Button {
            id: msgSonarPing
            anchors.horizontalCenter: parent.horizontalCenter
            width: 60
            height: 15
            text: "One Ping"
            anchors.top: msgGreenLedOff.bottom
            anchors.topMargin: 5
            font.pointSize: 7
            checkable: false
            onClicked: {
                eClass.quickButtonSend(6)
                commandButtonsFrame.visible = false
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
    }
}
