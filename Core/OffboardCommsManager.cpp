/**
 * @file OffboardCommsManager.cpp
 * @brief Implements reliability features for offboard communication workflows.
 */

#include "OffboardCommsManager.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <utility>

#include "MqttTelemetryClient.h"

OffboardCommsManager::OffboardCommsManager(QObject* parent)
    : QObject(parent)
{
    pushEvent(QStringLiteral("Comms manager initialized in NORMAL mode."));
}

QString OffboardCommsManager::activeLink() const { return m_activeLink; }
QString OffboardCommsManager::standbyLink() const { return m_standbyLink; }
QString OffboardCommsManager::mode() const { return m_mode; }
QString OffboardCommsManager::transportName() const { return m_transportName; }
QString OffboardCommsManager::schemaVersion() const { return m_schemaVersion; }
QString OffboardCommsManager::deviceId() const { return m_deviceId; }
QString OffboardCommsManager::securityProfile() const { return m_securityProfile; }
bool OffboardCommsManager::tlsEnabled() const { return m_tlsEnabled; }
bool OffboardCommsManager::certPinningEnabled() const { return m_certPinningEnabled; }
bool OffboardCommsManager::payloadSigningEnabled() const { return m_payloadSigningEnabled; }
double OffboardCommsManager::rttMs() const { return m_rttMs; }
double OffboardCommsManager::jitterMs() const { return m_jitterMs; }
double OffboardCommsManager::packetLossPct() const { return m_packetLossPct; }
double OffboardCommsManager::throughputKbps() const { return m_throughputKbps; }
int OffboardCommsManager::queueDepth() const { return m_queue.size(); }
int OffboardCommsManager::pendingCommandCount() const { return m_pendingCommands.size(); }
int OffboardCommsManager::retriesCount() const { return m_retriesCount; }
int OffboardCommsManager::droppedCount() const { return m_droppedCount; }
int OffboardCommsManager::validationErrors() const { return m_validationErrors; }
QString OffboardCommsManager::lastFailoverReason() const { return m_lastFailoverReason; }
QString OffboardCommsManager::lastAckStatus() const { return m_lastAckStatus; }
QString OffboardCommsManager::faultProfile() const { return m_faultProfile; }
QStringList OffboardCommsManager::eventTimeline() const { return m_eventTimeline; }
QStringList OffboardCommsManager::trafficInspector() const { return m_trafficInspector; }
bool OffboardCommsManager::recording() const { return m_recording; }

void OffboardCommsManager::setMode(const QString& mode)
{
    const QString upper = mode.trimmed().toUpper();
    if (upper.isEmpty() || upper == m_mode) {
        return;
    }

    m_mode = upper;
    if (m_mode == QStringLiteral("EMERGENCY")) {
        m_packetLossPct = qMax(0.0, m_packetLossPct - 0.05);
        m_jitterMs = qMax(1.0, m_jitterMs - 1.5);
    } else if (m_mode == QStringLiteral("DEGRADED")) {
        m_packetLossPct = qMax(0.2, m_packetLossPct);
    }

    pushEvent(QStringLiteral("Mode changed to %1.").arg(m_mode));
    emit stateChanged();
}

void OffboardCommsManager::setTransportName(const QString& name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || trimmed == m_transportName) {
        return;
    }
    m_transportName = trimmed;
    pushEvent(QStringLiteral("Transport set to %1.").arg(m_transportName));
    emit stateChanged();
}

void OffboardCommsManager::setSchemaVersion(const QString& version)
{
    const QString trimmed = version.trimmed();
    if (trimmed.isEmpty() || trimmed == m_schemaVersion) {
        return;
    }
    m_schemaVersion = trimmed;
    pushEvent(QStringLiteral("Schema version updated to %1.").arg(m_schemaVersion));
    emit stateChanged();
}

void OffboardCommsManager::setDeviceId(const QString& id)
{
    const QString trimmed = id.trimmed();
    if (trimmed.isEmpty() || trimmed == m_deviceId) {
        return;
    }
    m_deviceId = trimmed;
    emit stateChanged();
}

void OffboardCommsManager::setSecurityProfile(const QString& profile)
{
    const QString trimmed = profile.trimmed();
    if (trimmed.isEmpty() || trimmed == m_securityProfile) {
        return;
    }
    m_securityProfile = trimmed;
    pushEvent(QStringLiteral("Security profile set to %1.").arg(m_securityProfile));
    emit stateChanged();
}

void OffboardCommsManager::setTlsEnabled(bool enabled)
{
    if (m_tlsEnabled == enabled) {
        return;
    }
    m_tlsEnabled = enabled;
    emit stateChanged();
}

void OffboardCommsManager::setCertPinningEnabled(bool enabled)
{
    if (m_certPinningEnabled == enabled) {
        return;
    }
    m_certPinningEnabled = enabled;
    emit stateChanged();
}

void OffboardCommsManager::setPayloadSigningEnabled(bool enabled)
{
    if (m_payloadSigningEnabled == enabled) {
        return;
    }
    m_payloadSigningEnabled = enabled;
    emit stateChanged();
}

void OffboardCommsManager::setFaultProfile(const QString& profile)
{
    const QString upper = profile.trimmed().toUpper();
    if (upper.isEmpty() || upper == m_faultProfile) {
        return;
    }
    m_faultProfile = upper;
    injectFault(m_faultProfile);
}

void OffboardCommsManager::setMqttClient(MqttTelemetryClient* client)
{
    m_mqttClient = client;
}

void OffboardCommsManager::connectGateway()
{
    if (m_mqttClient) {
        m_mqttClient->connectToBroker();
    }
    pushEvent(QStringLiteral("Gateway connection requested."));
    emit stateChanged();
}

void OffboardCommsManager::disconnectGateway()
{
    if (m_mqttClient) {
        m_mqttClient->disconnectFromBroker();
    }
    pushEvent(QStringLiteral("Gateway disconnect requested."));
    emit stateChanged();
}

QString OffboardCommsManager::sendCommand(const QString& commandName, const QVariantMap& args)
{
    const QString id = nextId(QStringLiteral("cmd"));

    QVariantMap cmd;
    cmd.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    cmd.insert(QStringLiteral("deviceId"), m_deviceId);
    cmd.insert(QStringLiteral("transport"), m_transportName);
    cmd.insert(QStringLiteral("type"), QStringLiteral("command"));
    cmd.insert(QStringLiteral("name"), commandName);
    cmd.insert(QStringLiteral("args"), args);
    cmd.insert(QStringLiteral("priority"), QStringLiteral("high"));
    cmd.insert(QStringLiteral("correlationId"), id);
    cmd.insert(QStringLiteral("timestampUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));

    if (!validateEnvelope(cmd)) {
        ++m_validationErrors;
        m_lastAckStatus = QStringLiteral("Validation error for command %1").arg(id);
        pushEvent(m_lastAckStatus);
        emit stateChanged();
        return id;
    }

    const QString payload = QString::fromUtf8(QJsonDocument::fromVariant(cmd).toJson(QJsonDocument::Compact));
    QueuedMessage msg;
    msg.id = id;
    msg.payload = payload;
    msg.topic = QStringLiteral("avionics/offboard/command");
    msg.priority = QStringLiteral("high");
    msg.command = true;
    msg.expectAck = true;
    msg.enqueueMs = QDateTime::currentMSecsSinceEpoch();

    queueMessage(msg);
    pushEvent(QStringLiteral("Queued command %1 (%2).")
                      .arg(id, commandName.trimmed().isEmpty() ? QStringLiteral("unnamed") : commandName));

    flushQueue();
    handlePendingCommands();
    emit stateChanged();
    return id;
}

void OffboardCommsManager::tickTelemetry(const QVariantMap& telemetry)
{
    QVariantMap envelope;
    envelope.insert(QStringLiteral("schemaVersion"), m_schemaVersion);
    envelope.insert(QStringLiteral("deviceId"), m_deviceId);
    envelope.insert(QStringLiteral("transport"), m_transportName);
    envelope.insert(QStringLiteral("type"), QStringLiteral("telemetry"));
    envelope.insert(QStringLiteral("sequence"), static_cast<qint64>(++m_sequence));
    envelope.insert(QStringLiteral("timestampUtc"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
    envelope.insert(QStringLiteral("mode"), m_mode);
    envelope.insert(QStringLiteral("activeLink"), m_activeLink);
    envelope.insert(QStringLiteral("payload"), telemetry);

    if (!validateEnvelope(envelope)) {
        ++m_validationErrors;
        pushEvent(QStringLiteral("Telemetry envelope rejected by schema guard."));
        emit stateChanged();
        return;
    }

    const QString payload = QString::fromUtf8(QJsonDocument::fromVariant(envelope).toJson(QJsonDocument::Compact));

    QueuedMessage msg;
    msg.id = nextId(QStringLiteral("tx"));
    msg.payload = payload;
    msg.topic = QStringLiteral("avionics/offboard/telemetry");
    msg.priority = (m_mode == QStringLiteral("EMERGENCY")) ? QStringLiteral("critical") : QStringLiteral("normal");
    msg.enqueueMs = QDateTime::currentMSecsSinceEpoch();

    queueMessage(msg);
    flushQueue();
    handlePendingCommands();
    recalcMetrics();
    emit stateChanged();
}

void OffboardCommsManager::updateLinkSignals(double a2gSignal, double a2aSignal, double satcomSignal)
{
    m_signalA2G = qBound(0.0, a2gSignal, 100.0);
    m_signalA2A = qBound(0.0, a2aSignal, 100.0);
    m_signalSatcom = qBound(0.0, satcomSignal, 100.0);

    maybeFailover();
    recalcMetrics();
    emit stateChanged();
}

void OffboardCommsManager::forceFailover(const QString& preferredLink)
{
    const QString wanted = preferredLink.trimmed().toUpper();
    if (wanted.isEmpty()) {
        return;
    }

    if (wanted == m_activeLink) {
        return;
    }

    m_standbyLink = m_activeLink;
    m_activeLink = wanted;
    m_lastFailoverReason = QStringLiteral("Manual failover to %1").arg(wanted);
    pushEvent(m_lastFailoverReason);
    emit stateChanged();
}

void OffboardCommsManager::flushQueue()
{
    QVector<QueuedMessage> remain;
    remain.reserve(m_queue.size());

    for (const QueuedMessage& msg : std::as_const(m_queue)) {
        if (publishNow(msg)) {
            if (msg.expectAck) {
                m_pendingCommands.push_back(msg);
            }
        } else {
            QueuedMessage retried = msg;
            retried.retryCount += 1;

            if (retried.retryCount > 3) {
                ++m_droppedCount;
                pushEvent(QStringLiteral("Dropped %1 after retry budget exhausted.").arg(retried.id));
                continue;
            }

            ++m_retriesCount;
            remain.push_back(retried);
        }
    }

    m_queue = remain;
    emit stateChanged();
}

void OffboardCommsManager::clearLogs()
{
    m_eventTimeline.clear();
    m_trafficInspector.clear();
    emit stateChanged();
}

void OffboardCommsManager::injectFault(const QString& faultName)
{
    const QString fault = faultName.trimmed().toUpper();
    m_faultProfile = fault;

    if (fault == QStringLiteral("CLEAR")) {
        m_packetLossPct = 0.2;
        m_rttMs = 45.0;
        m_jitterMs = 6.0;
        m_lastFailoverReason = QStringLiteral("No active fault profile.");
    } else if (fault == QStringLiteral("HIGH_LOSS")) {
        m_packetLossPct = 22.0;
        m_jitterMs = 32.0;
        m_lastFailoverReason = QStringLiteral("High packet loss injected.");
    } else if (fault == QStringLiteral("BROKER_OUTAGE")) {
        m_packetLossPct = 100.0;
        m_rttMs = 600.0;
        m_lastFailoverReason = QStringLiteral("Broker outage injected.");
    } else if (fault == QStringLiteral("LATENCY_SPIKE")) {
        m_rttMs = 350.0;
        m_jitterMs = 80.0;
        m_lastFailoverReason = QStringLiteral("Latency spike injected.");
    } else if (fault == QStringLiteral("TLS_FAILURE")) {
        m_tlsEnabled = false;
        m_certPinningEnabled = false;
        m_lastFailoverReason = QStringLiteral("TLS failure injected.");
    }

    pushEvent(QStringLiteral("Fault profile set to %1.").arg(m_faultProfile));
    maybeFailover();
    recalcMetrics();
    emit stateChanged();
}

void OffboardCommsManager::startRecording()
{
    if (m_recording) {
        return;
    }
    m_recording = true;
    pushEvent(QStringLiteral("Replay recording started."));
    emit stateChanged();
}

void OffboardCommsManager::stopRecording()
{
    if (!m_recording) {
        return;
    }
    m_recording = false;
    pushEvent(QStringLiteral("Replay recording stopped."));
    emit stateChanged();
}

void OffboardCommsManager::clearRecording()
{
    m_replayRecords.clear();
    pushEvent(QStringLiteral("Replay recording buffer cleared."));
    emit stateChanged();
}

bool OffboardCommsManager::saveReplay(const QString& filePath)
{
    if (filePath.trimmed().isEmpty()) {
        return false;
    }

    QJsonArray arr;
    for (const QVariantMap& row : std::as_const(m_replayRecords)) {
        arr.append(QJsonObject::fromVariantMap(row));
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        pushEvent(QStringLiteral("Replay save failed: cannot open %1").arg(filePath));
        emit stateChanged();
        return false;
    }

    const qint64 written = file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    file.close();

    if (written <= 0) {
        pushEvent(QStringLiteral("Replay save failed: write error."));
        emit stateChanged();
        return false;
    }

    pushEvent(QStringLiteral("Replay saved to %1.").arg(filePath));
    emit stateChanged();
    return true;
}

bool OffboardCommsManager::loadReplay(const QString& filePath)
{
    if (filePath.trimmed().isEmpty()) {
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        pushEvent(QStringLiteral("Replay load failed: cannot open %1").arg(filePath));
        emit stateChanged();
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    file.close();

    if (parseError.error != QJsonParseError::NoError || !doc.isArray()) {
        pushEvent(QStringLiteral("Replay load failed: invalid JSON format."));
        emit stateChanged();
        return false;
    }

    m_replayRecords.clear();
    const QJsonArray arr = doc.array();
    for (const QJsonValue& v : arr) {
        if (v.isObject()) {
            m_replayRecords.push_back(v.toObject().toVariantMap());
        }
    }

    pushEvent(QStringLiteral("Replay loaded: %1 frames.").arg(m_replayRecords.size()));
    emit stateChanged();
    return true;
}

void OffboardCommsManager::playReplay(double speed)
{
    Q_UNUSED(speed)

    if (m_replayRecords.isEmpty()) {
        pushEvent(QStringLiteral("Replay play ignored: buffer is empty."));
        emit stateChanged();
        return;
    }

    for (const QVariantMap& row : std::as_const(m_replayRecords)) {
        const QString payload = QString::fromUtf8(QJsonDocument::fromVariant(row).toJson(QJsonDocument::Compact));
        pushTraffic(QStringLiteral("%1 RX-REPLAY %2").arg(nowStamp(), payload));
    }

    pushEvent(QStringLiteral("Replay played (%1 frames).").arg(m_replayRecords.size()));
    emit stateChanged();
}

QString OffboardCommsManager::nowStamp() const
{
    return QDateTime::currentDateTime().toString(QStringLiteral("HH:mm:ss.zzz"));
}

void OffboardCommsManager::pushEvent(const QString& text)
{
    m_eventTimeline.prepend(QStringLiteral("%1  %2").arg(nowStamp(), text));
    while (m_eventTimeline.size() > 120) {
        m_eventTimeline.removeLast();
    }
}

void OffboardCommsManager::pushTraffic(const QString& text)
{
    m_trafficInspector.prepend(text);
    while (m_trafficInspector.size() > 120) {
        m_trafficInspector.removeLast();
    }

    if (m_recording) {
        QVariantMap row;
        row.insert(QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs));
        row.insert(QStringLiteral("line"), text);
        m_replayRecords.push_back(row);
    }
}

QString OffboardCommsManager::nextId(const QString& prefix)
{
    return QStringLiteral("%1-%2").arg(prefix, QString::number(++m_sequence));
}

bool OffboardCommsManager::publishNow(const QueuedMessage& msg)
{
    const bool brokerOutage = m_faultProfile == QStringLiteral("BROKER_OUTAGE");
    const bool tlsFailure = m_faultProfile == QStringLiteral("TLS_FAILURE");

    const double lossProbability = m_packetLossPct / 100.0;
    const bool dropped = brokerOutage || (QRandomGenerator::global()->generateDouble() < lossProbability);

    if (tlsFailure && m_tlsEnabled) {
        pushEvent(QStringLiteral("Blocked %1 due to TLS failure.").arg(msg.id));
        return false;
    }

    if (dropped) {
        pushEvent(QStringLiteral("Transport drop simulated for %1.").arg(msg.id));
        return false;
    }

    if (m_mqttClient) {
        m_mqttClient->publishMessage(msg.payload);
    }

    pushTraffic(QStringLiteral("%1 TX[%2] %3")
                        .arg(nowStamp(), msg.topic, msg.payload));
    return true;
}

void OffboardCommsManager::queueMessage(const QueuedMessage& msg)
{
    if (msg.priority == QStringLiteral("critical") || msg.priority == QStringLiteral("high")) {
        m_queue.prepend(msg);
    } else {
        m_queue.push_back(msg);
    }
}

void OffboardCommsManager::handlePendingCommands()
{
    QVector<QueuedMessage> remain;

    for (const QueuedMessage& cmd : std::as_const(m_pendingCommands)) {
        const bool nack = (QRandomGenerator::global()->bounded(100) < static_cast<int>(m_packetLossPct));

        if (nack) {
            m_lastAckStatus = QStringLiteral("NACK for %1").arg(cmd.id);
            pushEvent(m_lastAckStatus);
            if (cmd.retryCount < 2) {
                QueuedMessage retried = cmd;
                retried.retryCount += 1;
                queueMessage(retried);
                ++m_retriesCount;
            } else {
                ++m_droppedCount;
            }
        } else {
            m_lastAckStatus = QStringLiteral("ACK for %1").arg(cmd.id);
            pushEvent(m_lastAckStatus);
            pushTraffic(QStringLiteral("%1 RX[ack] {\"correlationId\":\"%2\",\"status\":\"ACK\"}")
                                .arg(nowStamp(), cmd.id));
        }
    }

    m_pendingCommands = remain;
}

void OffboardCommsManager::maybeFailover()
{
    struct LinkSignal {
        QString name;
        double signal;
    };

    const QVector<LinkSignal> links = {
        { QStringLiteral("A2G"), m_signalA2G },
        { QStringLiteral("A2A"), m_signalA2A },
        { QStringLiteral("SATCOM"), m_signalSatcom }
    };

    LinkSignal best = links.first();
    LinkSignal second = links[1];

    for (const LinkSignal& link : links) {
        if (link.signal > best.signal) {
            second = best;
            best = link;
        } else if (link.name != best.name && link.signal > second.signal) {
            second = link;
        }
    }

    const bool degraded = best.signal < 45.0 || m_packetLossPct > 18.0 || m_rttMs > 300.0;
    if (degraded && best.name != m_activeLink) {
        m_lastFailoverReason = QStringLiteral("Automatic failover to %1 due to degraded route.").arg(best.name);
        m_standbyLink = m_activeLink;
        m_activeLink = best.name;
        pushEvent(m_lastFailoverReason);
    }

    if (second.name != m_activeLink) {
        m_standbyLink = second.name;
    }
}

void OffboardCommsManager::recalcMetrics()
{
    const double activeSignal = (m_activeLink == QStringLiteral("A2G"))
            ? m_signalA2G
            : ((m_activeLink == QStringLiteral("A2A")) ? m_signalA2A : m_signalSatcom);

    const double qualityFactor = qBound(0.15, activeSignal / 100.0, 1.0);
    m_rttMs = qBound(18.0, (110.0 / qualityFactor) + m_jitterMs, 900.0);
    m_jitterMs = qBound(2.0, (25.0 * (1.0 - qualityFactor)) + (m_packetLossPct * 0.5), 120.0);
    m_throughputKbps = qBound(18.0, 480.0 * qualityFactor, 900.0);
}

bool OffboardCommsManager::validateEnvelope(const QVariantMap& envelope)
{
    if (!envelope.contains(QStringLiteral("schemaVersion"))
        || !envelope.contains(QStringLiteral("deviceId"))
        || !envelope.contains(QStringLiteral("timestampUtc"))
        || !envelope.contains(QStringLiteral("type"))) {
        return false;
    }

    const QString schema = envelope.value(QStringLiteral("schemaVersion")).toString().trimmed();
    if (schema.isEmpty()) {
        return false;
    }

    return true;
}
