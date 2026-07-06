/*
 * Hosts the standalone Offboard Communication view.
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "Arinc661/Layers"

ApplicationWindow {
    id: root
    width: 1240
    height: 700
    visible: true
    title: "Offboard Communication"
    color: "#111826"

    header: Rectangle {
        color: "#0b1220"
        height: 48
        border.color: "#1f3552"
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 12

            Label {
                text: "OFFBOARD COMMUNICATION STATION"
                color: "#72e3ff"
                font.pixelSize: 16
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                radius: 6
                color: "#13243a"
                border.color: "#325e87"
                border.width: 1
                implicitWidth: 360
                implicitHeight: 30

                Label {
                    anchors.centerIn: parent
                    text: "Use the MQTT panel to connect and publish snapshots"
                    color: "#9bc6ee"
                    font.pixelSize: 12
                }
            }
        }
    }

    CommsLayer {
        anchors.fill: parent
        anchors.margins: 0
    }
}
