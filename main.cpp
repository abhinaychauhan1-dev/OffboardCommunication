/**
 * @file main.cpp
 * @brief Entry point for the OffboardCommunication Qt Quick application.
 */

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <QtMath>
#include <cmath>

#include "Core/MqttTelemetryClient.h"
#include "Core/OffboardCommsManager.h"
#include "Core/SoftwareUpdateManager.h"
#include "Views/Arinc661/ParameterBus.h"

namespace {
void seedInitialValues(ParameterBus& bus)
{
    bus.setValue(Arinc661::P_ALTITUDE, 4500.0);
    bus.setValue(Arinc661::P_AIRSPEED, 132.0);
    bus.setValue(Arinc661::P_LATITUDE, 28.6139);
    bus.setValue(Arinc661::P_LONGITUDE, 77.2090);
    bus.setValue(Arinc661::P_FLIGHT_PHASE, QStringLiteral("CRUISE"));
    bus.setValue(Arinc661::P_FUEL, 62.5);
    bus.setValue(Arinc661::P_OAT, 14.0);
    bus.setValue(Arinc661::P_WIND_SPEED, 11.0);
    bus.setValue(Arinc661::P_WIND_DIRECTION, 265.0);
    bus.setValue(Arinc661::P_ROLL, 2.0);
    bus.setValue(Arinc661::P_NEAREST_AIRPORT_DIST, 34.0);
    bus.setValue(Arinc661::P_COM1_ACTIVE, 118.000);
    bus.setValue(Arinc661::P_COM1_STANDBY, 121.900);
    bus.setValue(Arinc661::P_NAV1_ACTIVE, 112.300);
    bus.setValue(Arinc661::P_NAV1_STANDBY, 115.700);
    bus.setValue(Arinc661::P_TUNED_NAVAID_IDENT, QStringLiteral("DLI"));
    bus.setValue(Arinc661::P_TUNED_NAVAID_FREQ, 112.3);
    bus.setValue(Arinc661::P_TUNED_NAVAID_BRG, 146.0);
    bus.setValue(Arinc661::P_TUNED_NAVAID_DIST, 24.6);
}

void updateTelemetry(ParameterBus& bus, int tick)
{
    const double t = tick / 10.0;
    const double altitude = 4500.0 + 250.0 * qSin(t * 0.33);
    const double airspeed = 132.0 + 9.0 * qSin(t * 0.51);
    const double latitude = 28.6139 + 0.04 * qSin(t * 0.08);
    const double longitude = 77.2090 + 0.04 * qCos(t * 0.08);
    const double windSpeed = 11.0 + 4.0 * qAbs(qSin(t * 0.42));
    const double roll = 8.0 * qSin(t * 0.48);
    const double airportDist = 34.0 + 9.0 * qAbs(qSin(t * 0.12));
    const double brg = std::fmod(146.0 + t * 3.2, 360.0);

    const QString phase = (altitude > 4700.0)
            ? QStringLiteral("CLIMB")
            : ((altitude < 4350.0) ? QStringLiteral("DESCENT") : QStringLiteral("CRUISE"));

    bus.setValue(Arinc661::P_ALTITUDE, altitude);
    bus.setValue(Arinc661::P_AIRSPEED, airspeed);
    bus.setValue(Arinc661::P_LATITUDE, latitude);
    bus.setValue(Arinc661::P_LONGITUDE, longitude);
    bus.setValue(Arinc661::P_FLIGHT_PHASE, phase);
    bus.setValue(Arinc661::P_FUEL, qMax(0.0, 62.5 - tick * 0.02));
    bus.setValue(Arinc661::P_OAT, 14.0 - 0.003 * (altitude - 4500.0));
    bus.setValue(Arinc661::P_WIND_SPEED, windSpeed);
    bus.setValue(Arinc661::P_WIND_DIRECTION, std::fmod(265.0 + tick * 0.7, 360.0));
    bus.setValue(Arinc661::P_ROLL, roll);
    bus.setValue(Arinc661::P_NEAREST_AIRPORT_DIST, airportDist);
    bus.setValue(Arinc661::P_TUNED_NAVAID_BRG, brg);
    bus.setValue(Arinc661::P_TUNED_NAVAID_DIST, 24.6 + 2.0 * qAbs(qSin(t * 0.2)));
}

double deriveA2gSignal(const ParameterBus& bus)
{
    const double dist = bus.doubleValue(Arinc661::P_NEAREST_AIRPORT_DIST, 0.0);
    const double wind = bus.doubleValue(Arinc661::P_WIND_SPEED, 0.0);
    return qBound(20.0, 95.0 - dist * 0.5 - wind * 0.35, 100.0);
}

double deriveA2aSignal(const ParameterBus& bus)
{
    const double speed = bus.doubleValue(Arinc661::P_AIRSPEED, 0.0);
    const double roll = qAbs(bus.doubleValue(Arinc661::P_ROLL, 0.0));
    return qBound(25.0, 78.0 + speed * 0.03 - roll * 0.45, 100.0);
}

double deriveSatcomSignal(const ParameterBus& bus)
{
    const double alt = bus.doubleValue(Arinc661::P_ALTITUDE, 0.0);
    return qBound(35.0, 70.0 + alt / 1800.0, 100.0);
}

QVariantMap captureTelemetrySnapshot(const ParameterBus& bus)
{
    QVariantMap payload;
    payload.insert(QStringLiteral("altitudeFt"), bus.doubleValue(Arinc661::P_ALTITUDE, 0.0));
    payload.insert(QStringLiteral("airspeedKt"), bus.doubleValue(Arinc661::P_AIRSPEED, 0.0));
    payload.insert(QStringLiteral("latitude"), bus.doubleValue(Arinc661::P_LATITUDE, 0.0));
    payload.insert(QStringLiteral("longitude"), bus.doubleValue(Arinc661::P_LONGITUDE, 0.0));
    payload.insert(QStringLiteral("phase"), bus.stringValue(Arinc661::P_FLIGHT_PHASE, QStringLiteral("UNKNOWN")));
    payload.insert(QStringLiteral("fuelGal"), bus.doubleValue(Arinc661::P_FUEL, 0.0));
    payload.insert(QStringLiteral("windKt"), bus.doubleValue(Arinc661::P_WIND_SPEED, 0.0));
    return payload;
}
}

int main(int argc, char* argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationVersion(QStringLiteral(OFFBOARD_APP_VERSION));

    ParameterBus arinc661Bus;
    SoftwareUpdateManager updateManager;
    MqttTelemetryClient mqttClient;
    OffboardCommsManager commsManager;

    commsManager.setMqttClient(&mqttClient);

    seedInitialValues(arinc661Bus);

    int tick = 0;
    QTimer telemetryTimer;
    telemetryTimer.setInterval(400);
    QObject::connect(&telemetryTimer, &QTimer::timeout, &app, [&arinc661Bus, &commsManager, &tick]() {
        ++tick;
        updateTelemetry(arinc661Bus, tick);

        const QVariantMap snapshot = captureTelemetrySnapshot(arinc661Bus);
        commsManager.tickTelemetry(snapshot);
        commsManager.updateLinkSignals(deriveA2gSignal(arinc661Bus),
                                       deriveA2aSignal(arinc661Bus),
                                       deriveSatcomSignal(arinc661Bus));
    });
    telemetryTimer.start();

    QQmlApplicationEngine engine;
    QQmlContext* rootContext = engine.rootContext();
    rootContext->setContextProperty(QStringLiteral("arinc661Bus"), &arinc661Bus);
    rootContext->setContextProperty(QStringLiteral("updateManager"), &updateManager);
    rootContext->setContextProperty(QStringLiteral("mqttClient"), &mqttClient);
    rootContext->setContextProperty(QStringLiteral("commsManager"), &commsManager);

    const QUrl url(QStringLiteral("qrc:/Views/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject* obj, const QUrl& objUrl) {
        if (!obj && url == objUrl) {
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
