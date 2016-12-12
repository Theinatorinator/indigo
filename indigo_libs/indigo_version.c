// Copyright (c) 2016 CloudMakers, s. r. o.
// All rights reserved.
//
// You can use this software under the terms of 'INDIGO Astronomy
// open-source license' (see LICENSE.md).
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS 'AS IS' AND ANY EXPRESS
// OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// version history
// 2.0 Build 0 - PoC by Peter Polakovic <peter.polakovic@cloudmakers.eu>

/** INDIGO Version mapping
 \file indigo_version.c
 */

#include <string.h>

#include "indigo_version.h"
#include "indigo_names.h"

// <getProperties version='1.7' name='CONFIG'/>

struct property_mapping {
	char *legacy;
	char *current;
	struct item_mapping {
		char *legacy;
		char *current;
	} items[10];
};

static struct property_mapping legacy[] = {
	{	"CONNECTION", CONNECTION_PROPERTY_NAME, {
			{ "CONNECT", CONNECTION_CONNECTED_ITEM_NAME },
			{ "DISCONNECT", CONNECTION_DISCONNECTED_ITEM_NAME },
			NULL
		}
	},
	{	"DEBUG", DEBUG_PROPERTY_NAME, {
			{ "ENABLE", DEBUG_ENABLED_ITEM_NAME },
			{ "DISABLE", DEBUG_DISABLED_ITEM_NAME },
			NULL
		}
	},
	{	"SIMULATION", SIMULATION_PROPERTY_NAME, {
			{ "ENABLE", SIMULATION_ENABLED_ITEM_NAME },
			{ "DISABLE", SIMULATION_DISABLED_ITEM_NAME },
			NULL
		}
	},
	{	"CONFIG_PROCESS", CONFIG_PROPERTY_NAME, {
			{ "CONFIG_LOAD", CONFIG_LOAD_ITEM_NAME },
			{ "CONFIG_SAVE", CONFIG_SAVE_ITEM_NAME },
			{ "CONFIG_DEFAULT", CONFIG_DEFAULT_ITEM_NAME },
			NULL
		}
	},
	{	"DRIVER_INFO", INFO_PROPERTY_NAME, {
			{ "DRIVER_NAME", INFO_DEVICE_NAME_ITEM_NAME },
			{ "DRIVER_VERSION", INFO_DEVICE_VERSION_ITEM_NAME },
			{ "DRIVER_INTERFACE", INFO_DEVICE_INTERFACE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_INFO", CCD_INFO_PROPERTY_NAME, {
			{ "CCD_MAX_X", CCD_INFO_WIDTH_ITEM_NAME },
			{ "CCD_MAX_Y", CCD_INFO_HEIGHT_ITEM_NAME },
			{ "CCD_MAX_BIN_X", CCD_INFO_MAX_HORIZONAL_BIN_ITEM_NAME },
			{ "CCD_MAX_BIN_Y", CCD_INFO_MAX_VERTICAL_BIN_ITEM_NAME },
			{ "CCD_PIXEL_SIZE", CCD_INFO_PIXEL_SIZE_ITEM_NAME },
			{ "CCD_PIXEL_SIZE_X", CCD_INFO_PIXEL_WIDTH_ITEM_NAME },
			{ "CCD_PIXEL_SIZE_Y", CCD_INFO_PIXEL_HEIGHT_ITEM_NAME },
			{ "CCD_BITSPERPIXEL", CCD_INFO_BITS_PER_PIXEL_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_EXPOSURE", CCD_EXPOSURE_PROPERTY_NAME, {
			{ "CCD_EXPOSURE_VALUE", CCD_EXPOSURE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_ABORT_EXPOSURE", CCD_ABORT_EXPOSURE_PROPERTY_NAME, {
			{ "ABORT", CCD_ABORT_EXPOSURE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_FRAME", CCD_FRAME_PROPERTY_NAME, {
			{ "X", CCD_FRAME_LEFT_ITEM_NAME },
			{ "Y", CCD_FRAME_TOP_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_BINNING", CCD_BIN_PROPERTY_NAME, {
			{ "VER_BIN", CCD_BIN_HORIZONTAL_ITEM_NAME },
			{ "HOR_BIN", CCD_BIN_VERTICAL_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_FRAME_TYPE", CCD_FRAME_TYPE_PROPERTY_NAME, {
			{ "FRAME_LIGHT", CCD_FRAME_TYPE_LIGHT_ITEM_NAME },
			{ "FRAME_BIAS", CCD_FRAME_TYPE_BIAS_ITEM_NAME },
			{ "FRAME_DARK", CCD_FRAME_TYPE_DARK_ITEM_NAME },
			{ "FRAME_FLAT", CCD_FRAME_TYPE_FLAT_ITEM_NAME },
			NULL
		}
	},
	{	"UPLOAD_MODE", CCD_UPLOAD_MODE_PROPERTY_NAME, {
			{ "UPLOAD_CLIENT", CCD_UPLOAD_MODE_CLIENT_ITEM_NAME },
			{ "UPLOAD_LOCAL", CCD_UPLOAD_MODE_LOCAL_ITEM_NAME },
			{ "UPLOAD_BOTH", CCD_UPLOAD_MODE_BOTH_ITEM_NAME },
			NULL
		}
	},
	{	"UPLOAD_SETTINGS", CCD_LOCAL_MODE_PROPERTY_NAME, {
			{ "UPLOAD_DIR", CCD_LOCAL_MODE_DIR_ITEM_NAME },
			{ "UPLOAD_PREFIX", CCD_LOCAL_MODE_PREFIX_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_FILE_PATH", CCD_IMAGE_FILE_PROPERTY_NAME, {
			{ "FILE_PATH", CCD_IMAGE_FILE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD1", CCD_IMAGE_PROPERTY_NAME, {
			{ "CCD1", CCD_IMAGE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_TEMPERATURE", CCD_TEMPERATURE_PROPERTY_NAME, {
			{ "CCD_TEMPERATURE_VALUE", CCD_TEMPERATURE_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_COOLER", CCD_COOLER_PROPERTY_NAME, {
			{ "COOLER_ON", CCD_COOLER_ON_ITEM_NAME },
			{ "COOLER_OFF", CCD_COOLER_OFF_ITEM_NAME },
			NULL
		}
	},
	{	"CCD_COOLER_POWER", CCD_COOLER_POWER_PROPERTY_NAME, {
			{ "CCD_COOLER_VALUE", CCD_COOLER_POWER_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_TIMED_GUIDE_NS", GUIDER_GUIDE_DEC_PROPERTY_NAME, {
			{ "TIMED_GUIDE_N", GUIDER_GUIDE_NORTH_ITEM_NAME },
			{ "TIMED_GUIDE_S", GUIDER_GUIDE_SOUTH_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_TIMED_GUIDE_WE", GUIDER_GUIDE_RA_PROPERTY_NAME, {
			{ "TIMED_GUIDE_W", GUIDER_GUIDE_WEST_ITEM_NAME },
			{ "TIMED_GUIDE_E", GUIDER_GUIDE_EAST_ITEM_NAME },
			NULL
		}
	},
	{	"FILTER_SLOT", WHEEL_SLOT_NAME_PROPERTY_NAME, {
			{ "FILTER_SLOT_VALUE", WHEEL_SLOT_ITEM_NAME },
			NULL
		}
	},
	{	"FILTER_NAME_VALUE", WHEEL_SLOT_PROPERTY_NAME, {
			{ "FILTER_SLOT_NAME_1", WHEEL_SLOT_NAME_1_ITEM_NAME },
			{ "FILTER_SLOT_NAME_2", WHEEL_SLOT_NAME_2_ITEM_NAME },
			{ "FILTER_SLOT_NAME_3", WHEEL_SLOT_NAME_3_ITEM_NAME },
			{ "FILTER_SLOT_NAME_4", WHEEL_SLOT_NAME_4_ITEM_NAME },
			{ "FILTER_SLOT_NAME_5", WHEEL_SLOT_NAME_5_ITEM_NAME },
			{ "FILTER_SLOT_NAME_6", WHEEL_SLOT_NAME_6_ITEM_NAME },
			{ "FILTER_SLOT_NAME_7", WHEEL_SLOT_NAME_7_ITEM_NAME },
			{ "FILTER_SLOT_NAME_8", WHEEL_SLOT_NAME_8_ITEM_NAME },
			{ "FILTER_SLOT_NAME_9", WHEEL_SLOT_NAME_9_ITEM_NAME },
			NULL
		}
	},
	{	"FOCUS_SPEED", FOCUSER_SPEED_PROPERTY_NAME, {
			{ "FOCUS_SPEED_VALUE", FOCUSER_SPEED_ITEM_NAME },
			NULL
		}
	},
	{	"FOCUS_MOTION", FOCUSER_DIRECTION_PROPERTY_NAME, {
			{ "FOCUS_INWARD", FOCUSER_DIRECTION_MOVE_INWARD_ITEM_NAME },
			{ "FOCUS_OUTWARD", FOCUSER_DIRECTION_MOVE_OUTWARD_ITEM_NAME },
			NULL
		}
	},
	{	"REL_FOCUS_POSITION", FOCUSER_STEPS_PROPERTY_NAME, {
			{ "FOCUS_RELATIVE_POSITION", FOCUSER_STEPS_ITEM_NAME },
			NULL
		}
	},
	{	"ABS_FOCUS_POSITION", FOCUSER_POSITION_PROPERTY_NAME, {
			{ "FOCUS_ABSOLUTE_POSITION", FOCUSER_POSITION_ITEM_NAME },
			NULL
		}
	},
	{	"FOCUS_ABORT_MOTION", FOCUSER_ABORT_MOTION_PROPERTY_NAME, {
			{ "ABORT", FOCUSER_ABORT_MOTION_ITEM_NAME },
			NULL
		}
	},
	{	"GEOGRAPHIC_COORD", MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY_NAME, {
			{ "LAT", MOUNT_GEOGRAPHIC_COORDINATES_LATITUDE_ITEM_NAME },
			{ "LONG", MOUNT_GEOGRAPHIC_COORDINATES_LONGITUDE_ITEM_NAME },
			{ "ELEV", MOUNT_GEOGRAPHIC_COORDINATES_ELEVATION_ITEM_NAME },
			NULL
		}
	},
	{	"TIME_LST", MOUNT_LST_TIME_PROPERTY_NAME, {
			{ "LST", MOUNT_LST_TIME_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_PARK", MOUNT_PARK_PROPERTY_NAME, {
			{ "PARK", MOUNT_PARK_PARKED_ITEM_NAME },
			{ "UNPARK", MOUNT_PARK_UNPARKED_ITEM_NAME },
			NULL
		}
	},
	{	"EQUATORIAL_EOD_COORD", MOUNT_EQUATORIAL_COORDINATES_PROPERTY_NAME, {
			{ "RA", MOUNT_EQUATORIAL_COORDINATES_RA_ITEM_NAME },
			{ "DEC", MOUNT_EQUATORIAL_COORDINATES_DEC_ITEM_NAME },
			NULL
		}
	},
	{	"HORIZONTAL_COORD", MOUNT_HORIZONTAL_COORDINATES_PROPERTY_NAME, {
			{ "ALT", MOUNT_HORIZONTAL_COORDINATES_ALT_ITEM_NAME },
			{ "AZ", MOUNT_HORIZONTAL_COORDINATES_AZ_ITEM_NAME },
			NULL
		}
	},
	{	"ON_COORD_SET", MOUNT_ON_COORDINATES_SET_PROPERTY_NAME, {
			{ "SLEW", MOUNT_ON_COORDINATES_SET_SLEW_ITEM_NAME },
			{ "TRACK", MOUNT_ON_COORDINATES_SET_TRACK_ITEM_NAME },
			{ "SYNC", MOUNT_ON_COORDINATES_SET_SYNC_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_SLEW_RATE", MOUNT_SLEW_RATE_PROPERTY_NAME, {
			{ "SLEW_GUIDE", MOUNT_SLEW_RATE_GUIDE_ITEM_NAME },
			{ "SLEW_CENTERING", MOUNT_SLEW_RATE_CENTERING_ITEM_NAME },
			{ "SLEW_FIND", MOUNT_SLEW_RATE_FIND_ITEM_NAME },
			{ "SLEW_MAX", MOUNT_SLEW_RATE_MAX_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_TRACK_RATE", MOUNT_TRACK_RATE_PROPERTY_NAME, {
			{ "TRACK_SIDEREAL", MOUNT_TRACK_RATE_SIDEREAL_ITEM_NAME },
			{ "TRACK_SOLAR", MOUNT_TRACK_RATE_SOLAR_ITEM_NAME },
			{ "TRACK_LUNAR", MOUNT_TRACK_RATE_LUNAR_ITEM_NAME },
			NULL
		}
	},
	{	"TELESCOPE_ABORT_MOTION", FOCUSER_ABORT_MOTION_PROPERTY_NAME, {
			{ "ABORT_MOTION", FOCUSER_ABORT_MOTION_ITEM_NAME },
			NULL
		}
	},
	NULL
};

void indigo_copy_property_name(indigo_version version, indigo_property *property, const char *name) {
	if (version == INDIGO_VERSION_LEGACY) {
		struct property_mapping *property_mapping = legacy;
		while (property_mapping->legacy) {
			if (!strcmp(name, property_mapping->legacy)) {
				INDIGO_DEBUG(indigo_debug("version: %s -> %s (current)", property_mapping->legacy, property_mapping->current));
				strcpy(property->name, property_mapping->current);
				return;
			}
			property_mapping++;
		}
	}
	strncpy(property->name, name, INDIGO_NAME_SIZE);
}

void indigo_copy_item_name(indigo_version version, indigo_property *property, indigo_item *item, const char *name) {
	if (version == INDIGO_VERSION_LEGACY) {
		struct property_mapping *property_mapping = legacy;
		while (property_mapping->legacy) {
			if (!strcmp(property->name, property_mapping->current)) {
				struct item_mapping *item_mapping = property_mapping->items;
				while (item_mapping->legacy) {
					if (!strcmp(name, item_mapping->legacy)) {
						INDIGO_DEBUG(indigo_debug("version: %s.%s -> %s.%s (current)", property_mapping->legacy, item_mapping->legacy, property_mapping->current, item_mapping->current));
						strncpy(item->name, item_mapping->current, INDIGO_NAME_SIZE);
						return;
					}
					item_mapping++;
				}
				strncpy(item->name, name, INDIGO_NAME_SIZE);
				return;
			}
			property_mapping++;
		}
	}
	strncpy(item->name, name, INDIGO_NAME_SIZE);
}

const char *indigo_property_name(indigo_version version, indigo_property *property) {
	if (version == INDIGO_VERSION_LEGACY) {
		struct property_mapping *property_mapping = legacy;
		while (property_mapping->legacy) {
			if (!strcmp(property->name, property_mapping->current)) {
				INDIGO_DEBUG(indigo_debug("version: %s -> %s (legacy)", property_mapping->current, property_mapping->legacy));
				return property_mapping->legacy;
			}
			property_mapping++;
		}
	}
	return property->name;
}

const char *indigo_item_name(indigo_version version, indigo_property *property, indigo_item *item) {
	if (version == INDIGO_VERSION_LEGACY) {
		struct property_mapping *property_mapping = legacy;
		while (property_mapping->legacy) {
			if (!strcmp(property->name, property_mapping->current)) {
				struct item_mapping *item_mapping = property_mapping->items;
				while (item_mapping->legacy) {
					if (!strcmp(item->name, item_mapping->current)) {
						INDIGO_DEBUG(indigo_debug("version: %s.%s -> %s.%s (legacy)", property_mapping->current, item_mapping->current, property_mapping->legacy, item_mapping->legacy));
						return item_mapping->legacy;
					}
					item_mapping++;
				}
				return item->name;
			}
			property_mapping++;
		}
	}
	return item->name;
}


