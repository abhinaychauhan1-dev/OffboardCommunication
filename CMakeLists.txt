cmake_minimum_required(VERSION 3.16)

project(OffboardCommunication VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml Network)
find_package(Qt6 QUIET COMPONENTS Mqtt)

set(OFFBOARD_HAS_QT_MQTT 0)
if(TARGET Qt6::Mqtt)
    set(OFFBOARD_HAS_QT_MQTT 1)
endif()

qt_add_executable(OffboardCommunication
    main.cpp
    Core/MqttTelemetryClient.h
    Core/MqttTelemetryClient.cpp
    Core/OffboardCommsManager.h
    Core/OffboardCommsManager.cpp
    Core/SoftwareUpdateManager.h
    Core/SoftwareUpdateManager.cpp
    Views/Arinc661/Arinc661Types.h
    Views/Arinc661/ParameterBus.h
    Views/Arinc661/ParameterBus.cpp
)

qt_add_resources(OffboardCommunication offboard_qml
    PREFIX "/"
    FILES
        Views/main.qml
        Views/Arinc661/Layers/CommsLayer.qml
        Views/Arinc661/Widgets/DigitalReadout.qml
        Views/Arinc661/ParameterBus.js
)

target_compile_definitions(OffboardCommunication PRIVATE
    OFFBOARD_HAS_QT_MQTT=${OFFBOARD_HAS_QT_MQTT}
    OFFBOARD_APP_VERSION="${PROJECT_VERSION}"
)

target_include_directories(OffboardCommunication PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Core
    ${CMAKE_CURRENT_SOURCE_DIR}/Views/Arinc661
)

target_link_libraries(OffboardCommunication PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Network
)

if(OFFBOARD_HAS_QT_MQTT)
    target_link_libraries(OffboardCommunication PRIVATE Qt6::Mqtt)
endif()

if(WIN32)
    set_target_properties(OffboardCommunication PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS OffboardCommunication
    RUNTIME DESTINATION bin
)
