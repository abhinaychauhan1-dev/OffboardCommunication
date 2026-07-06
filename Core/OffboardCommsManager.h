/**
 * @file OffboardCommsManager.h
 * @brief Provides reliability, failover, metrics, logging, and replay for offboard communication.
 */

#ifndef OFFBOARDCOMMSMANAGER_H
#define OFFBOARDCOMMSMANAGER_H

#include <QObject>
#include <QDateTime>
#include <QPointer>
#include <QVariantMap>
#include <QStringList>
#include <QVector>

class MqttTelemetryClient;

class OffboardCommsManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString activeLink READ activeLink NOTIFY stateChanged)
    Q_PROPERTY(QString standbyLink READ standbyLink NOTIFY stateChanged)
    Q_PROPERTY(QString mode READ mode WRITE setMode NOTIFY stateChanged)
    Q_PROPERTY(QString transportName READ transportName WRITE setTransportName NOTIFY stateChanged)
    Q_PROPERTY(QString schemaVersion READ schemaVersion WRITE setSchemaVersion NOTIFY stateChanged)
    Q_PROPERTY(QString deviceId READ deviceId WRITE setDeviceId NOTIFY stateChanged)
    Q_PROPERTY(QString securityProfile READ securityProfile WRITE setSecurityProfile NOTIFY stateChanged)
    Q_PROPERTY(bool tlsEnabled READ tlsEnabled WRITE setTlsEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool certPinningEnabled READ certPinningEnabled WRITE setCertPinningEnabled NOTIFY stateChanged)
    Q_PROPERTY(bool payloadSigningEnabled READ payloadSigningEnabled WRITE setPayloadSigningEnabled NOTIFY stateChanged)
    Q_PROPERTY(double rttMs READ rttMs NOTIFY stateChanged)
    Q_PROPERTY(double jitterMs READ jitterMs NOTIFY stateChanged)
    Q_PROPERTY(double packetLossPct READ packetLossPct NOTIFY stateChanged)
    Q_PROPERTY(double throughputKbps READ throughputKbps NOTIFY stateChanged)
    Q_PROPERTY(int queueDepth READ queueDepth NOTIFY stateChanged)
    Q_PROPERTY(int pendingCommandCount READ pendingCommandCount NOTIFY stateChanged)
    Q_PROPERTY(int retriesCount READ retriesCount NOTIFY stateChanged)
    Q_PROPERTY(int droppedCount READ droppedCount NOTIFY stateChanged)
    Q_PROPERTY(int validationErrors READ validationErrors NOTIFY stateChanged)
    Q_PROPERTY(QString lastFailoverReason READ lastFailoverReason NOTIFY stateChanged)
    Q_PROPERTY(QString lastAckStatus READ lastAckStatus NOTIFY stateChanged)
    Q_PROPERTY(QString faultProfile READ faultProfile WRITE setFaultProfile NOTIFY stateChanged)
    Q_PROPERTY(QStringList eventTimeline READ eventTimeline NOTIFY stateChanged)
    Q_PROPERTY(QStringList trafficInspector READ trafficInspector NOTIFY stateChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY stateChanged)

public:
    explicit OffboardCommsManager(QObject* parent = nullptr);

    QString activeLink() const;
    QString standbyLink() const;
    QString mode() const;
    void setMode(const QString& mode);

    QString transportName() const;
    void setTransportName(const QString& name);

    QString schemaVersion() const;
    void setSchemaVersion(const QString& version);

    QString deviceId() const;
    void setDeviceId(const QString& id);

    QString securityProfile() const;
    void setSecurityProfile(const QString& profile);

    bool tlsEnabled() const;
    void setTlsEnabled(bool enabled);

    bool certPinningEnabled() const;
    void setCertPinningEnabled(bool enabled);

    bool payloadSigningEnabled() const;
    void setPayloadSigningEnabled(bool enabled);

    double rttMs() const;
    double jitterMs() const;
    double packetLossPct() const;
    double throughputKbps() const;

    int queueDepth() const;
    int pendingCommandCount() const;
    int retriesCount() const;
    int droppedCount() const;
    int validationErrors() const;

    QString lastFailoverReason() const;
    QString lastAckStatus() const;

    QString faultProfile() const;
    void setFaultProfile(const QString& profile);

    QStringList eventTimeline() const;
    QStringList trafficInspector() const;

    bool recording() const;

    void setMqttClient(MqttTelemetryClient* client);

    Q_INVOKABLE void connectGateway();
    Q_INVOKABLE void disconnectGateway();
    Q_INVOKABLE QString sendCommand(const QString& commandName, const QVariantMap& args);
    Q_INVOKABLE void tickTelemetry(const QVariantMap& telemetry);
    Q_INVOKABLE void updateLinkSignals(double a2gSignal, double a2aSignal, double satcomSignal);
    Q_INVOKABLE void forceFailover(const QString& preferredLink);
    Q_INVOKABLE void flushQueue();
    Q_INVOKABLE void clearLogs();

    Q_INVOKABLE void injectFault(const QString& faultName);

    Q_INVOKABLE void startRecording();
    Q_INVOKABLE void stopRecording();
    Q_INVOKABLE void clearRecording();
    Q_INVOKABLE bool saveReplay(const QString& filePath);
    Q_INVOKABLE bool loadReplay(const QString& filePath);
    Q_INVOKABLE void playReplay(double speed);

signals:
    void stateChanged();

private:
    struct QueuedMessage {
        QString id;
        QString payload;
        QString topic;
        QString priority;
        bool command = false;
        bool expectAck = false;
        int retryCount = 0;
        qint64 enqueueMs = 0;
    };

    QString nowStamp() const;
    void pushEvent(const QString& text);
    void pushTraffic(const QString& text);
    QString nextId(const QString& prefix);

    bool publishNow(const QueuedMessage& msg);
    void queueMessage(const QueuedMessage& msg);
    void handlePendingCommands();
    void maybeFailover();
    void recalcMetrics();
    bool validateEnvelope(const QVariantMap& envelope);

    QVector<QueuedMessage> m_queue;
    QVector<QueuedMessage> m_pendingCommands;
    QVector<QVariantMap> m_replayRecords;

    QPointer<MqttTelemetryClient> m_mqttClient;

    QString m_activeLink = QStringLiteral("A2G");
    QString m_standbyLink = QStringLiteral("SATCOM");
    QString m_mode = QStringLiteral("NORMAL");
    QString m_transportName = QStringLiteral("MQTT");
    QString m_schemaVersion = QStringLiteral("1.0");
    QString m_deviceId = QStringLiteral("OFFBOARD-SIM-001");
    QString m_securityProfile = QStringLiteral("TLS-MUTUAL-AUTH");
    QString m_faultProfile = QStringLiteral("CLEAR");

    bool m_tlsEnabled = true;
    bool m_certPinningEnabled = true;
    bool m_payloadSigningEnabled = false;
    bool m_recording = false;

    double m_rttMs = 45.0;
    double m_jitterMs = 6.0;
    double m_packetLossPct = 0.2;
    double m_throughputKbps = 0.0;

    double m_signalA2G = 70.0;
    double m_signalA2A = 60.0;
    double m_signalSatcom = 75.0;

    int m_retriesCount = 0;
    int m_droppedCount = 0;
    int m_validationErrors = 0;

    QString m_lastFailoverReason;
    QString m_lastAckStatus = QStringLiteral("No commands sent");

    QStringList m_eventTimeline;
    QStringList m_trafficInspector;

    quint64 m_sequence = 0;
};

#endif
