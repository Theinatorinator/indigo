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

/** INDIGO Focuser Driver base
 \file indigo_focuser_driver.h
 */

#ifndef indigo_focuser_h
#define indigo_focuser_h

#include "indigo_bus.h"
#include "indigo_driver.h"

/** Main focuser group name string.
 */
#define FOCUSER_MAIN_GROUP                "Main"

/** Device context pointer.
 */
#define FOCUSER_CONTEXT                ((indigo_focuser_context *)device->device_context)

/** FOCUSER_SPEED property pointer, property is mandatory, property change request should be fully handled by indigo_focuser_change_property
 */
#define FOCUSER_SPEED_PROPERTY								(FOCUSER_CONTEXT->focuser_speed_property)

/** FOCUSER_SPEED.SPEED property item pointer.
 */
#define FOCUSER_SPEED_ITEM										(FOCUSER_SPEED_PROPERTY->items+0)

/** FOCUSER_DIRECTION property pointer, property is mandatory, property change request should be fully handled by indigo_focuser_change_property.
 */
#define FOCUSER_DIRECTION_PROPERTY						(FOCUSER_CONTEXT->focuser_direction_property)

/** FOCUSER_DIRECTION.MOVE_INWARD property item pointer.
 */
#define FOCUSER_DIRECTION_MOVE_INWARD_ITEM		(FOCUSER_DIRECTION_PROPERTY->items+0)

/** FOCUSER_DIRECTION.MOVE_OUTWARD property item pointer.
 */
#define FOCUSER_DIRECTION_MOVE_OUTWARD_ITEM		(FOCUSER_DIRECTION_PROPERTY->items+1)

/** FOCUSER_STEPS property pointer, property is mandatory, property change request should be fully handled by focuser driver
 */
#define FOCUSER_STEPS_PROPERTY								(FOCUSER_CONTEXT->focuser_steps_property)

/** FOCUSER_STEPS.STEPS property item pointer.
 */
#define FOCUSER_STEPS_ITEM										(FOCUSER_STEPS_PROPERTY->items+0)

/** FOCUSER_POSITION property pointer, property is mandatory, property change request should be fully handled by focuser driver
 */
#define FOCUSER_POSITION_PROPERTY							(FOCUSER_CONTEXT->focuser_position_property)

/** FOCUSER_POSITION.POSITION property item pointer.
 */
#define FOCUSER_POSITION_ITEM									(FOCUSER_POSITION_PROPERTY->items+0)

/** FOCUSER_ABORT_MOTION property pointer, property is mandatory, property change request should be fully handled by focuser driver
 */
#define FOCUSER_ABORT_MOTION_PROPERTY					(FOCUSER_CONTEXT->focuser_abort_motion_property)

/** FOCUSER_ABORT_MOTION.ABORT_MOTION property item pointer.
 */
#define FOCUSER_ABORT_MOTION_ITEM							(FOCUSER_ABORT_MOTION_PROPERTY->items+0)


/** Focuser device context structure.
 */
typedef struct {
	indigo_device_context device_context;						///< device context base
	indigo_property *focuser_speed_property;				///< FOCUSER_SPEED property pointer
	indigo_property *focuser_direction_property;		///< FOCUSER_DIRECTION property pointer
	indigo_property *focuser_steps_property;				///< FOCUSER_STEPS property pointer
	indigo_property *focuser_position_property;			///< FOCUSER_POSITION property pointer
	indigo_property *focuser_abort_motion_property;	///< FOCUSER_ABORT_MOTION property pointer
} indigo_focuser_context;

/** Attach callback function.
 */
extern indigo_result indigo_focuser_attach(indigo_device *device, unsigned version);
/** Enumerate properties callback function.
 */
extern indigo_result indigo_focuser_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property);
/** Change property callback function.
 */
extern indigo_result indigo_focuser_change_property(indigo_device *device, indigo_client *client, indigo_property *property);
/** Detach callback function.
 */
extern indigo_result indigo_focuser_detach(indigo_device *device);

#endif /* indigo_focuser_h */

