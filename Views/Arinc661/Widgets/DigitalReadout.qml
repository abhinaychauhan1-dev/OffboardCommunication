/*
 * Provides a reusable digital readout widget for ARINC 661 cockpit data.
 * Developed by Abhinay Chauhan
 */

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../ParameterBus.js" as Arinc661

/**
 * @brief ARINC 661 Digital Readout widget.
 *
 * Displays a numeric or string value from the parameter bus with
 * configurable formatting, prefix, suffix, and color thresholds.
 */
Rectangle {
    id: root
    clip: true

    // ---- ARINC 661 Parameter Binding ----
    /// The parameter ID to display
    property int parameterId: 0

    /// Auto-bind when parameterId is set
    onParameterIdChanged: { if (parameterId) updateFromBus() }

    // ---- Display Configuration ----
    property string label: ""
    property string prefix: ""
    property string suffix: ""
    property int decimalPlaces: 0
    property string format: "number"  // "number", "string", "time"
    property var thresholdMap: ({})     // { min: color, max: color, ... }
    property int valueFontPixelSize: 22
    property int headerFontPixelSize: 10
    property int valueMinimumPixelSize: 11
    property int contentMargin: 8
    property int verticalSpacing: 2
    property int verticalPadding: 1

    property color normalColor: "#ffffff"
    property color activeColor: normalColor

    // ---- Internals ----
    property double _numValue: 0
    property string _strValue: ""
    property double _min: 0
    property double _max: 99999

    // ---- Styling ----
    color: "#10192f"
    radius: 8
    border { color: "#2a4066"; width: 1 }

    // ARINC 661 bus subscription
    Connections {
        target: arinc661Bus
        function onParameterChanged(id) {
            if (id === root.parameterId) {
                root.updateFromBus()
            }
        }
    }

    function updateFromBus() {
        if (!root.parameterId || !arinc661Bus) return
        if (root.format === "string") {
            _strValue = arinc661Bus.stringValue(root.parameterId, "")
        } else {
            _numValue = arinc661Bus.doubleValue(root.parameterId, 0)
        }
        applyColorThreshold()
    }

    function applyColorThreshold() {
        if (!thresholdMap || Object.keys(thresholdMap).length === 0) {
            activeColor = normalColor
            return
        }
        activeColor = normalColor
        for (var key in thresholdMap) {
            var parts = key.split(",")
            for (var i = 0; i < parts.length; i++) {
                var range = parts[i].trim()
                var op = range.charAt(0)
                var val = parseFloat(range.substring(1))
                if (op === "<" && _numValue < val) {
                    activeColor = thresholdMap[key]
                    return
                } else if (op === ">" && _numValue > val) {
                    activeColor = thresholdMap[key]
                    return
                }
            }
        }
    }

    Component.onCompleted: updateFromBus()

    // ---- Layout ----
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.contentMargin
        spacing: root.verticalSpacing

        Label {
            text: root.label
            font.pixelSize: root.headerFontPixelSize
            color: "#888888"
            visible: root.label.length > 0
        }

        Item { Layout.preferredHeight: root.verticalPadding }

        Text {
            Layout.fillWidth: true
            Layout.minimumHeight: 20
            text: {
                if (root.format === "string")
                    return root._strValue
                return root.prefix + root._numValue.toFixed(root.decimalPlaces) + root.suffix
            }
            font.pixelSize: root.valueFontPixelSize
            minimumPixelSize: root.valueMinimumPixelSize
            fontSizeMode: Text.Fit
            font.bold: true
            color: root.activeColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            clip: true
        }

        Item { Layout.preferredHeight: root.verticalPadding }
    }
}
