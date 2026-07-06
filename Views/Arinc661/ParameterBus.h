/**
 * @file ParameterBus.h
 * @brief Declares the ARINC 661 parameter bus shared by cockpit widgets and backend code.
 *
 * Developed by Abhinay Chauhan
 */

#ifndef PARAMETERBUS_H
#define PARAMETERBUS_H

#include <QObject>
#include <QVariant>
#include <QHash>
#include <QList>

#include "Arinc661Types.h"

/**
 * @brief ARINC 661 ParameterBus â€” the central data bus between the
 *        User Application (C++ backend) and Cockpit Display System (QML).
 *
 * The bus holds a flat parameter space identified by Arinc661::ParameterId
 * integers.  The User Application calls setValue() for each tick, which
 * emits parameterChanged(id) so that QML widgets subscribed to that ID
 * update reactively.
 *
 * Usage in QML:
 *   id: myWidget
 *   onBind: arinc661Bus.subscribe(myWidget, Arinc661.P_ALTITUDE)
 *   property double altitude  // updated by ARINC 661 bus callback
 *
 * Usage in C++:
 *   ParameterBus *bus = ...;
 *   bus->setValue(Arinc661::P_ALTITUDE, flightData.altitude);
 *   bus->setValue(Arinc661::P_HEADING,  flightData.heading);
 */
class ParameterBus : public QObject
{
    Q_OBJECT

public:
    explicit ParameterBus(QObject *parent = nullptr);

    // ---- Value access ----
    /// Set a parameter (int, double, bool, QString, or QList<double>).
    Q_INVOKABLE void setValue(Arinc661::ParameterId id, const QVariant &value);

    /// Get the current value of a parameter (QVariant).
    Q_INVOKABLE QVariant value(Arinc661::ParameterId id) const;

    /// Get as a specific type (convenience).
    Q_INVOKABLE double doubleValue(Arinc661::ParameterId id, double fallback = 0.0) const;
    Q_INVOKABLE int intValue(Arinc661::ParameterId id, int fallback = 0) const;
    Q_INVOKABLE bool boolValue(Arinc661::ParameterId id, bool fallback = false) const;
    Q_INVOKABLE QString stringValue(Arinc661::ParameterId id, const QString &fallback = QString()) const;
    Q_INVOKABLE QVariantList listValue(Arinc661::ParameterId id) const;

    /// Query whether a parameter has been set at all.
    Q_INVOKABLE bool has(Arinc661::ParameterId id) const;

    // ---- CDS Commands ----
    /// Send a command to the CDS (e.g., show/hide/move widgets)
    Q_INVOKABLE void sendCommand(Arinc661::ParameterId cmd, const QVariant &value);

    /// Convenience: show a widget by UWI
    Q_INVOKABLE void showWidget(int uwi);
    /// Convenience: hide a widget by UWI
    Q_INVOKABLE void hideWidget(int uwi);
    /// Convenience: move a widget
    Q_INVOKABLE void moveWidget(int uwi, double x, double y);
    /// Convenience: resize a widget
    Q_INVOKABLE void resizeWidget(int uwi, double w, double h);

signals:
    /** Emitted whenever a parameter's value changes. Widgets connect here. */
    void parameterChanged(Arinc661::ParameterId id);

private:
    QHash<Arinc661::ParameterId, QVariant> m_params;
};

#endif // PARAMETERBUS_H
