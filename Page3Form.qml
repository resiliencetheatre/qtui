import QtQuick 2.15
import QtQuick.Controls 2.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

Page {

    Component.onCompleted: {

    }

    id: pageThree
    width: 240
    height: 1440
    background: Rectangle {
        color: "#000000"
    }
    header: Label {
        id: labelHeader
        y: 5
        x: 0
        text: qsTr("Settings")
        font.pointSize: 10
        padding: 0
        bottomPadding: 20
        color: eClass.mainColor
    }

    Flickable {
        anchors.fill: parent
        contentWidth: parent.width
        contentHeight: 600
        clip: true


        // ****
        Item {
            id: settingsFrame
            anchors.fill: parent

            Label {
                id: wifiScanTitle
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: 5
                text: qsTr("Wifi network")
                font.pointSize: 10
                color: eClass.mainColor
            }

            Rectangle {
                height: 1
                gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: eClass.mainColor }
                        GradientStop { position: 1.0; color: "#000000"  }
                }
                anchors {
                    top: wifiScanTitle.bottom
                    left: parent.left
                    right: parent.right
                    leftMargin: 5
                    rightMargin: 5
                    topMargin: 1
                    bottomMargin: 1

                }
            }

            Label {
                id: wifiNotesLabel
                anchors.top: wifiScanTitle.bottom
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.topMargin: 10
                text: qsTr("Scan and connect to wifi network")
                font.pointSize: 6
                color: eClass.dimColor
            }

            // Erase All
            Button {
                id: wifiEraseAllButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: wifiScanTitle.bottom //  parent.top
                anchors.topMargin: 10
                width: 70
                height: 20
                text: "Erase All"
                font.pointSize: 8
                checkable: false
                onClicked: {
                    eClass.wifiEraseAllConnection()
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
                    color: parent.down ? eClass.dimColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: parent.down ? eClass.mainColor : eClass.mainColor
                    radius: 2
                }
            }

            ComboBox {
                id: control
                model: eClass.wifiNetworks
                font.pointSize: 8
                anchors.top: wifiEraseAllButton.bottom // wifiNotesLabel.bottom
                anchors.topMargin: 10
                anchors.left: parent.left
                anchors.leftMargin: 15 // 5
                height: 20
                width: 130 // 140
                leftPadding: 3
                onCurrentIndexChanged: wifiPassword.text = "";
                delegate: ItemDelegate {
                    width: control.width
                    contentItem: Text {
                        text: modelData
                        color: eClass.mainColor
                        font: control.font
                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                // red arrow down indicator
                indicator: Canvas {
                    id: canvas
                    x: control.width - width - control.rightPadding
                    y: control.topPadding + (control.availableHeight - height) / 2
                    width: 12
                    height: 8
                    contextType: "2d"
                    Connections {
                        target: control
                        function onPressedChanged() { canvas.requestPaint(); }
                    }
                    onPaint: {
                        context.reset();
                        context.moveTo(0, 0);
                        context.lineTo(width, 0);
                        context.lineTo(width / 2, height);
                        context.closePath();
                        context.fillStyle = control.pressed ? eClass.mainColor : "#ff0000";
                        context.fill();
                    }
                }
                contentItem: Text {
                    leftPadding: 1
                    rightPadding: control.indicator.width + control.spacing
                    text: control.displayText
                    font: control.font
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight

                }
                background: Rectangle {
                    color: "transparent"
                    implicitWidth: 120
                    implicitHeight: 20
                    border.color:  eClass.mainColor
                    border.width: control.visualFocus ? 2 : 1
                    radius: 2
                }
                popup: Popup {
                    y: control.height - 1
                    width: control.width
                    implicitHeight: 200
                    padding: 1
                    contentItem: ListView {
                        clip: true
                        implicitHeight: contentHeight
                        model: control.popup.visible ? control.delegateModel : null
                        currentIndex: control.highlightedIndex
                        ScrollIndicator.vertical: ScrollIndicator { }
                    }
                    background: Rectangle {
                        color: "#000000"
                        border.color: eClass.mainColor
                        radius: 2
                    }
                }
            }

            // Buttons
            Button {
                id: wifiScanButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: wifiEraseAllButton.bottom
                anchors.topMargin: 10
                width: 70
                height: 20
                text: "Scan"
                font.pointSize: 8
                checkable: false
                onClicked: {
                    eClass.wifiScanButton()
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
                    color: parent.down ? eClass.dimColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: eClass.mainColor
                    radius: 2
                }
            }
            // Password text entry
            TextField {
                id: wifiPassword
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 20
                width: 130
                anchors.top: control.bottom
                anchors.topMargin: 10
                padding: 1
                font.pointSize: 8
                color: eClass.mainColor
                placeholderTextColor: eClass.dimColor
                placeholderText: "<wifi password>"
                onAccepted: {

                }
                /* Limit to 63 */
                onTextChanged: {
                    eClass.registerTouch()
                    if (length > 63)
                        remove(63, length)
                }
                background: Rectangle {
                    radius: 2
                    color: "#000000"
                    anchors.fill: parent
                    height: 20
                    border.color: eClass.mainColor
                    border.width: 1
                }
            }

            // Save button
            Button {
                id: wifiConnectButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: wifiScanButton.bottom
                anchors.topMargin: 10
                width: 70
                height: 20
                text: "Connect"
                font.pointSize: 8
                checkable: false
                onClicked: {
                    eClass.wifiConnectButton(control.currentValue,wifiPassword.text)
                    wifiPassword.text = "*******";
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
                    color: parent.down ? eClass.dimColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: parent.down ? eClass.mainColor : eClass.dimColor
                    radius: 2
                }
            }

            Label {
                id: wifiStatusTitle
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 15
                width: 140
                anchors.top: wifiPassword.bottom
                anchors.topMargin: 10
                padding: 1
                font.pointSize: 6
                text: qsTr("Wifi status:")
                color: eClass.dimColor
            }

            Text {
                id: wifiStatusText
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 20
                width: parent.width - 50
                anchors.rightMargin: 20
                anchors.top: wifiStatusTitle.bottom
                anchors.topMargin: -1
                padding: 1
                font.pointSize: 8
                wrapMode: Text.WordWrap
                text: eClass.wifiStatusText
                color: eClass.mainColor
            }

            // Wifi status button
            Button {
                id: wifiStatusButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: wifiStatusTitle.bottom
                anchors.topMargin: -7
                width: 20
                height: 20
                text: "?"
                font.pointSize: 8
                checkable: false
                onClicked: {
                    eClass.getWifiStatus()
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
                    color: parent.down ? eClass.mainColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: parent.down ? eClass.mainColor : eClass.dimColor
                    radius: 10
                }
            }

            // Power save section
            Label {
                id: powerSaveTitle
                anchors.left: parent.left
                anchors.leftMargin: 5
                height: 20
                width: 140
                anchors.top: wifiStatusText.bottom
                anchors.topMargin: 20
                padding: 1
                font.pointSize: 10
                text: qsTr("Options")
                color: eClass.mainColor
            }

            Rectangle {
                height: 1
                gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: eClass.mainColor }
                        GradientStop { position: 1.0; color: "#000000"  }
                }
                anchors {
                    top: powerSaveTitle.bottom
                    left: parent.left
                    right: parent.right
                    leftMargin: 5
                    rightMargin: 5
                    topMargin: 1
                    bottomMargin: 1

                }
            }

            // Checkbox (this was hard for me to understand)
            CheckBox {
                id: deepSleepCheckbox
                anchors.top: powerSaveTitle.bottom
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("deep sleep on screen lock")
                checked: eClass.deepSleepEnabled
                onCheckedChanged: {
                    if (checked !== eClass.deepSleepEnabled ) {
                        eClass.changeDeepSleepEnabled(deepSleepCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: deepSleepCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: deepSleepCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: deepSleepCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: deepSleepCheckbox.indicator.width + deepSleepCheckbox.spacing
                }
            }

            // Night mode
            CheckBox {
                id: nightModeCheckbox
                anchors.top: deepSleepCheckbox.bottom
                anchors.topMargin: -5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Night mode")
                checked: eClass.nightModeEnabled
                onCheckedChanged: {
                    if (checked !== eClass.changeNightModeEnabled ) {
                        eClass.changeNightModeEnabled(nightModeCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: nightModeCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: nightModeCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: nightModeCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: nightModeCheckbox.indicator.width + nightModeCheckbox.spacing
                }
            }

            // Call Sign visible on vault pin (aku) m_callSignVisibleOnVaultPage
            CheckBox {
                id: vaultPinCallSignCheckbox
                anchors.top: nightModeCheckbox.bottom
                anchors.topMargin: -5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Call sign on vault pin page")
                checked: eClass.callSignOnVaultEnabled
                onToggled: {
                    if (checked !== eClass.callSignOnVaultEnabled ) {
                        eClass.setCallSignOnVaultEnabled(vaultPinCallSignCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: vaultPinCallSignCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: vaultPinCallSignCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: vaultPinCallSignCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: vaultPinCallSignCheckbox.indicator.width + vaultPinCallSignCheckbox.spacing
                }
            }


            // ---

            Label {
                id: connectivityTitle
                anchors.left: parent.left
                anchors.leftMargin: 5
                height: 20
                width: 140
                anchors.top: vaultPinCallSignCheckbox.bottom
                anchors.topMargin: 10
                padding: 1
                font.pointSize: 10
                text: qsTr("Cellular connection")
                color: eClass.mainColor
            }

            Rectangle {
                height: 1
                gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: eClass.mainColor }
                        GradientStop { position: 1.0; color: "#000000"  }
                }
                anchors {
                    top: connectivityTitle.bottom
                    left: parent.left
                    right: parent.right
                    leftMargin: 5
                    rightMargin: 5
                    topMargin: 1
                    bottomMargin: 1

                }
            }


            // LTE
            CheckBox {
                id: lteCheckbox
                anchors.top: connectivityTitle.bottom
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Cellular data (requires boot)")
                checked: eClass.lteEnabled
                onCheckedChanged: {
                    if (checked !== eClass.lteEnabled ) {
                        eClass.changeLteEnabled(lteCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: lteCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: lteCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: lteCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: lteCheckbox.indicator.width + lteCheckbox.spacing
                }
            }
            // LTE cell display
            CheckBox {
                id: lteCellDisplayCheckbox
                anchors.top: lteCheckbox.bottom
                anchors.topMargin: -5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Display cell environment")
                checked: eClass.cellDisplayEnabled
                onToggled: {
                    if (checked !== eClass.lteCellDisplayEnabled ) {
                        eClass.setLteCellDisplayEnabled(lteCellDisplayCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: lteCellDisplayCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: lteCellDisplayCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: lteCellDisplayCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: lteCellDisplayCheckbox.indicator.width + lteCellDisplayCheckbox.spacing
                }
            }

            Label {
                id: apnTitle
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 15
                width: 140
                anchors.top: lteCellDisplayCheckbox.bottom
                anchors.topMargin: 0
                padding: 1
                font.pointSize: 6
                text: qsTr("APN (requires reboot)")
                color: eClass.dimColor
            }

            // APN
            TextField {
                id: apnName
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 20
                width: 130
                anchors.top: apnTitle.bottom
                anchors.topMargin: 0
                padding: 1
                font.pointSize: 8
                color: eClass.mainColor
                placeholderTextColor: eClass.dimColor
                text: eClass.apnName
                onAccepted: {

                }
                /* Limit to 63 */
                onTextChanged: {
                    eClass.registerTouch()
                    if (length > 63)
                        remove(63, length)
                }
                background: Rectangle {
                    radius: 2
                    color: "#000000"
                    anchors.fill: parent
                    height: 20
                    border.color: eClass.mainColor
                    border.width: 1
                }
            }

            // APN save button
            Button {
                id: apnSaveButton
                anchors.right: parent.right
                anchors.rightMargin: 10
                anchors.top: apnTitle.bottom
                anchors.topMargin: 0
                width: 70
                height: 20
                text: "Save"
                font.pointSize: 8
                checkable: false
                onClicked: {
                    eClass.apnSaveButton(apnName.text)
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
                    color: parent.down ? eClass.dimColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: parent.down ? eClass.mainColor : eClass.dimColor
                    radius: 2
                }
            }

            // macsec
            Label {
                id: macsecTitle
                anchors.left: parent.left
                anchors.leftMargin: 5
                height: 20
                width: 140
                anchors.top: apnName.bottom
                anchors.topMargin: 15
                padding: 1
                font.pointSize: 10
                text: qsTr("MACsec (IEEE 802.1AE)")
                color: eClass.mainColor
            }

            Rectangle {
                height: 1
                gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop { position: 0.0; color: eClass.mainColor }
                        GradientStop { position: 1.0; color: "#000000"  }
                }
                anchors {
                    top: macsecTitle.bottom
                     left: parent.left
                     right: parent.right
                     leftMargin: 5
                     rightMargin: 5
                     topMargin: 1
                     bottomMargin: 1
                }
            }

            // Layer2Wifi
            CheckBox {
                id: layer2Wifi
                anchors.top: macsecTitle.bottom
                anchors.topMargin: 5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("layer2 wifi only")
                checked: eClass.layer2Wifi
                onToggled: {
                    if (checked !== eClass.layer2Wifi ) {
                        eClass.setLayer2Wifi(layer2Wifi.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: layer2Wifi.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: layer2Wifi.checked
                    }
                }
                contentItem: Text {
                    text: layer2Wifi.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: layer2Wifi.indicator.width + layer2Wifi.spacing
                }
            }
            Label {
                id: layer2WifiHelpText
                anchors.left: parent.left
                anchors.leftMargin: 32
                height: 15
                width: 140
                anchors.top: layer2Wifi.bottom
                anchors.topMargin: -6
                padding: 1
                font.pointSize: 6
                text: qsTr("Disables layer3 on wifi connection")
                color: eClass.dimColor
            }




            // macsec PTT
            CheckBox {
                id: pttCheckbox
                anchors.top: layer2WifiHelpText.bottom
                anchors.topMargin: -5
                anchors.left: parent.left
                anchors.leftMargin: 10
                text: qsTr("Enable PTT to macsec segment")
                checked: eClass.macsecPttEnabled
                onToggled: {
                    if (checked !== eClass.macsecPttEnabled ) {
                        eClass.setMacsecPttEnabled(pttCheckbox.checkState)
                    }
                }
                indicator: Rectangle {
                    implicitWidth: 10
                    implicitHeight: 10
                    x: pttCheckbox.leftPadding
                    y: parent.height / 2 - height / 2
                    radius: 2
                    color: "transparent"
                    border.color: eClass.mainColor

                    Rectangle {
                        width: 6
                        height: 6
                        x: 2
                        y: 2
                        radius: 2
                        color: eClass.mainColor
                        visible: pttCheckbox.checked
                    }
                }
                contentItem: Text {
                    text: pttCheckbox.text
                    font.pointSize: 8
                    opacity: enabled ? 1.0 : 0.3
                    color: eClass.mainColor
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: pttCheckbox.indicator.width + pttCheckbox.spacing
                }
            }

            Label {
                id: pttHelpText
                anchors.left: parent.left
                anchors.leftMargin: 32
                height: 15
                width: 140
                anchors.top: pttCheckbox.bottom
                anchors.topMargin: -6
                padding: 1
                font.pointSize: 6
                text: qsTr("Volume Down changes to PTT button.\nRequires L2 capable radio, like AN/PRC-169.")
                color: eClass.dimColor
            }
            Label {
                id: pttKeyingStatus
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 15
                width: 110
                anchors.top: pttHelpText.bottom
                anchors.topMargin: 20
                padding: 1
                font.pointSize: 8
                text: qsTr("MACsec key status:")
                color: eClass.mainColor
            }
            Rectangle {
                height: 20
                color: "transparent"
                radius: 0
                border.width: 1
                border.color: eClass.mainColor

                anchors {
                    top: pttHelpText.bottom
                     left: pttKeyingStatus.right
                     right: parent.right
                     leftMargin: 45
                     rightMargin: 10
                     topMargin: 25
                     bottomMargin: 10
                }
                Text {

                        anchors.centerIn: parent
                        id: macsecKeyStatus
                        font.pointSize: 8
                        text:  eClass.macsecKeyed
                        color: eClass.dimColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
            }
            Label {
                id: keyloadHelpText
                anchors.left: parent.left
                anchors.leftMargin: 15
                height: 15
                width: 140
                anchors.top: pttKeyingStatus.bottom
                anchors.topMargin: -1
                padding: 1
                font.pointSize: 6
                text: qsTr("Insert HSM module to rekey MACsec.")
                color: eClass.dimColor
            }

            // About button
            Button {
                id: aboutButton
                anchors.horizontalCenter: parent.horizontalCenter
                // anchors.top: nightModeCheckbox.bottom
                // anchors.topMargin: 50
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 20

                width: 20
                height: 20
                text: "i"
                font.pointSize: 10
                checkable: false
                onClicked: {
                    popup.open()
                    eClass.registerTouch()
                }
                contentItem: Text {
                    text: parent.text
                    font: parent.font
                    opacity: enabled ? 1.0 : 0.3
                    color: parent.down ? eClass.mainColor : eClass.dimColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
                background: Rectangle {
                    anchors.fill: parent
                    color: parent.down ? eClass.mainColor : "#000"
                    opacity: enabled ? 1 : 0.3
                    border.color: parent.down ? eClass.dimColor : eClass.mainColor
                    radius: 10
                }
            }

            // Version label
            Label {
                id: versionLabel
                height: 20
                width: 50
                anchors.top: aboutButton.bottom
                anchors.topMargin: 5
                anchors.horizontalCenter: parent.horizontalCenter
                padding: 1
                font.pointSize: 6
                text: eClass.appVersion() // qsTr("v0.3 [DEV]")
                color: eClass.mainColor
                horizontalAlignment: Text.AlignHCenter
            }


        } // ** item
    } // ** flickable

    Popup {
        id: popup
        anchors.centerIn: parent
        width: 220
        height: 350
        modal: true
        focus: true
        clip: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
        background: Rectangle {
            color: "#000000"
            border.color: eClass.mainColor
            radius: 2
        }

        Text {
            id: popupTitleText
            font.pointSize: 12
            text: "About"
            color: eClass.mainColor
        }
        ScrollView {
            id: view
            clip: true
            contentWidth: -1
            anchors.fill: parent
            anchors.top: popupTitleText.bottom
            anchors.topMargin: 25
            anchors.bottomMargin: 30
            Text {
                textFormat: Text.RichText
                width: 200
                wrapMode: Text.WordWrap
                color: eClass.mainColor
                text: eClass.aboutTextContent
                font.pointSize: 7
            }
        }
        // Close button
        Button {
            id: popupCloseButton

            anchors{
                bottom: parent.bottom
                horizontalCenter: parent.horizontalCenter
            }
            width: 50
            height: 20
            text: "Close"
            font.pointSize: 8
            checkable: false
            onClicked: {
                popup.close()
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
                color: parent.down ? eClass.dimColor : "transparent"
                opacity: enabled ? 1 : 0.3
                border.color: eClass.mainColor
                radius: 2
            }
        }
        enter: Transition {
            NumberAnimation { property: "opacity"; from: 0.0; to: 1.0 }
        }
    } // ** popup

}
