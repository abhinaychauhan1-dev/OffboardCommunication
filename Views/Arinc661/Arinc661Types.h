/**
 * @file Arinc661Types.h
 * @brief Defines ARINC 661 parameter identifiers and shared types for cockpit widgets.
 *
 * Developed by Abhinay Chauhan
 */

#ifndef ARINC661TYPES_H
#define ARINC661TYPES_H

/**
 * @file Arinc661Types.h
 * @brief ARINC 661 parameter identifiers for the avionics data bus.
 *
 * Follows the ARINC 661 standard where every display parameter
 * is assigned a unique integer identifier.  Parameters are grouped
 * logically by ID range.
 *
 * Ranges:
 *   0x0100 â€“ 0x01FF   Flight / Attitude
 *   0x0200 â€“ 0x02FF   Navigation
 *   0x0300 â€“ 0x03FF   Engine / Fuel
 *   0x0400 â€“ 0x04FF   Electrical / Systems
 *   0x0500 â€“ 0x05FF   Radio / Comms
 *   0x0600 â€“ 0x06FF   Alerts / CAS
 *   0x0700 â€“ 0x07FF   Autopilot
 *   0x0800 â€“ 0x08FF   Telemetry / History
 *   0x0900 â€“ 0x09FF   State / Meta
 *   0xFF00            System reconfiguration (reserved)
 */

#include <cstdint>

namespace Arinc661 {

// ---- Type aliases (ARINC 661 uses 32-bit IDs) ----
using ParameterId = int32_t;

// ---- Flight / Attitude (0x0100 â€“ 0x01FF) ----
constexpr ParameterId P_ALTITUDE           = 0x0101;
constexpr ParameterId P_AIRSPEED           = 0x0102;
constexpr ParameterId P_HEADING            = 0x0103;
constexpr ParameterId P_PITCH              = 0x0104;
constexpr ParameterId P_ROLL               = 0x0105;
constexpr ParameterId P_VERTICAL_SPEED     = 0x0106;
constexpr ParameterId P_OAT                = 0x0107;
constexpr ParameterId P_WIND_SPEED         = 0x0108;
constexpr ParameterId P_WIND_DIRECTION     = 0x0109;
constexpr ParameterId P_LATITUDE           = 0x010A;
constexpr ParameterId P_LONGITUDE          = 0x010B;
constexpr ParameterId P_TRUE_ALTITUDE      = 0x010C;

// ---- Navigation (0x0200 â€“ 0x02FF) ----
constexpr ParameterId P_ACTIVE_WAYPOINT    = 0x0201;
constexpr ParameterId P_WAYPOINT_REMAINING = 0x0202;
constexpr ParameterId P_DISTANCE_TO_GO     = 0x0203;
constexpr ParameterId P_ETA_MINUTES        = 0x0204;
constexpr ParameterId P_NEAREST_AIRPORT    = 0x0205;
constexpr ParameterId P_NEAREST_AIRPORT_DIST = 0x0206;
constexpr ParameterId P_NEAREST_NAVAID     = 0x0207;
constexpr ParameterId P_NEAREST_NAVAID_DIST= 0x0208;
constexpr ParameterId P_NEAREST_NAVAID_FREQ= 0x0209;
constexpr ParameterId P_NEAREST_NAVAID_TYPE= 0x020A;
constexpr ParameterId P_TUNED_NAVAID_IDENT = 0x020B;
constexpr ParameterId P_TUNED_NAVAID_FREQ  = 0x020C;
constexpr ParameterId P_TUNED_NAVAID_BRG   = 0x020D;
constexpr ParameterId P_TUNED_NAVAID_DIST  = 0x020E;

// ---- Engine / Fuel / Oil (0x0300 â€“ 0x03FF) ----
constexpr ParameterId P_ENGINE_RPM         = 0x0301;
constexpr ParameterId P_ENGINE_TEMP        = 0x0302;
constexpr ParameterId P_FUEL               = 0x0303;
constexpr ParameterId P_OIL_PRESSURE       = 0x0304;
constexpr ParameterId P_OIL_TEMP           = 0x0305;
constexpr ParameterId P_EGT                = 0x0306;
constexpr ParameterId P_FLAPS_POSITION     = 0x0307;
constexpr ParameterId P_GEAR_UP            = 0x0308;
constexpr ParameterId P_GEAR_POSITION      = 0x0309;

// ---- Electrical / Systems (0x0400 â€“ 0x04FF) ----
constexpr ParameterId P_VOLTAGE            = 0x0401;
constexpr ParameterId P_AMPERAGE           = 0x0402;

// ---- Radio / Communications (0x0500 â€“ 0x05FF) ----
constexpr ParameterId P_COM1_ACTIVE        = 0x0501;
constexpr ParameterId P_COM1_STANDBY       = 0x0502;
constexpr ParameterId P_NAV1_ACTIVE        = 0x0503;
constexpr ParameterId P_NAV1_STANDBY       = 0x0504;

// ---- Alerts / CAS (0x0600 â€“ 0x06FF) ----
constexpr ParameterId P_CAS_WARNING_COUNT  = 0x0601;
constexpr ParameterId P_CAS_CAUTION_COUNT  = 0x0602;

// ---- Autopilot (0x0700 â€“ 0x07FF) ----
constexpr ParameterId P_AP_ENGAGED         = 0x0701;
constexpr ParameterId P_AP_HEADING_SEL     = 0x0702;
constexpr ParameterId P_AP_ALTITUDE_SEL    = 0x0703;
constexpr ParameterId P_AP_VS_SEL          = 0x0704;

// ---- Telemetry / History (0x0800 â€“ 0x08FF) ----
constexpr ParameterId P_HIST_ALTITUDE      = 0x0801;
constexpr ParameterId P_HIST_AIRSPEED      = 0x0802;
constexpr ParameterId P_HIST_VERTICAL_SPD  = 0x0803;

// ---- State / Meta (0x0900 â€“ 0x09FF) ----
constexpr ParameterId P_FLIGHT_PHASE       = 0x0901;
constexpr ParameterId P_FLIGHT_PHASE_ENUM  = 0x0902;
constexpr ParameterId P_ELAPSED_IN_PHASE   = 0x0903;

// ---- CDS Commands (0xF000 â€“ 0xF0FF) ----
constexpr ParameterId CDS_CMD_SHOW_LAYER    = 0xF001;
constexpr ParameterId CDS_CMD_HIDE_LAYER    = 0xF002;
constexpr ParameterId CDS_CMD_SHOW_WIDGET   = 0xF003;
constexpr ParameterId CDS_CMD_HIDE_WIDGET   = 0xF004;
constexpr ParameterId CDS_CMD_ENABLE_WIDGET = 0xF005;
constexpr ParameterId CDS_CMD_DISABLE_WIDGET= 0xF006;
constexpr ParameterId CDS_CMD_MOVE_WIDGET   = 0xF007;
constexpr ParameterId CDS_CMD_RESIZE_WIDGET = 0xF008;
constexpr ParameterId CDS_CMD_SET_ACTIVE_LAYER = 0xF009;

// ---- Input Events (0xF100 â€“ 0xF1FF) ----
constexpr ParameterId INPUT_BUTTON_PRESS    = 0xF101;
constexpr ParameterId INPUT_KNOB_TURN       = 0xF102;
constexpr ParameterId INPUT_SWITCH_TOGGLE   = 0xF103;
constexpr ParameterId INPUT_TOUCH_TAP       = 0xF104;
constexpr ParameterId INPUT_TOUCH_DRAG      = 0xF105;

// ---- Widget Instance IDs (UWI range) ----
constexpr int UWI_AIRSPEED_TAPE      = 1001;
constexpr int UWI_ALTITUDE_TAPE      = 1002;
constexpr int UWI_HEADING_STRIP      = 1003;
constexpr int UWI_ATTITUDE_INDICATOR = 1004;
constexpr int UWI_VSI_BAR            = 1005;
constexpr int UWI_ENGINE_RPM_BAR     = 2001;
constexpr int UWI_FUEL_BAR           = 2002;
constexpr int UWI_EGT_BAR            = 2003;
constexpr int UWI_OIL_PRESSURE       = 2004;
constexpr int UWI_OIL_TEMP           = 2005;
constexpr int UWI_FLAPS_READOUT      = 2006;
constexpr int UWI_GEAR_STATUS        = 2007;
constexpr int UWI_VOLTAGE_BAR        = 3001;
constexpr int UWI_AMPERAGE_BAR       = 3002;
constexpr int UWI_AP_STATUS          = 3003;
constexpr int UWI_CAS_WARNINGS       = 3004;
constexpr int UWI_NAV_MAP            = 4001;
constexpr int UWI_RADIO_PANEL        = 4002;
constexpr int UWI_MULTI_FN_DISPLAY   = 4003;

} // namespace Arinc661

#endif // ARINC661TYPES_H
