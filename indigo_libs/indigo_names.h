//
//  indigo_names.h
//  indigo
//
//  Created by Peter Polakovic on 22/11/2016.
//  Copyright © 2016 CloudMakers, s. r. o. All rights reserved.
//

#ifndef indigo_names_h
#define indigo_names_h

//----------------------------------------------------------------------
/** CONNECTION property name.
 */
#define CONNECTION_PROPERTY_NAME							"CONNECTION"

/** CONNECTION.CONNECTED property item name.
 */
#define CONNECTION_CONNECTED_ITEM_NAME				"CONNECTED"

/** CONNECTION.DISCONNECTED property item name.
 */
#define CONNECTION_DISCONNECTED_ITEM_NAME			"DISCONNECTED"

//----------------------------------------------------------------------
/** INFO property name.
 */
#define INFO_PROPERTY_NAME										"INFO"

/** INFO.DEVICE_NAME property item name.
 */
#define INFO_DEVICE_NAME_ITEM_NAME						"DEVICE_NAME"

/** INFO.DEVICE_VERSION property item name.
 */
#define INFO_DEVICE_VERSION_ITEM_NAME					"DEVICE_VERSION"

/** INFO.DEVICE_INTERFACE property item name.
 */
#define INFO_DEVICE_INTERFACE_ITEM_NAME				"DEVICE_INTERFACE"

/** INFO.FRAMEWORK_NAME property item name.
 */
#define INFO_FRAMEWORK_NAME_ITEM_NAME					"FRAMEWORK_NAME"

/** INFO.FRAMEWORK_VERSION property item name.
 */
#define INFO_FRAMEWORK_VERSION_ITEM_NAME			"FRAMEWORK_VERSION"

//----------------------------------------------------------------------
/** DEBUG property name.
 */
#define DEBUG_PROPERTY_NAME										"DEBUG"

/** DEBUG.ENABLED property item name.
 */
#define DEBUG_ENABLED_ITEM_NAME								"ENABLED"

/** DEBUG.DISABLED property item name.
 */
#define DEBUG_DISABLED_ITEM_NAME							"DISABLED"

//----------------------------------------------------------------------
/** SIMULATION property name.
 */
#define SIMULATION_PROPERTY_NAME							"SIMULATION"

/** SIMULATION.DISABLED property name.
 */
#define SIMULATION_ENABLED_ITEM_NAME					"ENABLED"

/** SIMULATION.DISABLED property item name.
 */
#define SIMULATION_DISABLED_ITEM_NAME					"DISABLED"

//----------------------------------------------------------------------
/** CONFIG property name.
 */
#define CONFIG_PROPERTY_NAME									"CONFIG"

/** CONFIG.LOAD property item name.
 */
#define CONFIG_LOAD_ITEM_NAME									"LOAD"

/** CONFIG.SAVE property item name.
 */
#define CONFIG_SAVE_ITEM_NAME									"SAVE"

/** CONFIG.DEFAULT property item name.
 */
#define CONFIG_DEFAULT_ITEM_NAME							"DEFAULT"

//----------------------------------------------------------------------
/** DEVICE_PORT property name.
 */
#define DEVICE_PORT_PROPERTY_NAME							"DEVICE_PORT"

/** DEVICE_PORT.DEVICE_PORT property name.
 */
#define DEVICE_PORT_ITEM_NAME									"DEVICE_PORT"

//----------------------------------------------------------------------
/** DEVICE_PORTS property name.
 */
#define DEVICE_PORTS_PROPERTY_NAME						"DEVICE_PORTS"

//----------------------------------------------------------------------
/** CCD_INFO property name.
 */
#define CCD_INFO_PROPERTY_NAME                "CCD_INFO"

/** CCD_INFO.WIDTH property item name.
 */
#define CCD_INFO_WIDTH_ITEM_NAME              "WIDTH"

/** CCD_INFO.HEIGHT property item name.
 */
#define CCD_INFO_HEIGHT_ITEM_NAME             "HEIGHT"

/** CCD_INFO.MAX_HORIZONAL_BIN property item name.
 */
#define CCD_INFO_MAX_HORIZONAL_BIN_ITEM_NAME  "MAX_HORIZONAL_BIN"

/** CCD_INFO.MAX_VERTICAL_BIN property item name.
 */
#define CCD_INFO_MAX_VERTICAL_BIN_ITEM_NAME   "MAX_VERTICAL_BIN"

/** CCD_INFO.PIXEL_SIZE property item name.
 */
#define CCD_INFO_PIXEL_SIZE_ITEM_NAME         "PIXEL_SIZE"

/** CCD_INFO.PIXEL_WIDTH property item name.
 */
#define CCD_INFO_PIXEL_WIDTH_ITEM_NAME        "PIXEL_WIDTH"

/** CCD_INFO.PIXEL_HEIGHT property item name.
 */
#define CCD_INFO_PIXEL_HEIGHT_ITEM_NAME       "PIXEL_HEIGHT"

/** CCD_INFO.BITS_PER_PIXEL property item name.
 */
#define CCD_INFO_BITS_PER_PIXEL_ITEM_NAME     "BITS_PER_PIXEL"

/** CCD_UPLOAD_MODE property name.
 */
#define CCD_UPLOAD_MODE_PROPERTY_NAME         "CCD_UPLOAD_MODE"

/** CCD_UPLOAD_MODE.CLIENT property item name.
 */
#define CCD_UPLOAD_MODE_CLIENT_ITEM_NAME      "CLIENT"

/** CCD_UPLOAD_MODE.LOCAL property item name.
 */
#define CCD_UPLOAD_MODE_LOCAL_ITEM_NAME       "LOCAL"

/** CCD_UPLOAD_MODE.BOTH property item name.
 */
#define CCD_UPLOAD_MODE_BOTH_ITEM_NAME        "BOTH"

//----------------------------------------------------------------------
/** CCD_LOCAL_MODE property name.
 */
#define CCD_LOCAL_MODE_PROPERTY_NAME          "CCD_LOCAL_MODE"

/** CCD_LOCAL_MODE.DIR property item name.
 */
#define CCD_LOCAL_MODE_DIR_ITEM_NAME          "DIR"

/** CCD_LOCAL_MODE.PREFIX property item name.
 */
#define CCD_LOCAL_MODE_PREFIX_ITEM_NAME       "PREFIX"

//----------------------------------------------------------------------
/** CCD_EXPOSURE property name.
 */
#define CCD_EXPOSURE_PROPERTY_NAME            "CCD_EXPOSURE"

/** CCD_EXPOSURE.EXPOSURE property item name.
 */
#define CCD_EXPOSURE_ITEM_NAME                "EXPOSURE"

//----------------------------------------------------------------------
/** CCD_ABORT_EXPOSURE property name.
 */
#define CCD_ABORT_EXPOSURE_PROPERTY_NAME      "CCD_ABORT_EXPOSURE"

/** CCD_ABORT_EXPOSURE.ABORT_EXPOSURE property item name.
 */
#define CCD_ABORT_EXPOSURE_ITEM_NAME          "ABORT_EXPOSURE"

//----------------------------------------------------------------------
/** CCD_FRAME property name.
 */
#define CCD_FRAME_PROPERTY_NAME               "CCD_FRAME"

/** CCD_FRAME.LEFT property item name.
 */
#define CCD_FRAME_LEFT_ITEM_NAME              "LEFT"

/** CCD_FRAME.TOP property item name.
 */
#define CCD_FRAME_TOP_ITEM_NAME               "TOP"

/** CCD_FRAME.WIDTH property item name.
 */
#define CCD_FRAME_WIDTH_ITEM_NAME             "WIDTH"

/** CCD_FRAME.HEIGHT property item name.
 */
#define CCD_FRAME_HEIGHT_ITEM_NAME            "HEIGHT"

/** CCD_FRAME.BITS_PER_PIXEL property item name.
 */
#define CCD_FRAME_BITS_PER_PIXEL_ITEM_NAME    "BITS_PER_PIXEL"

//----------------------------------------------------------------------
/** CCD_BIN property name.
 */
#define CCD_BIN_PROPERTY_NAME                 "CCD_BIN"

/** CCD_BIN.HORIZONTAL property item name.
 */
#define CCD_BIN_HORIZONTAL_ITEM_NAME          "HORIZONTAL"

/** CCD_BIN.VERTICAL property item name.
 */
#define CCD_BIN_VERTICAL_ITEM_NAME            "VERTICAL"

//----------------------------------------------------------------------
/** CCD_MODE property name.
 */
#define CCD_MODE_PROPERTY_NAME								"CCD_MODE"

//----------------------------------------------------------------------
/** CCD_GAIN property name.
 */
#define CCD_GAIN_PROPERTY_NAME                "CCD_GAIN"

/** CCD_GAIN.GAIN property item name.
 */
#define CCD_GAIN_ITEM_NAME                    "GAIN"

//----------------------------------------------------------------------
/** CCD_OFFSET property name.
 */
#define CCD_OFFSET_PROPERTY_NAME              "CCD_OFFSET"

/** CCD_OFFSET.OFFSET property item name.
 */
#define CCD_OFFSET_ITEM_NAME									"OFFSET"

//----------------------------------------------------------------------
/** CCD_GAMMA property name.
 */
#define CCD_GAMMA_PROPERTY_NAME               "CCD_GAMMA"

/** CCD_GAMMA.GAMMA property item name.
 */
#define CCD_GAMMA_ITEM_NAME                   "GAMMA"

//----------------------------------------------------------------------
/** CCD_FRAME_TYPE property name.
 */
#define CCD_FRAME_TYPE_PROPERTY_NAME          "CCD_FRAME_TYPE"

/** CCD_FRAME_TYPE.LIGHT property item name
 */
#define CCD_FRAME_TYPE_LIGHT_ITEM_NAME        "LIGHT"

/** CCD_FRAME_TYPE.BIAS property item name.
 */
#define CCD_FRAME_TYPE_BIAS_ITEM_NAME         "BIAS"

/** CCD_FRAME_TYPE.DARK property item name.
 */
#define CCD_FRAME_TYPE_DARK_ITEM_NAME         "DARK"

/** CCD_FRAME_TYPE.FLAT property item name.
 */
#define CCD_FRAME_TYPE_FLAT_ITEM_NAME         "FLAT"

//----------------------------------------------------------------------
/** CCD_IMAGE_FORMAT property name.
 */
#define CCD_IMAGE_FORMAT_PROPERTY_NAME        "CCD_IMAGE_FORMAT"

/** CCD_IMAGE_FORMAT.RAW property item name.
 */
#define CCD_IMAGE_FORMAT_RAW_ITEM_NAME        "RAW"

/** CCD_IMAGE_FORMAT.FITS property item name.
 */
#define CCD_IMAGE_FORMAT_FITS_ITEM_NAME       "FITS"

/** CCD_IMAGE_FORMAT.JPEG property item name.
 */
#define CCD_IMAGE_FORMAT_JPEG_ITEM_NAME       "JPEG"

//----------------------------------------------------------------------
/** CCD_IMAGE_FILE property name.
 */
#define CCD_IMAGE_FILE_PROPERTY_NAME          "CCD_IMAGE_FILE"

/** CCD_IMAGE_FILE.FILE property item name.
 */
#define CCD_IMAGE_FILE_ITEM_NAME              "FILE "

/** CCD_IMAGE property name.
 */
#define CCD_IMAGE_PROPERTY_NAME               "CCD_IMAGE"

/** CCD_IMAGE.IMAGE property item name.
 */
#define CCD_IMAGE_ITEM_NAME                   "IMAGE"

//----------------------------------------------------------------------
/** CCD_TEMPERATURE property name.
 */
#define CCD_TEMPERATURE_PROPERTY_NAME         "CCD_TEMPERATURE"

/** CCD_TEMPERATURE.TEMPERATURE property item name.
 */
#define CCD_TEMPERATURE_ITEM_NAME             "TEMPERATURE"

//----------------------------------------------------------------------
/** CCD_COOLER property name.
 */
#define CCD_COOLER_PROPERTY_NAME              "CCD_COOLER"

/** CCD_COOLER.ON property item name.
 */
#define CCD_COOLER_ON_ITEM_NAME               "ON"

/** CCD_COOLER.OFF property item name.
 */
#define CCD_COOLER_OFF_ITEM_NAME              "OFF"

//----------------------------------------------------------------------
/** CCD_COOLER_POWER property name.
 */
#define CCD_COOLER_POWER_PROPERTY_NAME        "CCD_COOLER_POWER"

/** CCD_COOLER_POWER.POWER property item name.
 */
#define CCD_COOLER_POWER_ITEM_NAME            "POWER"

//----------------------------------------------------------------------
/** GUIDER_GUIDE_DEC property name.
 */
#define GUIDER_GUIDE_DEC_PROPERTY_NAME        "GUIDER_GUIDE_DEC"

/** GUIDER_GUIDE.NORTH property item name.
 */
#define GUIDER_GUIDE_NORTH_ITEM_NAME          "NORTH"

/** GUIDER_GUIDE.SOUTH property item name.
 */
#define GUIDER_GUIDE_SOUTH_ITEM_NAME          "SOUTH"

//----------------------------------------------------------------------
/** GUIDER_GUIDE_RA property name.
 */
#define GUIDER_GUIDE_RA_PROPERTY_NAME         "GUIDER_GUIDE_RA"

/** GUIDER_GUIDE.EAST property item name.
 */
#define GUIDER_GUIDE_EAST_ITEM_NAME           "EAST"

/** GUIDER_GUIDE.WEST property item name.
 */
#define GUIDER_GUIDE_WEST_ITEM_NAME           "WEST"

//----------------------------------------------------------------------
/** WHEEL_SLOT property name.
 */
#define WHEEL_SLOT_PROPERTY_NAME							"WHEEL_SLOT"

/** WHEEL_SLOT.SLOT property item name.
 */
#define WHEEL_SLOT_ITEM_NAME									"SLOT"

//----------------------------------------------------------------------
/** WHEEL_SLOT_NAME property name.
 */
#define WHEEL_SLOT_NAME_PROPERTY_NAME					"WHEEL_SLOT_NAME"

/** WHEEL_SLOT_NAME.NAME_%d property item name.
 */
#define WHEEL_SLOT_NAME_ITEM_NAME							"SLOT_NAME_%d"

/** WHEEL_SLOT_NAME.NAME_1 property item name.
 */
#define WHEEL_SLOT_NAME_1_ITEM_NAME						"SLOT_NAME_1"

/** WHEEL_SLOT_NAME.NAME_2 property item name.
 */
#define WHEEL_SLOT_NAME_2_ITEM_NAME						"SLOT_NAME_2"

/** WHEEL_SLOT_NAME.NAME_3 property item name.
 */
#define WHEEL_SLOT_NAME_3_ITEM_NAME						"SLOT_NAME_3"

/** WHEEL_SLOT_NAME.NAME_4 property item name.
 */
#define WHEEL_SLOT_NAME_4_ITEM_NAME						"SLOT_NAME_4"

/** WHEEL_SLOT_NAME.NAME_5 property item name.
 */
#define WHEEL_SLOT_NAME_5_ITEM_NAME						"SLOT_NAME_5"

/** WHEEL_SLOT_NAME.NAME_6 property item name.
 */
#define WHEEL_SLOT_NAME_6_ITEM_NAME						"SLOT_NAME_6"

/** WHEEL_SLOT_NAME.NAME_7 property item name.
 */
#define WHEEL_SLOT_NAME_7_ITEM_NAME						"SLOT_NAME_7"

/** WHEEL_SLOT_NAME.NAME_8 property item name.
 */
#define WHEEL_SLOT_NAME_8_ITEM_NAME						"SLOT_NAME_8"

/** WHEEL_SLOT_NAME.NAME_9 property item name.
 */
#define WHEEL_SLOT_NAME_9_ITEM_NAME						"SLOT_NAME_9"

//----------------------------------------------------------------------
/** FOCUSER_SPEED property name.
 */
#define FOCUSER_SPEED_PROPERTY_NAME						"FOCUSER_SPEED"

/** FOCUSER_SPEED.SPEED property item name.
 */
#define FOCUSER_SPEED_ITEM_NAME								"SPEED"

//----------------------------------------------------------------------
/** FOCUSER_DIRECTION property name.
 */
#define FOCUSER_DIRECTION_PROPERTY_NAME				"FOCUSER_DIRECTION"

/** FOCUSER_DIRECTION.MOVE_INWARD property item name.
 */
#define FOCUSER_DIRECTION_MOVE_INWARD_ITEM_NAME		"MOVE_INWARD"

/** FOCUSER_DIRECTION.MOVE_OUTWARD property item name.
 */
#define FOCUSER_DIRECTION_MOVE_OUTWARD_ITEM_NAME	"MOVE_OUTWARD"

//----------------------------------------------------------------------
/** FOCUSER_STEPS property name.
 */
#define FOCUSER_STEPS_PROPERTY_NAME						"FOCUSER_STEPS"

/** FOCUSER_STEPS.STEPS property item name.
 */
#define FOCUSER_STEPS_ITEM_NAME								"STEPS"

//----------------------------------------------------------------------
/** FOCUSER_POSITION property name.
 */
#define FOCUSER_POSITION_PROPERTY_NAME				"FOCUSER_POSITION"

/** FOCUSER_POSITION.POSITION property item name.
 */
#define FOCUSER_POSITION_ITEM_NAME						"POSITION"

//----------------------------------------------------------------------
/** FOCUSER_ABORT_MOTION property name.
 */
#define FOCUSER_ABORT_MOTION_PROPERTY_NAME		"FOCUSER_ABORT_MOTION"

/** FOCUSER_ABORT_MOTION.ABORT_MOTION property item name.
 */
#define FOCUSER_ABORT_MOTION_ITEM_NAME				"ABORT_MOTION"

//----------------------------------------------------------------------
/** MOUNT_GEOGRAPHIC_COORDINATES property name.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY_NAME	"MOUNT_GEOGRAPHIC_COORDINATES"

/** MOUNT_GEOGRAPHIC_COORDINATES.LATITUDE property item name.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_LATITUDE_ITEM_NAME	"LATITUDE"

/** MOUNT_GEOGRAPHIC_COORDINATES.LONGITUDE property item name.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_LONGITUDE_ITEM_NAME	"LONGITUDE"

/** MOUNT_GEOGRAPHIC_COORDINATES.ELEVATION property item name.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_ELEVATION_ITEM_NAME	"ELEVATION"

//----------------------------------------------------------------------
/** MOUNT_LST_TIME property name.
 */
#define MOUNT_LST_TIME_PROPERTY_NAME						"MOUNT_LST_TIME"

/** MOUNT_TIME.TIME property item name.
 */
#define MOUNT_LST_TIME_ITEM_NAME								"TIME"

//----------------------------------------------------------------------
/** MOUNT_UTC_TIME property name.
 */
#define MOUNT_UTC_TIME_PROPERTY_NAME						"UTC_TIME"

/** MOUNT_UTC_TIME.UTC property item name.
 */
#define MOUNT_UTC_ITEM_NAME								"UTC"

/** MOUNT_UTC_TIME.OFFSET property item name.
 */
#define MOUNT_UTC_OFFSET_ITEM_NAME								"OFFSET"

//----------------------------------------------------------------------
/** MOUNT_PARK property name.
 */
#define MOUNT_PARK_PROPERTY_NAME								"MOUNT_PARK"

/** MOUNT_PARK.PARKED property item name.
 */
#define MOUNT_PARK_PARKED_ITEM_NAME							"PARKED"

/** MOUNT_PARK.UNPARKED property item name.
 */
#define MOUNT_PARK_UNPARKED_ITEM_NAME						"UNPARKED"

//----------------------------------------------------------------------
/** MOUNT_ON_COORDINATES_SET property name.
 */
#define MOUNT_ON_COORDINATES_SET_PROPERTY_NAME	"MOUNT_ON_COORDINATES_SET"

/** MOUNT_ON_COORDINATES_SET.TRACK property item name.
 */
#define MOUNT_ON_COORDINATES_SET_TRACK_ITEM_NAME	"TRACK"

/** MOUNT_ON_COORDINATES_SET.SYNC property item name.
 */
#define MOUNT_ON_COORDINATES_SET_SYNC_ITEM_NAME		"SYNC"

/** MOUNT_ON_COORDINATES_SET.SLEW property item name.
 */
#define MOUNT_ON_COORDINATES_SET_SLEW_ITEM_NAME		"SLEW"

//----------------------------------------------------------------------
/** MOUNT_SLEW_RATE property name.
 */
#define MOUNT_SLEW_RATE_PROPERTY_NAME						"MOUNT_SLEW_RATE"

/** MOUNT_SLEW_RATE.GUIDE property item name.
 */
#define MOUNT_SLEW_RATE_GUIDE_ITEM_NAME					"GUIDE"

/** MOUNT_SLEW_RATE.CENTERING property item name.
 */
#define MOUNT_SLEW_RATE_CENTERING_ITEM_NAME			"CENTERING"

/** MOUNT_SLEW_RATE.FIND property item name.
 */
#define MOUNT_SLEW_RATE_FIND_ITEM_NAME					"FIND"

/** MOUNT_SLEW_RATE.MAX property item name.
 */
#define MOUNT_SLEW_RATE_MAX_ITEM_NAME						"MAX"

//----------------------------------------------------------------------
/** MOUNT_MOTION_NS_PROPERTY_NAME property name.
 */
#define MOUNT_MOTION_NS_PROPERTY_NAME                "MOUNT_MOTION_NS"

/** MOUNT_MOTION_NS.NORTH property item name.
 */
#define MOUNT_MOTION_NORTH_ITEM_NAME                 "MOTION_NORTH"

/** MOUNT_MOTION_NS.SOUTH property item name.
 */
#define MOUNT_MOTION_SOUTH_ITEM_NAME                 "MOTION_SOUTH"

//----------------------------------------------------------------------
/** MOUNT_MOTION_WE_PROPERTY_NAME property name.
 */
#define MOUNT_MOTION_WE_PROPERTY_NAME                "MOUNT_MOTION_WE"

/** MOUNT_MOTION_NS.WEST property item name.
 */
#define MOUNT_MOTION_WEST_ITEM_NAME                 "MOTION_WEST"

/** MOUNT_MOTION_NS.EAST property item name.
 */
#define MOUNT_MOTION_EAST_ITEM_NAME                 "MOTION_EAST"

//----------------------------------------------------------------------
/** MOUNT_TRACK_RATE property name.
 */
#define MOUNT_TRACK_RATE_PROPERTY_NAME					"MOUNT_TRACK_RATE"

/** MOUNT_TRACK_RATE.SIDEREAL property item name.
 */
#define MOUNT_TRACK_RATE_SIDEREAL_ITEM_NAME			"SIDEREAL"

/** MOUNT_TRACK_RATE.SOLAR property item name.
 */
#define MOUNT_TRACK_RATE_SOLAR_ITEM_NAME				"SOLAR"

/** MOUNT_TRACK_RATE.LUNAR property item name.
 */
#define MOUNT_TRACK_RATE_LUNAR_ITEM_NAME				"LUNAR"

//----------------------------------------------------------------------
/** MOUNT_GUIDE_RATE property name.
 */
#define MOUNT_GUIDE_RATE_PROPERTY_NAME					"MOUNT_GUIDE_RATE"

/** MOUNT_GUIDE_RATE.RA property item name.
 */
#define MOUNT_GUIDE_RATE_RA_ITEM_NAME						"RA"

/** MOUNT_GUIDE_RATE.DEC property item name.
 */
#define MOUNT_GUIDE_RATE_DEC_ITEM_NAME					"DEC"

//----------------------------------------------------------------------
/** MOUNT_EQUATORIAL_COORDINATES property name.
 */
#define MOUNT_EQUATORIAL_COORDINATES_PROPERTY_NAME	"MOUNT_EQUATORIAL_COORDINATES"

/** MOUNT_EQUATORIAL_COORDINATES.RA property item name.
 */
#define MOUNT_EQUATORIAL_COORDINATES_RA_ITEM_NAME		"RA"

/** MOUNT_EQUATORIAL_COORDINATES.DEC property item name.
 */
#define MOUNT_EQUATORIAL_COORDINATES_DEC_ITEM_NAME	"DEC"

//----------------------------------------------------------------------
/** MOUNT_HORIZONTAL_COORDINATES property name.
 */
#define MOUNT_HORIZONTAL_COORDINATES_PROPERTY_NAME	"MOUNT_HORIZONTAL_COORDINATES"

/** MOUNT_HORIZONTAL_COORDINATES.ALT property item name.
 */
#define MOUNT_HORIZONTAL_COORDINATES_ALT_ITEM_NAME	"ALT"

/** MOUNT_HORIZONTAL_COORDINATES.AZ property item name.
 */
#define MOUNT_HORIZONTAL_COORDINATES_AZ_ITEM_NAME	"AZ"

//----------------------------------------------------------------------
/** MOUNT_ABORT_MOTION property name.
 */
#define MOUNT_ABORT_MOTION_PROPERTY_NAME				"MOUNT_ABORT_MOTION"

/** FOCUSER_ABORT_MOTION.ABORT_MOTION property item name.
 */
#define MOUNT_ABORT_MOTION_ITEM_NAME						"ABORT_MOTION"


#endif /* indigo_names_h */
