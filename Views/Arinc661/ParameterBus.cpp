/**
 * @file ParameterBus.cpp
 * @brief Implements the ARINC 661 parameter store and notification bus for QML widgets.
 *
 */

#include "ParameterBus.h"

ParameterBus::ParameterBus(QObject *parent)
    : QObject(parent)
{
}

void ParameterBus::setValue(Arinc661::ParameterId id, const QVariant &value)
{
    // Only emit if value actually changed
    auto it = m_params.constFind(id);
    if (it != m_params.constEnd() && *it == value) {
        return; // no change
    }
    m_params[id] = value;
    emit parameterChanged(id);
}

QVariant ParameterBus::value(Arinc661::ParameterId id) const
{
    return m_params.value(id);
}

double ParameterBus::doubleValue(Arinc661::ParameterId id, double fallback) const
{
    auto it = m_params.constFind(id);
    if (it == m_params.constEnd())
        return fallback;
    return it->toDouble();
}

int ParameterBus::intValue(Arinc661::ParameterId id, int fallback) const
{
    auto it = m_params.constFind(id);
    if (it == m_params.constEnd())
        return fallback;
    return it->toInt();
}

bool ParameterBus::boolValue(Arinc661::ParameterId id, bool fallback) const
{
    auto it = m_params.constFind(id);
    if (it == m_params.constEnd())
        return fallback;
    return it->toBool();
}

QString ParameterBus::stringValue(Arinc661::ParameterId id, const QString &fallback) const
{
    auto it = m_params.constFind(id);
    if (it == m_params.constEnd())
        return fallback;
    return it->toString();
}

QVariantList ParameterBus::listValue(Arinc661::ParameterId id) const
{
    auto it = m_params.constFind(id);
    if (it == m_params.constEnd())
        return QVariantList();
    return it->toList();
}

bool ParameterBus::has(Arinc661::ParameterId id) const
{
    return m_params.contains(id);
}

void ParameterBus::sendCommand(Arinc661::ParameterId cmd, const QVariant &value)
{
    setValue(cmd, value);
}

void ParameterBus::showWidget(int uwi)
{
    setValue(Arinc661::CDS_CMD_SHOW_WIDGET, uwi);
}

void ParameterBus::hideWidget(int uwi)
{
    setValue(Arinc661::CDS_CMD_HIDE_WIDGET, uwi);
}

void ParameterBus::moveWidget(int uwi, double x, double y)
{
    QVariantList v = { uwi, x, y };
    setValue(Arinc661::CDS_CMD_MOVE_WIDGET, v);
}

void ParameterBus::resizeWidget(int uwi, double w, double h)
{
    QVariantList v = { uwi, w, h };
    setValue(Arinc661::CDS_CMD_RESIZE_WIDGET, v);
}
