/*
 * Defines ARINC 661 parameter identifiers shared by the QML cockpit UI and C++ backend.
 */

// ARINC 661 Parameter ID constants for QML
// Mirrors Arinc661Types.h — keep in sync

var Arinc661 = {
    // Flight / Attitude
    P_ALTITUDE:          0x0101,
    P_AIRSPEED:          0x0102,
    P_HEADING:           0x0103,
    P_PITCH:             0x0104,
    P_ROLL:              0x0105,
    P_VERTICAL_SPEED:    0x0106,
    P_OAT:               0x0107,
    P_WIND_SPEED:        0x0108,
    P_WIND_DIRECTION:    0x0109,
    P_LATITUDE:          0x010A,
    P_LONGITUDE:         0x010B,
    P_TRUE_ALTITUDE:     0x010C,

    // Navigation
    P_ACTIVE_WAYPOINT:    0x0201,
    P_WAYPOINT_REMAINING: 0x0202,
    P_DISTANCE_TO_GO:     0x0203,
    P_ETA_MINUTES:        0x0204,
    P_NEAREST_AIRPORT:    0x0205,
    P_NEAREST_AIRPORT_DIST:0x0206,
    P_NEAREST_NAVAID:     0x0207,
    P_NEAREST_NAVAID_DIST:0x0208,
    P_NEAREST_NAVAID_FREQ:0x0209,
    P_NEAREST_NAVAID_TYPE:0x020A,
    P_TUNED_NAVAID_IDENT: 0x020B,
    P_TUNED_NAVAID_FREQ:  0x020C,
    P_TUNED_NAVAID_BRG:   0x020D,
    P_TUNED_NAVAID_DIST:  0x020E,

    // Engine / Fuel / Oil
    P_ENGINE_RPM:         0x0301,
    P_ENGINE_TEMP:        0x0302,
    P_FUEL:               0x0303,
    P_OIL_PRESSURE:       0x0304,
    P_OIL_TEMP:           0x0305,
    P_EGT:                0x0306,
    P_FLAPS_POSITION:     0x0307,
    P_GEAR_UP:            0x0308,
    P_GEAR_POSITION:      0x0309,

    // Electrical
    P_VOLTAGE:            0x0401,
    P_AMPERAGE:           0x0402,

    // Radio
    P_COM1_ACTIVE:        0x0501,
    P_COM1_STANDBY:       0x0502,
    P_NAV1_ACTIVE:        0x0503,
    P_NAV1_STANDBY:       0x0504,

    // CAS
    P_CAS_WARNING_COUNT:  0x0601,
    P_CAS_CAUTION_COUNT:  0x0602,

    // Autopilot
    P_AP_ENGAGED:         0x0701,
    P_AP_HEADING_SEL:     0x0702,
    P_AP_ALTITUDE_SEL:    0x0703,
    P_AP_VS_SEL:          0x0704,

    // Telemetry History
    P_HIST_ALTITUDE:      0x0801,
    P_HIST_AIRSPEED:      0x0802,
    P_HIST_VERTICAL_SPD:  0x0803,

    // State
    P_FLIGHT_PHASE:       0x0901,
    P_FLIGHT_PHASE_ENUM:  0x0902,
    P_ELAPSED_IN_PHASE:   0x0903,

    // CDS Commands
    CDS_CMD_SHOW_LAYER:      0xF001,
    CDS_CMD_HIDE_LAYER:      0xF002,
    CDS_CMD_SHOW_WIDGET:     0xF003,
    CDS_CMD_HIDE_WIDGET:     0xF004,
    CDS_CMD_ENABLE_WIDGET:   0xF005,
    CDS_CMD_DISABLE_WIDGET:  0xF006,
    CDS_CMD_MOVE_WIDGET:     0xF007,
    CDS_CMD_RESIZE_WIDGET:   0xF008,
    CDS_CMD_SET_ACTIVE_LAYER:0xF009,

    // Input Events
    INPUT_BUTTON_PRESS:    0xF101,
    INPUT_KNOB_TURN:       0xF102,
    INPUT_SWITCH_TOGGLE:   0xF103,
    INPUT_TOUCH_TAP:       0xF104,
    INPUT_TOUCH_DRAG:      0xF105
};

// Make available to all QML files that import this
var P = Arinc661;
