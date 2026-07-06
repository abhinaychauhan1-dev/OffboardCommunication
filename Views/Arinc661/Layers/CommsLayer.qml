/*
 * Renders a multi-view offboard communication console with aligned, non-overlapping layouts.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../ParameterBus.js" as P661
import "../Widgets"

Rectangle {
    id: commsRoot
    color: "#132037"
    clip: true

    readonly property int navBarHeight: Math.max(36, Math.min(48, Math.round(height * 0.055)))
    readonly property int linkCardHeight: Math.max(68, Math.min(90, Math.round(height * 0.102)))
    readonly property int statusStripHeight: Math.max(72, Math.min(94, Math.round(height * 0.106)))
    readonly property int opsTopCardHeight: Math.max(142, Math.min(194, Math.round(height * 0.215)))
    readonly property int monitorBottomHeight: Math.max(96, Math.min(132, Math.round(height * 0.145)))

    property int atcMessageIndex: 0
    property int weatherMessageIndex: 0
    property int activePage: 0
    property string commandNameDraft: "SET_MODE"
    property string commandArgsDraft: "{\"mode\":\"EMERGENCY\"}"
    property string replayPathDraft: "D:/Qt_Programs/New_Projects/OffboardCommunication/replay.json"

    readonly property real fontScale: Math.max(0.82, Math.min(1.06, height / 720.0))
    readonly property int fontTitle: Math.max(12, Math.round(14 * fontScale))
    readonly property int fontSection: Math.max(11, Math.round(12 * fontScale))
    readonly property int fontBody: Math.max(10, Math.round(11 * fontScale))
    readonly property int fontSmall: Math.max(9, Math.round(10 * fontScale))
    readonly property int fontMetric: Math.max(18, Math.round(24 * fontScale))
    readonly property int controlHeight: Math.max(26, Math.min(36, Math.round(height * 0.042)))
    readonly property int inputHeight: Math.max(28, Math.min(38, Math.round(height * 0.046)))

    readonly property var atcMessages: [
        "ATC: Maintain current heading and report at next waypoint.",
        "ATC: Traffic advisory active in your 2 o'clock sector.",
        "ATC: Cleared direct to active waypoint when ready.",
        "ATC: Continue present altitude, expect descent window in 12 minutes."
    ]

    readonly property var weatherMessages: [
        "WX: Light chop along route, no convective cells nearby.",
        "WX: Surface winds shifting at destination, monitor runway assignment.",
        "WX: Satellite cloud cover increasing over alternate airport corridor.",
        "WX: Isolated buildup detected 35 nm east of current track."
    ]

    function clamp(v, minV, maxV) {
        return Math.max(minV, Math.min(maxV, v))
    }

    function a2gSignal() {
        var dist = arinc661Bus.doubleValue(P661.P.P_NEAREST_AIRPORT_DIST, 0)
        var wind = arinc661Bus.doubleValue(P661.P.P_WIND_SPEED, 0)
        return clamp(95 - dist * 0.5 - wind * 0.35, 20, 100)
    }

    function a2aSignal() {
        var speed = arinc661Bus.doubleValue(P661.P.P_AIRSPEED, 0)
        var roll = Math.abs(arinc661Bus.doubleValue(P661.P.P_ROLL, 0))
        return clamp(78 + speed * 0.03 - roll * 0.45, 25, 100)
    }

    function satcomSignal() {
        var alt = arinc661Bus.doubleValue(P661.P.P_ALTITUDE, 0)
        return clamp(70 + alt / 1800.0, 35, 100)
    }

    function linkStatus(signalStrength) {
        if (signalStrength >= 80) return "ONLINE"
        if (signalStrength >= 55) return "DEGRADED"
        return "LIMITED"
    }

    function linkColor(signalStrength) {
        if (signalStrength >= 80) return "#44ff44"
        if (signalStrength >= 55) return "#ffaa00"
        return "#ff5555"
    }

    function parseCommandArgs(text) {
        try {
            if (!text || text.trim().length === 0) {
                return ({})
            }
            return JSON.parse(text)
        } catch (e) {
            return ({ parseError: e.toString() })
        }
    }

    Timer {
        interval: 5000
        running: true
        repeat: true
        onTriggered: {
            commsRoot.atcMessageIndex = (commsRoot.atcMessageIndex + 1) % commsRoot.atcMessages.length
            commsRoot.weatherMessageIndex = (commsRoot.weatherMessageIndex + 1) % commsRoot.weatherMessages.length
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 2
        spacing: 4

        Label {
            text: "OFFBOARD COMMUNICATION CONSOLE"
            color: "#00ff88"
            font.pixelSize: commsRoot.fontTitle
            font.bold: true
        }

        Label {
            text: "Use feature buttons to open focused screens without crowding the main page."
            color: "#8adbb5"
            font.pixelSize: commsRoot.fontBody
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: commsRoot.navBarHeight
            radius: 10
            color: "#162642"
            border { color: "#2a4066"; width: 1 }

            RowLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 4

                Button {
                    implicitHeight: commsRoot.controlHeight
                    text: "Overview"
                    checkable: true
                    checked: commsRoot.activePage === 0
                    onClicked: commsRoot.activePage = 0
                }
                Button {
                    implicitHeight: commsRoot.controlHeight
                    text: "Operations"
                    checkable: true
                    checked: commsRoot.activePage === 1
                    onClicked: commsRoot.activePage = 1
                }
                Button {
                    implicitHeight: commsRoot.controlHeight
                    text: "Monitoring"
                    checkable: true
                    checked: commsRoot.activePage === 2
                    onClicked: commsRoot.activePage = 2
                }

                Item { Layout.fillWidth: true }

                Label {
                    text: "Active Link: " + commsManager.activeLink + " | Standby: " + commsManager.standbyLink
                    color: "#99c7ff"
                    font.pixelSize: commsRoot.fontSmall
                }
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: commsRoot.activePage

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Repeater {
                            model: [
                                { title: "A2G LINK", signal: commsRoot.a2gSignal(), desc: "Telemetry + ATC relay" },
                                { title: "A2A LINK", signal: commsRoot.a2aSignal(), desc: "Traffic coordination" },
                                { title: "SATCOM LINK", signal: commsRoot.satcomSignal(), desc: "Beyond line-of-sight" }
                            ]

                            delegate: Rectangle {
                                Layout.fillWidth: true
                                Layout.preferredHeight: commsRoot.linkCardHeight
                                radius: 10
                                color: "#162642"
                                border { color: "#2a4066"; width: 1 }

                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    spacing: 4

                                    Label { text: modelData.title; color: "#66d9ff"; font.pixelSize: commsRoot.fontSection; font.bold: true }
                                    Label {
                                        text: "Status: " + commsRoot.linkStatus(modelData.signal)
                                        color: commsRoot.linkColor(modelData.signal)
                                        font.pixelSize: commsRoot.fontBody
                                        font.bold: true
                                    }
                                    ProgressBar { Layout.fillWidth: true; from: 0; to: 100; value: modelData.signal }
                                    Label { text: modelData.desc; color: "#9aa8c4"; font.pixelSize: commsRoot.fontSmall }
                                }
                            }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 6

                                Label { text: "RADIO STACK"; color: "#00ff88"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    DigitalReadout {
                                        parameterId: P661.P.P_COM1_ACTIVE
                                        label: "COM1 ACTIVE"
                                        decimalPlaces: 3
                                        suffix: " MHz"
                                        valueFontPixelSize: 18
                                        Layout.fillWidth: true
                                    }
                                    DigitalReadout {
                                        parameterId: P661.P.P_COM1_STANDBY
                                        label: "COM1 STANDBY"
                                        decimalPlaces: 3
                                        suffix: " MHz"
                                        valueFontPixelSize: 18
                                        Layout.fillWidth: true
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    DigitalReadout {
                                        parameterId: P661.P.P_NAV1_ACTIVE
                                        label: "NAV1 ACTIVE"
                                        decimalPlaces: 3
                                        suffix: " MHz"
                                        valueFontPixelSize: 18
                                        normalColor: "#00aaff"
                                        Layout.fillWidth: true
                                    }
                                    DigitalReadout {
                                        parameterId: P661.P.P_NAV1_STANDBY
                                        label: "NAV1 STANDBY"
                                        decimalPlaces: 3
                                        suffix: " MHz"
                                        valueFontPixelSize: 18
                                        normalColor: "#4488aa"
                                        Layout.fillWidth: true
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6

                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "COM XFER"
                                        onClicked: {
                                            var active = arinc661Bus.doubleValue(P661.P.P_COM1_ACTIVE, 118.0)
                                            var standby = arinc661Bus.doubleValue(P661.P.P_COM1_STANDBY, 121.9)
                                            arinc661Bus.setValue(P661.P.P_COM1_ACTIVE, standby)
                                            arinc661Bus.setValue(P661.P.P_COM1_STANDBY, active)
                                        }
                                    }

                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "NAV XFER"
                                        onClicked: {
                                            var active = arinc661Bus.doubleValue(P661.P.P_NAV1_ACTIVE, 112.3)
                                            var standby = arinc661Bus.doubleValue(P661.P.P_NAV1_STANDBY, 115.7)
                                            arinc661Bus.setValue(P661.P.P_NAV1_ACTIVE, standby)
                                            arinc661Bus.setValue(P661.P.P_NAV1_STANDBY, active)
                                        }
                                    }

                                    Item { Layout.fillWidth: true }

                                    Label {
                                        text: "Mode: " + commsManager.mode
                                        color: "#99c7ff"
                                        font.pixelSize: commsRoot.fontSmall
                                    }
                                }

                                Item { Layout.fillHeight: true }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 6

                                Label { text: "MQTT GATEWAY"; color: "#66d9ff"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Label { text: "Broker"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    TextField {
                                        implicitHeight: commsRoot.inputHeight
                                        Layout.fillWidth: true
                                        text: mqttClient.brokerHost
                                        enabled: !mqttClient.connected
                                        onEditingFinished: mqttClient.brokerHost = text
                                    }
                                    Label { text: "Port"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    TextField {
                                        implicitHeight: commsRoot.inputHeight
                                        Layout.preferredWidth: 78
                                        text: mqttClient.brokerPort.toString()
                                        enabled: !mqttClient.connected
                                        validator: IntValidator { bottom: 1; top: 65535 }
                                        onEditingFinished: mqttClient.brokerPort = parseInt(text)
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Label { text: "Topic"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    TextField {
                                        implicitHeight: commsRoot.inputHeight
                                        Layout.fillWidth: true
                                        text: mqttClient.topic
                                        enabled: !mqttClient.connected
                                        onEditingFinished: mqttClient.topic = text
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Connect"
                                        enabled: !mqttClient.connected && mqttClient.mqttAvailable
                                        onClicked: commsManager.connectGateway()
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Disconnect"
                                        enabled: mqttClient.connected
                                        onClicked: commsManager.disconnectGateway()
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Publish Snapshot"
                                        onClicked: {
                                            mqttClient.publishTelemetrySnapshot(
                                                        arinc661Bus.doubleValue(P661.P.P_ALTITUDE, 0),
                                                        arinc661Bus.doubleValue(P661.P.P_AIRSPEED, 0),
                                                        arinc661Bus.doubleValue(P661.P.P_LATITUDE, 0),
                                                        arinc661Bus.doubleValue(P661.P.P_LONGITUDE, 0),
                                                        arinc661Bus.stringValue(P661.P.P_FLIGHT_PHASE, "UNKNOWN"))
                                        }
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: mqttClient.statusText
                                    color: mqttClient.mqttAvailable ? "#9bd0ff" : "#ffaa00"
                                    font.pixelSize: commsRoot.fontSmall
                                    wrapMode: Text.WordWrap
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "TX: " + (mqttClient.lastPublishedMessage.length > 0 ? mqttClient.lastPublishedMessage : "(none)")
                                    color: "#88c9a8"
                                    font.pixelSize: commsRoot.fontSmall
                                    elide: Text.ElideRight
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "RX: " + (mqttClient.lastReceivedMessage.length > 0 ? mqttClient.lastReceivedMessage : "(none)")
                                    color: "#88a8d8"
                                    font.pixelSize: commsRoot.fontSmall
                                    elide: Text.ElideRight
                                }

                                Item { Layout.fillHeight: true }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: commsRoot.statusStripHeight
                        radius: 10
                        color: "#162642"
                        border { color: "#2a4066"; width: 1 }

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 6

                            ColumnLayout {
                                Layout.fillWidth: true
                                Label { text: "ATC UPDATE"; color: "#ffaa00"; font.pixelSize: commsRoot.fontSmall; font.bold: true }
                                Label { text: commsRoot.atcMessages[commsRoot.atcMessageIndex]; color: "#d8d8d8"; font.pixelSize: commsRoot.fontSmall; wrapMode: Text.WordWrap }
                            }
                            Rectangle { width: 1; Layout.fillHeight: true; color: "#243248" }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Label { text: "WEATHER UPDATE"; color: "#66d9ff"; font.pixelSize: commsRoot.fontSmall; font.bold: true }
                                Label { text: commsRoot.weatherMessages[commsRoot.weatherMessageIndex]; color: "#d8d8d8"; font.pixelSize: commsRoot.fontSmall; wrapMode: Text.WordWrap }
                            }
                            Rectangle { width: 1; Layout.fillHeight: true; color: "#243248" }
                            ColumnLayout {
                                Layout.fillWidth: true
                                Label { text: "LIVE TELEMETRY"; color: "#44ff88"; font.pixelSize: commsRoot.fontSmall; font.bold: true }
                                Label {
                                    text: "ALT " + arinc661Bus.doubleValue(P661.P.P_ALTITUDE, 0).toFixed(0) + " ft | SPD " + arinc661Bus.doubleValue(P661.P.P_AIRSPEED, 0).toFixed(0) + " kt"
                                    color: "#d8d8d8"
                                    font.pixelSize: commsRoot.fontSmall
                                }
                                Label {
                                    text: "POS " + arinc661Bus.doubleValue(P661.P.P_LATITUDE, 0).toFixed(3) + ", " + arinc661Bus.doubleValue(P661.P.P_LONGITUDE, 0).toFixed(3)
                                    color: "#d8d8d8"
                                    font.pixelSize: commsRoot.fontSmall
                                }
                            }
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: commsRoot.opsTopCardHeight
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 5

                                Label { text: "COMMAND CHANNEL"; color: "#ffbf66"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Command"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    TextField {
                                        implicitHeight: commsRoot.inputHeight
                                        Layout.fillWidth: true
                                        text: commsRoot.commandNameDraft
                                        onTextChanged: commsRoot.commandNameDraft = text
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Args JSON"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    TextField {
                                        implicitHeight: commsRoot.inputHeight
                                        Layout.fillWidth: true
                                        text: commsRoot.commandArgsDraft
                                        onTextChanged: commsRoot.commandArgsDraft = text
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Send Command"
                                        onClicked: {
                                            var args = commsRoot.parseCommandArgs(commsRoot.commandArgsDraft)
                                            commsManager.sendCommand(commsRoot.commandNameDraft, args)
                                        }
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Flush Queue"
                                        onClicked: commsManager.flushQueue()
                                    }
                                    Item { Layout.fillWidth: true }
                                    Label {
                                        text: commsManager.lastAckStatus
                                        color: commsManager.lastAckStatus.indexOf("NACK") >= 0 ? "#ff8888" : "#8cffb2"
                                        font.pixelSize: commsRoot.fontSmall
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Queue: " + commsManager.queueDepth + " | Pending: " + commsManager.pendingCommandCount + " | Retries: " + commsManager.retriesCount + " | Dropped: " + commsManager.droppedCount
                                    color: "#9fc2e8"
                                    font.pixelSize: commsRoot.fontSmall
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: commsRoot.opsTopCardHeight
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 5

                                Label { text: "SECURITY + MODE"; color: "#66d9ff"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "Mode"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    ComboBox {
                                        implicitHeight: commsRoot.inputHeight
                                        model: ["NORMAL", "DEGRADED", "EMERGENCY"]
                                        currentIndex: Math.max(0, model.indexOf(commsManager.mode))
                                        onActivated: commsManager.mode = currentText
                                        Layout.preferredWidth: 130
                                    }

                                    Label { text: "Security"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                    ComboBox {
                                        implicitHeight: commsRoot.inputHeight
                                        model: ["TLS-MUTUAL-AUTH", "TLS-SERVER-ONLY", "LAB-NO-TLS"]
                                        currentIndex: Math.max(0, model.indexOf(commsManager.securityProfile))
                                        onActivated: commsManager.securityProfile = currentText
                                        Layout.preferredWidth: 190
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    CheckBox {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "TLS"
                                        checked: commsManager.tlsEnabled
                                        onToggled: commsManager.tlsEnabled = checked
                                    }
                                    CheckBox {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Cert Pin"
                                        checked: commsManager.certPinningEnabled
                                        onToggled: commsManager.certPinningEnabled = checked
                                    }
                                    CheckBox {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Sign"
                                        checked: commsManager.payloadSigningEnabled
                                        onToggled: commsManager.payloadSigningEnabled = checked
                                    }
                                }

                                RowLayout {
                                    Layout.fillWidth: true
                                    spacing: 6
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Clear Fault"
                                        onClicked: commsManager.injectFault("CLEAR")
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "High Loss"
                                        onClicked: commsManager.injectFault("HIGH_LOSS")
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Outage"
                                        onClicked: commsManager.injectFault("BROKER_OUTAGE")
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Latency"
                                        onClicked: commsManager.injectFault("LATENCY_SPIKE")
                                    }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "TLS Failure"
                                        onClicked: commsManager.injectFault("TLS_FAILURE")
                                    }
                                }

                                Label {
                                    Layout.fillWidth: true
                                    text: "Fault: " + commsManager.faultProfile + " | " + (commsManager.lastFailoverReason.length > 0 ? commsManager.lastFailoverReason : "No failover yet")
                                    color: "#9fc2e8"
                                    font.pixelSize: commsRoot.fontSmall
                                    wrapMode: Text.WordWrap
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 10
                        color: "#162642"
                        border { color: "#2a4066"; width: 1 }

                        GridLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            columns: 4
                            columnSpacing: 8
                            rowSpacing: 6

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 8
                                color: "#111b2d"
                                border { color: "#284260"; width: 1 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    Label { text: "RTT"; color: "#88bbff"; font.pixelSize: commsRoot.fontSmall }
                                    Label { text: commsManager.rttMs.toFixed(1) + " ms"; color: "#f0f0f0"; font.pixelSize: commsRoot.fontMetric; font.bold: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 8
                                color: "#111b2d"
                                border { color: "#284260"; width: 1 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    Label { text: "JITTER"; color: "#88bbff"; font.pixelSize: commsRoot.fontSmall }
                                    Label { text: commsManager.jitterMs.toFixed(1) + " ms"; color: "#f0f0f0"; font.pixelSize: commsRoot.fontMetric; font.bold: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 8
                                color: "#111b2d"
                                border { color: "#284260"; width: 1 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    Label { text: "PACKET LOSS"; color: "#88bbff"; font.pixelSize: commsRoot.fontSmall }
                                    Label { text: commsManager.packetLossPct.toFixed(2) + " %"; color: "#f0f0f0"; font.pixelSize: commsRoot.fontMetric; font.bold: true }
                                }
                            }

                            Rectangle {
                                Layout.fillWidth: true
                                Layout.fillHeight: true
                                radius: 8
                                color: "#111b2d"
                                border { color: "#284260"; width: 1 }
                                ColumnLayout {
                                    anchors.fill: parent
                                    anchors.margins: 6
                                    Label { text: "THROUGHPUT"; color: "#88bbff"; font.pixelSize: commsRoot.fontSmall }
                                    Label { text: commsManager.throughputKbps.toFixed(1) + " kbps"; color: "#f0f0f0"; font.pixelSize: commsRoot.fontMetric; font.bold: true }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 6

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        spacing: 6

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 6

                                RowLayout {
                                    Layout.fillWidth: true
                                    Label { text: "EVENT TIMELINE"; color: "#ffaa00"; font.pixelSize: commsRoot.fontSection; font.bold: true }
                                    Item { Layout.fillWidth: true }
                                    Button {
                                        implicitHeight: commsRoot.controlHeight
                                        text: "Clear Logs"
                                        onClicked: commsManager.clearLogs()
                                    }
                                }

                                ListView {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    model: commsManager.eventTimeline
                                    clip: true
                                    spacing: 2
                                    delegate: Label {
                                        width: ListView.view.width
                                        text: modelData
                                        color: "#d6d6d6"
                                        font.pixelSize: commsRoot.fontSmall
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: "#162642"
                            border { color: "#2a4066"; width: 1 }

                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 6

                                Label { text: "TRAFFIC INSPECTOR"; color: "#66d9ff"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                                ListView {
                                    Layout.fillWidth: true
                                    Layout.fillHeight: true
                                    model: commsManager.trafficInspector
                                    clip: true
                                    spacing: 2
                                    delegate: Label {
                                        width: ListView.view.width
                                        text: modelData
                                        color: "#9bd0ff"
                                        font.pixelSize: commsRoot.fontSmall
                                        elide: Text.ElideRight
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.preferredHeight: commsRoot.monitorBottomHeight
                        radius: 10
                        color: "#162642"
                        border { color: "#2a4066"; width: 1 }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 6
                            spacing: 6

                            Label { text: "REPLAY + CONTRACT CHECK"; color: "#8cffb2"; font.pixelSize: commsRoot.fontSection; font.bold: true }

                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 6

                                Button {
                                    implicitHeight: commsRoot.controlHeight
                                    text: commsManager.recording ? "Stop Recording" : "Start Recording"
                                    onClicked: {
                                        if (commsManager.recording) {
                                            commsManager.stopRecording()
                                        } else {
                                            commsManager.startRecording()
                                        }
                                    }
                                }
                                Button {
                                    implicitHeight: commsRoot.controlHeight
                                    text: "Clear Recording"
                                    onClicked: commsManager.clearRecording()
                                }
                                Button {
                                    implicitHeight: commsRoot.controlHeight
                                    text: "Save Replay"
                                    onClicked: commsManager.saveReplay(commsRoot.replayPathDraft)
                                }
                                Button {
                                    implicitHeight: commsRoot.controlHeight
                                    text: "Load Replay"
                                    onClicked: commsManager.loadReplay(commsRoot.replayPathDraft)
                                }
                                Button {
                                    implicitHeight: commsRoot.controlHeight
                                    text: "Play Replay"
                                    onClicked: commsManager.playReplay(1.0)
                                }

                                Item { Layout.fillWidth: true }

                                Label {
                                    text: "Schema errors: " + commsManager.validationErrors
                                    color: commsManager.validationErrors > 0 ? "#ff8888" : "#88ffaa"
                                    font.pixelSize: commsRoot.fontSmall
                                }
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Label { text: "Replay Path"; color: "#b8c8e6"; font.pixelSize: commsRoot.fontSmall }
                                TextField {
                                    implicitHeight: commsRoot.inputHeight
                                    Layout.fillWidth: true
                                    text: commsRoot.replayPathDraft
                                    onTextChanged: commsRoot.replayPathDraft = text
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}




