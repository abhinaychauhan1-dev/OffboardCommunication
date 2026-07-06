/**
 * @file MqttTelemetryClient.cpp
 * @brief Implements the MQTT telemetry client used to publish and receive simulator data.
 *
 */

#include "MqttTelemetryClient.h"

#include <QJsonDocument>
#include <QJsonObject>

#if OFFBOARD_HAS_QT_MQTT
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttSubscription>
#endif

MqttTelemetryClient::MqttTelemetryClient(QObject* parent)
    : QObject(parent)
    , m_brokerHost(QStringLiteral("localhost"))
    , m_topic(QStringLiteral("avionics/offboard/telemetry"))
{
#if OFFBOARD_HAS_QT_MQTT
    m_client = new QMqttClient(this);

    connect(m_client, &QMqttClient::connected, this, [this]() {
        m_connected = true;
        setStatusText(QStringLiteral("Connected to MQTT broker."));
        m_client->subscribe(m_topic, 0);
        emit stateChanged();
    });

    connect(m_client, &QMqttClient::disconnected, this, [this]() {
        m_connected = false;
        setStatusText(QStringLiteral("Disconnected from MQTT broker."));
        emit stateChanged();
    });

    connect(m_client, &QMqttClient::messageReceived,
            this,
            [this](const QByteArray& message, const QMqttTopicName&) {
        m_lastReceivedMessage = QString::fromUtf8(message);
        emit stateChanged();
    });

    connect(m_client, &QMqttClient::stateChanged, this, [this](QMqttClient::ClientState state) {
        if (state == QMqttClient::Connecting) {
            setStatusText(QStringLiteral("Connecting to MQTT broker..."));
            emit stateChanged();
        }
    });

    connect(m_client,
            &QMqttClient::errorChanged,
            this,
            [this](QMqttClient::ClientError error) {
        if (error != QMqttClient::NoError) {
            m_connected = false;
            setStatusText(QStringLiteral("MQTT error: %1").arg(m_client->errorString()));
            emit stateChanged();
        }
    });

    setStatusText(QStringLiteral("MQTT ready."));
#else
    setStatusText(QStringLiteral("Qt MQTT module not available in this Offboard build."));
#endif
}

QString MqttTelemetryClient::brokerHost() const { return m_brokerHost; }

void MqttTelemetryClient::setBrokerHost(const QString& host)
{
    const QString trimmed = host.trimmed();
    if (trimmed.isEmpty() || trimmed == m_brokerHost) {
        return;
    }
    m_brokerHost = trimmed;
    emit stateChanged();
}

int MqttTelemetryClient::brokerPort() const { return m_brokerPort; }

void MqttTelemetryClient::setBrokerPort(int port)
{
    if (port <= 0 || port == m_brokerPort) {
        return;
    }
    m_brokerPort = port;
    emit stateChanged();
}

QString MqttTelemetryClient::topic() const { return m_topic; }

void MqttTelemetryClient::setTopic(const QString& topic)
{
    const QString trimmed = topic.trimmed();
    if (trimmed.isEmpty() || trimmed == m_topic) {
        return;
    }
    m_topic = trimmed;
    emit stateChanged();
}

bool MqttTelemetryClient::connected() const { return m_connected; }

bool MqttTelemetryClient::mqttAvailable() const
{
#if OFFBOARD_HAS_QT_MQTT
    return true;
#else
    return false;
#endif
}

QString MqttTelemetryClient::statusText() const { return m_statusText; }
QString MqttTelemetryClient::lastReceivedMessage() const { return m_lastReceivedMessage; }
QString MqttTelemetryClient::lastPublishedMessage() const { return m_lastPublishedMessage; }

void MqttTelemetryClient::connectToBroker()
{
#if OFFBOARD_HAS_QT_MQTT
    if (!m_client) {
        return;
    }

    if (m_connected) {
        setStatusText(QStringLiteral("Already connected."));
        emit stateChanged();
        return;
    }

    m_client->setHostname(m_brokerHost);
    m_client->setPort(m_brokerPort);
    m_client->connectToHost();
#else
    setStatusText(QStringLiteral("MQTT not available. Rebuild with Qt6::Mqtt installed."));
    emit stateChanged();
#endif
}

void MqttTelemetryClient::disconnectFromBroker()
{
#if OFFBOARD_HAS_QT_MQTT
    if (!m_client) {
        return;
    }

    m_client->disconnectFromHost();
#else
    setStatusText(QStringLiteral("MQTT not available in this build."));
    emit stateChanged();
#endif
}

void MqttTelemetryClient::publishMessage(const QString& payload)
{
    const QString text = payload.trimmed();
    if (text.isEmpty()) {
        return;
    }

#if OFFBOARD_HAS_QT_MQTT
    if (!m_client || !m_connected) {
        setStatusText(QStringLiteral("Cannot publish: MQTT broker is not connected."));
        emit stateChanged();
        return;
    }

    m_client->publish(m_topic, text.toUtf8(), 0, false);
    m_lastPublishedMessage = text;
    setStatusText(QStringLiteral("Published MQTT message."));
    emit stateChanged();
#else
    m_lastPublishedMessage = text;
    setStatusText(QStringLiteral("MQTT unavailable: payload staged locally only."));
    emit stateChanged();
#endif
}

void MqttTelemetryClient::publishTelemetrySnapshot(double altitude,
                                                   double airspeed,
                                                   double latitude,
                                                   double longitude,
                                                   const QString& phase)
{
    QJsonObject obj;
    obj.insert(QStringLiteral("altitudeFt"), altitude);
    obj.insert(QStringLiteral("airspeedKt"), airspeed);
    obj.insert(QStringLiteral("latitude"), latitude);
    obj.insert(QStringLiteral("longitude"), longitude);
    obj.insert(QStringLiteral("phase"), phase);
    obj.insert(QStringLiteral("source"), QStringLiteral("offboard-communication"));

    const QString payload = QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    publishMessage(payload);
}

void MqttTelemetryClient::setStatusText(const QString& text)
{
    m_statusText = text;
}
