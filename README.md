# OffboardCommunication

Standalone Qt Quick project extracted from the Offboard Communication feature in AvionicsSimulator.

## Included Components

- MQTT gateway backend from Core/MqttTelemetryClient.*
- Software update manager from Core/SoftwareUpdateManager.*
- Reliability, failover, queueing, metrics, and replay manager in Core/OffboardCommsManager.*
- ARINC 661 communication layer UI from Views/Arinc661/Layers/CommsLayer.qml
- Minimal ARINC parameter bus (ParameterBus) and IDs (Arinc661Types)
- Built-in telemetry generator in main.cpp so the UI remains active without simulator engine dependencies

## Capabilities Added

- Command uplink with simulated ACK/NACK, correlation IDs, retries, and retry budget
- Store-and-forward queue with priority insertion and flush controls
- Dynamic link selection/failover across A2G, A2A, and SATCOM
- Link metrics (RTT, jitter, packet loss, throughput)
- Event timeline and traffic inspector streams
- Operational modes (NORMAL, DEGRADED, EMERGENCY)
- Security toggles (TLS, cert pinning, payload signing)
- Fault injection profiles (clear, high loss, outage, latency spike, TLS failure)
- Replay recording, save/load, and playback
- Basic envelope schema guard with validation error counter

## Build

```powershell
cmake -S . -B build
cmake --build build --config Release
```

## Run

Executable path after build (Visual Studio generator):

- build/Release/OffboardCommunication.exe

Notes:

- MQTT support is optional. If Qt6::Mqtt is found, the app uses it; otherwise the UI still runs and reports MQTT as unavailable.
- Update feed URL can be overridden with OFFBOARD_UPDATE_FEED environment variable.
