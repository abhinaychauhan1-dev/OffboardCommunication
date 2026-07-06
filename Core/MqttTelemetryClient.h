/**
 * @file MqttTelemetryClient.h
 * @brief Declares the MQTT telemetry client that connects the simulator to a broker.
 *
 * Developed by Abhinay Chauhan
 */

#ifndef MQTTTELEMETRYCLIENT_H
#define MQTTTELEMETRYCLIENT_H

#include <QObject>
#include <QString>

#if OFFBOARD_HAS_QT_MQTT
class QMqttClient;
#endif

class MqttTelemetryClient : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString brokerHost READ brokerHost WRITE setBrokerHost NOTIFY stateChanged)
    Q_PROPERTY(int brokerPort READ brokerPort WRITE setBrokerPort NOTIFY stateChanged)
    Q_PROPERTY(QString topic READ topic WRITE setTopic NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY stateChanged)
    Q_PROPERTY(bool mqttAvailable READ mqttAvailable CONSTANT)
    Q_PROPERTY(QString statusText READ statusText NOTIFY stateChanged)
    Q_PROPERTY(QString lastReceivedMessage READ lastReceivedMessage NOTIFY stateChanged)
    Q_PROPERTY(QString lastPublishedMessage READ lastPublishedMessage NOTIFY stateChanged)

public:
    explicit MqttTelemetryClient(QObject* parent = nullptr);

    QString brokerHost() const;
    void setBrokerHost(const QString& host);

    int brokerPort() const;
    void setBrokerPort(int port);

    QString topic() const;
    void setTopic(const QString& topic);

    bool connected() const;
    bool mqttAvailable() const;

    QString statusText() const;
    QString lastReceivedMessage() const;
    QString lastPublishedMessage() const;

    Q_INVOKABLE void connectToBroker();
    Q_INVOKABLE void disconnectFromBroker();
    Q_INVOKABLE void publishMessage(const QString& payload);
    Q_INVOKABLE void publishTelemetrySnapshot(double altitude,
                                              double airspeed,
                                              double latitude,
                                              double longitude,
                                              const QString& phase);

signals:
    void stateChanged();

private:
    void setStatusText(const QString& text);

    QString m_brokerHost;
    int m_brokerPort = 1883;
    QString m_topic;
    bool m_connected = false;
    QString m_statusText;
    QString m_lastReceivedMessage;
    QString m_lastPublishedMessage;

#if OFFBOARD_HAS_QT_MQTT
    QMqttClient* m_client = nullptr;
#endif
};

#endif
