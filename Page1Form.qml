import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3 // cell

Page {
    id: page
    width: 720
    height: 1440
    property string p_commandButtonColor: eClass.mainColor

    Component.onCompleted: {
    }

    background: Rectangle {
        color: "#000000"
    }

    header: Label {
        id: labelHeader
        y: 5
        x: 0
        visible: false
        text: qsTr("Voice communication")
        font.pointSize: 8
        padding: 0
        color: "#ffff00"
    }


    Frame {
        id: callStatus
        anchors.topMargin: 0
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 230 // 250
        background: Rectangle {
            color: "transparent"
            border.color: "#ffffff"
            border.width: 0
            radius: 2
        }
        Rectangle {
            id: callSignImage
            border.width: 2
            border.color: "#ffffff"
            anchors.centerIn: parent
            color: "transparent"
            Image {
                id: callSignInsignia
                anchors.centerIn: parent
                width: 130
                height: 130
                source: eClass.callSignInsigniaImage
                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        // eClass.debugThis('')
                        // console.log("clicked")
                    }
                }
            }
            Label {
                id: insigniaLabel
                anchors.top: callSignInsignia.bottom
                anchors.right: callSignInsignia.right
                text: eClass.insigniaLabelText
                font.pointSize: 12
                padding: 0
                color: eClass.mainColor
            }
            Label {
                id: insigniaLabelState
                anchors.top: insigniaLabel.bottom
                anchors.right: insigniaLabel.right
                text: eClass.insigniaLabelStateText
                font.pointSize: 8
                padding: 0
                color: eClass.mainColor
            }
        }
    }

    Frame {
        id: statusTextFrame
        anchors.topMargin: 0
        anchors.top: callStatus.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 30
        background: Rectangle {
            color: "transparent"
            border.color: "#ffffff"
            border.width: 0
            radius: 2
        }
        Label {
            id: statusMessage
            text: eClass.statusMessage
            font.pointSize: 8
            padding: 0
            color: eClass.mainColor
        }
        // TODO: Remove this
        Image {
            id: powerButtonImage
            source: "powerbutton.png"
            width: 15
            height: 15
            anchors.top: parent.top
            anchors.right: parent.right
            visible: false
            MouseArea {
                id: area
                hoverEnabled: true
                width: 15
                height: 15
                anchors.fill: parent
                acceptedButtons: Qt.AllButtons
                onClicked: {
                    eClass.powerOff()
                }
            }
        }
    }
    //
    Frame {
        id: cellStatus
        visible: eClass.lteEnabled
        anchors.topMargin: -5
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        background: Rectangle {
            color: "transparent"
            border.color: "#ffffff"
            border.width: 0
            radius: 2
        }
        RowLayout {
            id: statusRowOne
            Label {
                id: plmnStatus
                text: eClass.plmn
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: taStatus
                text: eClass.ta
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: gcStatus
                text: eClass.gc
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: scStatus
                text: eClass.sc
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: acStatus
                text: eClass.ac
                font.pointSize: 6
                color: eClass.dimColor
            }
        }
        RowLayout {
            id: statusRowTwo
            anchors.top: statusRowOne.bottom
            anchors.topMargin: 5
            Label {
                id: rssiStatus
                text: eClass.rssi
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: rsrqStatus
                text: eClass.rsrq
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: rsrpStatus
                text: eClass.rsrp
                font.pointSize: 6
                color: eClass.dimColor
            }
            Label {
                id: snrStatus
                text: eClass.snr
                font.pointSize: 6
                color: eClass.dimColor
            }

        }


    }

    /* Contacts: 10 peer */
    Frame {
        id: contactButtonFrame
        anchors.top: statusTextFrame.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        background: Rectangle {
            color: "transparent"
            border.color: "#21be2b"
            border.width: 0
            radius: 2
        }

        // --- First row ---
        Button {
            id: contactOneButton
            x: 0
            y: 0
            width: 65
            height: 20
            text: eClass.peer_0_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_0_active
            onClicked: {
                eClass.connectButton(0)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_0_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000" // "#141" : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor // parent.down ? "#0F0" : "#0D0"
                radius: 2
            }
        }
        Label {
            id: contactOneStatusLabel
            x: 0
            y: 22
            width: 65
            height: 5
            visible: eClass.peer_0_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_0_label_color
            }
        }

        Button {
            id: contactTwoButton
            x: 75
            y: 0
            width: 65
            height: 20
            text: eClass.peer_1_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_1_active
            onClicked: {
                eClass.connectButton(1)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_1_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactTwoStatusLabel
            x: 75
            y: 22
            width: 65
            height: 5
            visible: eClass.peer_1_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_1_label_color
            }
        }

        Button {
            id: contactThreeButton
            x: 150
            y: 0
            width: 65
            height: 20
            text: eClass.peer_2_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_2_active
            onClicked: {
                eClass.connectButton(2)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_2_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactThreeStatusLabel
            x: 150
            y: 22
            width: 65
            height: 5
            visible: eClass.peer_2_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_2_label_color
            }
        }

        // --- Second row ---
        Button {
            id: contactFourButton
            x: 0
            y: 35
            width: 65
            height: 20
            text: eClass.peer_3_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_3_active
            onClicked: {
                eClass.connectButton(3)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_3_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactFourStatusLabel
            x: 0
            y: 57
            width: 65
            height: 5
            visible: eClass.peer_3_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_3_label_color
            }
        }

        Button {
            id: contactFiveButton
            x: 75
            y: 35
            width: 65
            height: 20
            text: eClass.peer_4_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_4_active
            onClicked: {
                eClass.connectButton(4)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_4_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactFiveStatusLabel
            x: 75
            y: 57
            width: 65
            height: 5
            visible: eClass.peer_4_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_4_label_color
            }
        }

        Button {
            id: contactSixButton
            x: 150
            y: 35
            width: 65
            height: 20
            text: eClass.peer_5_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_5_active
            onClicked: {
                eClass.connectButton(5)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_5_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactSixStatusLabel
            x: 150
            y: 57
            width: 65
            height: 5
            visible: eClass.peer_5_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_5_label_color
            }
        }

        // --- Third row ---
        Button {
            id: contactSevenButton
            x: 0
            y: 70
            width: 65
            height: 20
            text: eClass.peer_6_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_6_active
            onClicked: {
                eClass.connectButton(6)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_6_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactSevenStatusLabel
            x: 0
            y: 92
            width: 65
            height: 5
            visible: eClass.peer_6_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_6_label_color
            }
        }

        Button {
            id: contactEightButton
            x: 75
            y: 70
            width: 65
            height: 20
            text: eClass.peer_7_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_7_active
            onClicked: {
                eClass.connectButton(7)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_7_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactEightStatusLabel
            x: 75
            y: 92
            width: 65
            height: 5
            visible: eClass.peer_7_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_7_label_color
            }
        }

        Button {
            id: contactNineButton
            x: 150
            y: 70
            width: 65
            height: 20
            text: eClass.peer_8_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_8_active
            onClicked: {
                eClass.connectButton(8)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_8_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactNineStatusLabel
            x: 150
            y: 92
            width: 65
            height: 5
            visible: eClass.peer_8_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_8_label_color
            }
        }

        // --- Fourth row ---
        Button {
            id: goSecureButton
            x: 0
            y: 105
            width: 65
            height: 25
            text: qsTr("Go Sec")
            font.pointSize: 8
            checkable: false
            enabled: eClass.goSecureButton_active
            onClicked: {
                eClass.on_goSecure_clicked()
                eClass.registerTouch()
            }

            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: p_commandButtonColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: p_commandButtonColor
                border.width: 1
                radius: 0
            }
        }

        // --- Contact ten button ---
        Button {
            id: contactTenButton
            x: 75
            y: 105
            width: 65
            height: 20
            text: eClass.peer_9_Name
            font.pointSize: 8
            checkable: false
            enabled: eClass.button_9_active
            onClicked: {
                eClass.connectButton(9)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: eClass.peer_9_NameColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                border.width: 1
                radius: 2
            }
        }
        Label {
            id: contactTenStatusLabel
            x: 75
            y: 127
            width: 65
            height: 5
            visible: eClass.peer_9_label
            background: Rectangle {
                anchors.fill: parent
                color: eClass.peer_9_label_color
            }
        }

        Button {
            id: msgSecureButton
            x: 150
            y: 105
            width: 65
            height: 25
            text: qsTr("Message")
            font.pointSize: 8
            checkable: false
            onClicked: {
                window.pageSwipeIndex.setCurrentIndex(1)
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: p_commandButtonColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: p_commandButtonColor
                border.width: 1
                radius: 0
            }
        }

        Button {
            id: terminateButton
            x: 0
            y: 140
            width: parent.width
            Component.onCompleted: {
            }

            height: 25
            text: qsTr("Terminate")
            font.pointSize: 8
            checkable: false
            onClicked: {
                eClass.disconnectButton()
                pageTwo.messageInput = ""
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: p_commandButtonColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? eClass.highColor : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: p_commandButtonColor
                border.width: 1
                radius: 0
            }
        }

        // --- Key status ---
        Button {
            id: keyStatusButton
            anchors.right: parent.right
            anchors.top: terminateButton.bottom
            anchors.topMargin: 10
            width: 65
            height: 15
            text: "Key status"
            font.pointSize: 6
            checkable: false
            onDownChanged: {
                keyStatusFrame.visible = !keyStatusFrame.visible
                eClass.registerTouch()
                plmnStatus.text = "[PLMN]"
                plmnStatus.color = eClass.mainColor
                taStatus.text = "[TAC]"
                taStatus.color = eClass.mainColor
                gcStatus.text = "[GLOBAL CELL]"
                gcStatus.color = eClass.mainColor
                scStatus.text = "[SERV CELL]"
                scStatus.color = eClass.mainColor
                acStatus.text = "[RF CH]"
                acStatus.color = eClass.mainColor
                rssiStatus.text = "[RSSI]"
                rssiStatus.color = eClass.mainColor
                rsrqStatus.text = "[RSRQ]"
                rsrqStatus.color = eClass.mainColor
                rsrpStatus.text = "[RSRP]"
                rsrpStatus.color = eClass.mainColor
                snrStatus.text = "[SNR]"
                snrStatus.color = eClass.mainColor
            }
            onReleased: {
                plmnStatus.text = eClass.plmn
                plmnStatus.color = eClass.dimColor
                taStatus.text = eClass.ta
                taStatus.color = eClass.dimColor
                gcStatus.text = eClass.gc
                gcStatus.color = eClass.dimColor
                scStatus.text = eClass.sc
                scStatus.color = eClass.dimColor
                acStatus.text = eClass.ac
                acStatus.color = eClass.dimColor
                rssiStatus.text = eClass.rssi
                rssiStatus.color = eClass.dimColor
                rsrqStatus.text = eClass.rsrq
                rsrqStatus.color = eClass.dimColor
                rsrpStatus.text = eClass.rsrp
                rsrpStatus.color = eClass.dimColor
                snrStatus.text = eClass.snr
                snrStatus.color = eClass.dimColor
            }

            onClicked: {
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: parent.down ? eClass.mainColor : eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                anchors.fill: parent
                color: parent.down ? "#151" : "#000"
                opacity: enabled ? 1 : 0.3
                border.color: parent.down ? eClass.mainColor : eClass.mainColor
                radius: 0
                width: 1
            }
        }
    } // Frame

    // --- Frame to show key % ---
    Frame {
        id: keyStatusFrame
        anchors.top: contactButtonFrame.top
        anchors.left: contactButtonFrame.left
        anchors.right: contactButtonFrame.right
        height: 150
        visible: false
        background: Rectangle {
            color: "#000000"
            opacity: 0.5
            border.color: "#21be2b"
            border.width: 0
            radius: 2
        }

        Rectangle {
            id: contactOnePersetangeLabel
            x: 0
            y: 0
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_0_keyPercentage
                font.pointSize: 10
                color: eClass.mainColor
            }
        }

        Rectangle {
            id: contactTwoPersetangeLabel
            x: 75
            y: 0
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_1_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }

        Rectangle {
            id: contactThreePersetangeLabel
            x: 150
            y: 0
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_2_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactFourPersetangeLabel
            x: 0
            y: 35
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_3_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactFivePersetangeLabel
            x: 75
            y: 35
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_4_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactSixPersetangeLabel
            x: 150
            y: 35
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_5_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        // ROW 3
        Rectangle {
            id: contactSevenPersetangeLabel
            x: 0
            y: 70
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_6_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactEightPersetangeLabel
            x: 75
            y: 70
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_7_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactNinePersetangeLabel
            x: 150
            y: 70
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_8_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
        Rectangle {
            id: contactTenPersetangeLabel
            x: 75
            y: 105
            width: 65
            height: 20
            color: "transparent"
            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                text: eClass.peer_9_keyPercentage
                font.pointSize: 8
                color: eClass.mainColor
            }
        }
    }

    // --- Power Off dialog ---
    Frame {
        id: powerOffDialog
        visible: eClass.powerOffDialogVisible
        x: 5
        y: 265
        width: 230
        height: 200
        background: Rectangle {
            anchors.fill: parent
            border.width: 2
            border.color: "#00aa00"
            radius: 0
            color: "#003300"
            opacity: 0.9
        }
        Button {
            id: powerOffButtonOnDialog
            x: 0
            y: 85
            width: (parent.width / 2) - 5
            height: 30
            text: qsTr("Shutdown")
            font.pointSize: 10
            checkable: false
            onClicked: {
                eClass.powerOff()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? "#DD0000" : "#A00"
                opacity: enabled ? 1 : 0.3
                border.color: "#FF0000"
                border.width: 1
                radius: 2
            }
        }
        Button {
            id: cancelPowerOff
            x: (parent.width / 2) + 5
            y: 85
            width: (parent.width / 2) - 5
            height: 30
            text: qsTr("Cancel")
            font.pointSize: 10
            checkable: false
            onClicked: {
                eClass.closePowerOffDialog()
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? "#00DD00" : "#0A0"
                opacity: enabled ? 1 : 0.3
                border.color: "#00DD00"
                border.width: 1
                radius: 2
            }
        }
        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 10
            text: qsTr("Power Off ?")
            color: "#FFFFFF"
            font.pointSize: 14
        }

    }

    // --- Incoming call ---
    Frame {
        id: incomingCallDialog
        visible: eClass.callDialogVisible
        x: 5
        y: 265
        width: 230
        height: 170
        background: Rectangle {
            anchors.fill: parent
            border.width: 2
            border.color: "#00aa00"
            radius: 0
            color: "#003300"
            opacity: 0.9
        }

        Button {
            id: denyCallButton
            x: (parent.width / 2) + 5
            y: 85
            width: (parent.width / 2) - 5
            height: 30
            text: qsTr("Deny")
            font.pointSize: 10
            checkable: false
            onClicked: {
                eClass.on_denyButton_clicked()
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? "#DD0000" : "#A00"
                opacity: enabled ? 1 : 0.3
                border.color: "#FF0000"
                border.width: 1
                radius: 2
            }
        }
        Button {
            id: acceptCallButton
            x: 0
            y: 85
            width: (parent.width / 2) - 5
            height: 30
            text: qsTr("Accept")
            font.pointSize: 10
            checkable: false
            onClicked: {
                eClass.on_answerButton_clicked()
                eClass.registerTouch()
            }
            contentItem: Text {
                text: parent.text
                font: parent.font
                opacity: enabled ? 1.0 : 0.3
                color: "#FFFFFF"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                elide: Text.ElideRight
            }
            background: Rectangle {
                color: parent.down ? "#00DD00" : "#0A0"
                opacity: enabled ? 1 : 0.3
                border.color: "#00DD00"
                border.width: 1
                radius: 2
            }
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            y: 10
            text: qsTr("Incoming audio")
            color: "#FFFFFF"
            font.pointSize: 14
        }
    }
}
