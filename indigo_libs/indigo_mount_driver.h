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

/** INDIGO Filter mount Driver base
 \file indigo_mount_driver.h
 */

#ifndef indigo_mount_h
#define indigo_mount_h

#include "indigo_bus.h"
#include "indigo_driver.h"

/** Main mount group name string.
 */
#define MOUNT_MAIN_GROUP															"Mount"

/** Main site group name string.
 */
#define MOUNT_SITE_GROUP															"Site"

/** Device context pointer.
 */
#define MOUNT_CONTEXT													((indigo_mount_context *)device->device_context)

/** MOUNT_GEOGRAPHIC_COORDINATES property pointer, property is mandatory, property change request should be fully handled by device driver.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY					(MOUNT_CONTEXT->mount_geographic_coordinates_property)

/** MOUNT_GEOGRAPHIC_COORDINATES.LATITUDE property item pointer.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_LATITUDE_ITEM		(MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY->items+0)

/** MOUNT_GEOGRAPHIC_COORDINATES.LONGITUDE property item pointer.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_LONGITUDE_ITEM		(MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY->items+1)

/** MOUNT_GEOGRAPHIC_COORDINATES.ELEVATION property item pointer.
 */
#define MOUNT_GEOGRAPHIC_COORDINATES_ELEVATION_ITEM		(MOUNT_GEOGRAPHIC_COORDINATES_PROPERTY->items+2)

/** MOUNT_LST_TIME property pointer, property is optional, property change request should be fully handled by device driver.
 */
#define MOUNT_LST_TIME_PROPERTY												(MOUNT_CONTEXT->mount_lst_time_property)

/** MOUNT_TIME.TIME property item pointer.
 */
#define MOUNT_LST_TIME_ITEM														(MOUNT_LST_TIME_PROPERTY->items+0)

/** MOUNT_PARK property pointer, property is mandatory, property change request should be fully handled by device driver.
 */
#define MOUNT_PARK_PROPERTY														(MOUNT_CONTEXT->mount_park_property)

/** MOUNT_PARK.PARKED property item pointer.
 */
#define MOUNT_PARK_PARKED_ITEM												(MOUNT_PARK_PROPERTY->items+0)

/** MOUNT_PARK.UNPARKED property item pointer.
 */
#define MOUNT_PARK_UNPARKED_ITEM											(MOUNT_PARK_PROPERTY->items+1)

/** MOUNT_ON_COORDINATES_SET property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_ON_COORDINATES_SET_PROPERTY							(MOUNT_CONTEXT->mount_on_coordinates_set_property)

/** MOUNT_ON_COORDINATES_SET.TRACK property item pointer.
 */
#define MOUNT_ON_COORDINATES_SET_TRACK_ITEM						(MOUNT_ON_COORDINATES_SET_PROPERTY->items+0)

/** MOUNT_ON_COORDINATES_SET.SYNC property item pointer.
 */
#define MOUNT_ON_COORDINATES_SET_SYNC_ITEM						(MOUNT_ON_COORDINATES_SET_PROPERTY->items+1)

/** MOUNT_ON_COORDINATES_SET.SLEW property item pointer.
 */
#define MOUNT_ON_COORDINATES_SET_SLEW_ITEM						(MOUNT_ON_COORDINATES_SET_PROPERTY->items+2)

/** MOUNT_SLEW_RATE property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_SLEW_RATE_PROPERTY											(MOUNT_CONTEXT->mount_slew_rate_property)

/** MOUNT_SLEW_RATE.GUIDE property item pointer.
 */
#define MOUNT_SLEW_RATE_GUIDE_ITEM										(MOUNT_SLEW_RATE_PROPERTY->items+0)

/** MOUNT_SLEW_RATE.CENTERING property item pointer.
 */
#define MOUNT_SLEW_RATE_CENTERING_ITEM								(MOUNT_SLEW_RATE_PROPERTY->items+1)

/** MOUNT_SLEW_RATE.FIND property item pointer.
 */
#define MOUNT_SLEW_RATE_FIND_ITEM											(MOUNT_SLEW_RATE_PROPERTY->items+2)

/** MOUNT_SLEW_RATE.MAX property item pointer.
 */
#define MOUNT_SLEW_RATE_MAX_ITEM											(MOUNT_SLEW_RATE_PROPERTY->items+3)

/** MOUNT_MOTION_NS property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_MOTION_NS_PROPERTY											(MOUNT_CONTEXT->mount_motion_ns_property)

/** MOUNT_MOTION_NS.NORTH property item pointer.
 */
#define  MOUNT_MOTION_NORTH_ITEM											(MOUNT_MOTION_NS_PROPERTY->items+0)

/** MOUNT_MOTION_NS.SOUTH property item pointer.
 */
#define  MOUNT_MOTION_SOUTH_ITEM						          (MOUNT_MOTION_NS_PROPERTY->items+1)

/** MOUNT_MOTION_WE property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_MOTION_WE_PROPERTY											(MOUNT_CONTEXT->mount_motion_we_property)

/** MOUNT_MOTION_WE.WEST property item pointer.
 */
#define  MOUNT_MOTION_WEST_ITEM												(MOUNT_MOTION_WE_PROPERTY->items+0)

/** MOUNT_MOTION_WE.EAST property item pointer.
 */
#define  MOUNT_MOTION_EAST_ITEM						            (MOUNT_MOTION_WE_PROPERTY->items+1)

/** MOUNT_TRACK_RATE property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_TRACK_RATE_PROPERTY											(MOUNT_CONTEXT->mount_track_rate_property)

/** MOUNT_TRACK_RATE.SIDEREAL property item pointer.
 */
#define MOUNT_TRACK_RATE_SIDEREAL_ITEM								(MOUNT_TRACK_RATE_PROPERTY->items+0)

/** MOUNT_TRACK_RATE.SOLAR property item pointer.
 */
#define MOUNT_TRACK_RATE_SOLAR_ITEM										(MOUNT_TRACK_RATE_PROPERTY->items+1)

/** MOUNT_TRACK_RATE.LINAR property item pointer.
 */
#define MOUNT_TRACK_RATE_LUNAR_ITEM										(MOUNT_TRACK_RATE_PROPERTY->items+2)

/** MOUNT_GUIDE_RATE property pointer, property is mandatory, property change request is handled by indigo_mount_change_property.
 */
#define MOUNT_GUIDE_RATE_PROPERTY											(MOUNT_CONTEXT->mount_guide_rate_property)

/** MOUNT_GUIDE_RATE.RA property item pointer.
 */
#define MOUNT_GUIDE_RATE_RA_ITEM											(MOUNT_GUIDE_RATE_PROPERTY->items+0)

/** MOUNT_GUIDE_RATE.DEC property item pointer.
 */
#define MOUNT_GUIDE_RATE_DEC_ITEM											(MOUNT_GUIDE_RATE_PROPERTY->items+1)

/** MOUNT_EQUATORIAL_COORDINATES property pointer, property is mandatory, property change request should be fully handled by device driver.
 */
#define MOUNT_EQUATORIAL_COORDINATES_PROPERTY					(MOUNT_CONTEXT->mount_equatorial_coordinates_property)

/** MOUNT_EQUATORIAL_COORDINATES.RA property item pointer.
 */
#define MOUNT_EQUATORIAL_COORDINATES_RA_ITEM					(MOUNT_EQUATORIAL_COORDINATES_PROPERTY->items+0)

/** MOUNT_EQUATORIAL_COORDINATES.DEC property item pointer.
 */
#define MOUNT_EQUATORIAL_COORDINATES_DEC_ITEM					(MOUNT_EQUATORIAL_COORDINATES_PROPERTY->items+1)

/** MOUNT_HORIZONTAL_COORDINATES property pointer, property is optional, property change request should be fully handled by device driver.
 */
#define MOUNT_HORIZONTAL_COORDINATES_PROPERTY					(MOUNT_CONTEXT->mount_horizontal_coordinates_property)

/** MOUNT_HORIZONTAL_COORDINATES.ALT property item pointer.
 */
#define MOUNT_HORIZONTAL_COORDINATES_ALT_ITEM					(MOUNT_HORIZONTAL_COORDINATES_PROPERTY->items+0)

/** MOUNT_HORIZONTAL_COORDINATES.AZ property item pointer.
 */
#define MOUNT_HORIZONTAL_COORDINATES_AZ_ITEM					(MOUNT_HORIZONTAL_COORDINATES_PROPERTY->items+1)

/** MOUNT_ABORT_MOTION property pointer, property is mandatory, property change request should be fully handled by focuser driver
 */
#define MOUNT_ABORT_MOTION_PROPERTY										(MOUNT_CONTEXT->mount_abort_motion_property)

/** FOCUSER_ABORT_MOTION.ABORT_MOTION property item pointer.
 */
#define MOUNT_ABORT_MOTION_ITEM												(MOUNT_ABORT_MOTION_PROPERTY->items+0)

/** Wheel device context structure.
 */
typedef struct {
	indigo_device_context device_context;										///< device context base
	indigo_property *mount_geographic_coordinates_property;	///< MOUNT_GEOGRAPHIC_COORDINATES property pointer
	indigo_property *mount_lst_time_property;								///< MOUNT_LST_TIME property pointer
	indigo_property *mount_park_property;										///< MOUNT_PARK property pointer
	indigo_property *mount_on_coordinates_set_property;			///< MOUNT_ON_COORDINATES_SET property pointer
	indigo_property *mount_slew_rate_property;							///< MOUNT_SLEW_RATE property pointer
	indigo_property *mount_track_rate_property;							///< MOUNT_TRACK_RATE property pointer
	indigo_property *mount_guide_rate_property;							///< MOUNT_GUIDE_RATE property pointer
	indigo_property *mount_equatorial_coordinates_property;	///< MOUNT_EQUATORIAL_COORDINATES property pointer
	indigo_property *mount_horizontal_coordinates_property;	///< MOUNT_HORIZONTAL_COORDINATES property pointer
	indigo_property *mount_abort_motion_property;						///< MOUNT_ABORT_MOTION property pointer
	indigo_property *mount_motion_ns_property;							///< MOUNT_MOTION_NS property pointer
	indigo_property *mount_motion_we_property;							///< MOUNT_MOTION_WE property pointer
} indigo_mount_context;

/** Attach callback function.
 */
extern indigo_result indigo_mount_attach(indigo_device *device, unsigned version);
/** Enumerate properties callback function.
 */
extern indigo_result indigo_mount_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property);
/** Change property callback function.
 */
extern indigo_result indigo_mount_change_property(indigo_device *device, indigo_client *client, indigo_property *property);
/** Detach callback function.
 */
extern indigo_result indigo_mount_detach(indigo_device *device);

#endif /* indigo_mount_h */

