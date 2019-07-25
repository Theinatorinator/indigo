// Copyright (c) 2018 CloudMakers, s. r. o.
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
// 2.0 by Peter Polakovic <peter.polakovic@cloudmakers.eu>

/** INDIGO Optec filter wheel driver
 \file indigo_wheel_optec.c
 */

#define DRIVER_VERSION 0x0001
#define DRIVER_NAME "indigo_wheel_optec"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <sys/time.h>

#include <indigo/indigo_io.h>

#include "indigo_wheel_optec.h"

// -------------------------------------------------------------------------------- interface implementation

#define PRIVATE_DATA        ((optec_private_data *)device->private_data)

typedef struct {
	int handle;
	int slot;
} optec_private_data;

static bool optec_open(indigo_device *device) {
	char *name = DEVICE_PORT_ITEM->text.value;
	PRIVATE_DATA->handle = indigo_open_serial_with_speed(name, 19200);
	if (PRIVATE_DATA->handle >= 0) {
		char reply;
		INDIGO_DRIVER_LOG(DRIVER_NAME, "connected to %s", name);
		if (indigo_printf(PRIVATE_DATA->handle, "WSMODE\r\n") && indigo_scanf(PRIVATE_DATA->handle, "%c\r\n", &reply) == 1 && reply == '!') {
			indigo_printf(PRIVATE_DATA->handle, "WFILTR\r\n");
			if (indigo_scanf(PRIVATE_DATA->handle, "%d\r\n", &PRIVATE_DATA->slot) == 1) {
				WHEEL_SLOT_ITEM->number.value = PRIVATE_DATA->slot;
				return true;
			} else {
				INDIGO_DRIVER_ERROR(DRIVER_NAME, "failed to read current position");
			}
		} else {
			INDIGO_DRIVER_ERROR(DRIVER_NAME, "failed to initialize");
		}
	} else {
		INDIGO_DRIVER_ERROR(DRIVER_NAME, "failed to connect to %s", name);
	}
	return false;
}

static void optec_goto(indigo_device *device) {
	int slot = WHEEL_SLOT_ITEM->number.target;
	char reply;
	WHEEL_SLOT_PROPERTY->state = INDIGO_BUSY_STATE;
	indigo_update_property(device, WHEEL_SLOT_PROPERTY, NULL);
	WHEEL_SLOT_PROPERTY->state = INDIGO_ALERT_STATE;
	if (indigo_printf(PRIVATE_DATA->handle, "WGOTO%d\r\n", slot) && indigo_scanf(PRIVATE_DATA->handle, "%c\r\n", &reply) == 1 && reply == '*') {
		WHEEL_SLOT_ITEM->number.value = slot;
		WHEEL_SLOT_PROPERTY->state = INDIGO_OK_STATE;
	} else {
		INDIGO_DRIVER_ERROR(DRIVER_NAME, "failed to set position");
	}
	indigo_update_property(device, WHEEL_SLOT_PROPERTY, NULL);
}

static void optec_close(indigo_device *device) {
	if (PRIVATE_DATA->handle > 0) {
		indigo_printf(PRIVATE_DATA->handle, "WEXITS\r\n");
		close(PRIVATE_DATA->handle);
		PRIVATE_DATA->handle = 0;
		INDIGO_DRIVER_LOG(DRIVER_NAME, "disconnected from %s", DEVICE_PORT_ITEM->text.value);
	}
}

// -------------------------------------------------------------------------------- INDIGO wheel device implementation

static indigo_result wheel_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_wheel_attach(device, DRIVER_VERSION) == INDIGO_OK) {
		DEVICE_PORT_PROPERTY->hidden = false;
		DEVICE_PORTS_PROPERTY->hidden = false;
		WHEEL_SLOT_ITEM->number.max = 8;
		INDIGO_DEVICE_ATTACH_LOG(DRIVER_NAME, device->name);
		return indigo_wheel_enumerate_properties(device, NULL, NULL);
	}
	return INDIGO_FAILED;
}

static indigo_result wheel_change_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	assert(DEVICE_CONTEXT != NULL);
	assert(property != NULL);
	if (indigo_property_match(CONNECTION_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- CONNECTION
		indigo_property_copy_values(CONNECTION_PROPERTY, property, false);
		if (CONNECTION_CONNECTED_ITEM->sw.value) {
			if (optec_open(device)) {
				CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
			} else {
				CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
				indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
			}
		} else {
			optec_close(device);
			CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
		}
	} else if (indigo_property_match(WHEEL_SLOT_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- WHEEL_SLOT
		indigo_property_copy_values(WHEEL_SLOT_PROPERTY, property, false);
		if (WHEEL_SLOT_ITEM->number.value < 1 || WHEEL_SLOT_ITEM->number.value > WHEEL_SLOT_ITEM->number.max) {
			WHEEL_SLOT_PROPERTY->state = INDIGO_ALERT_STATE;
		} else if (WHEEL_SLOT_ITEM->number.value == PRIVATE_DATA->slot) {
			WHEEL_SLOT_PROPERTY->state = INDIGO_OK_STATE;
		} else {
			WHEEL_SLOT_PROPERTY->state = INDIGO_BUSY_STATE;
			WHEEL_SLOT_ITEM->number.value = PRIVATE_DATA->slot;
			indigo_set_timer(device, 0, optec_goto);
		}
		indigo_update_property(device, WHEEL_SLOT_PROPERTY, NULL);
		return INDIGO_OK;
		// --------------------------------------------------------------------------------
	}
	return indigo_wheel_change_property(device, client, property);
}

static indigo_result wheel_detach(indigo_device *device) {
	assert(device != NULL);
	if (CONNECTION_CONNECTED_ITEM->sw.value)
		indigo_device_disconnect(NULL, device->name);
	INDIGO_DEVICE_DETACH_LOG(DRIVER_NAME, device->name);
	return indigo_wheel_detach(device);
}

static indigo_device *device = NULL;

indigo_result indigo_wheel_optec(indigo_driver_action action, indigo_driver_info *info) {
	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;
	static indigo_device wheel_template = INDIGO_DEVICE_INITIALIZER(
		"Optec Filter Wheel",
		wheel_attach,
		indigo_wheel_enumerate_properties,
		wheel_change_property,
		NULL,
		wheel_detach
		);

	SET_DRIVER_INFO(info, "Optec Filter Wheel", __FUNCTION__, DRIVER_VERSION, false, last_action);

	if (action == last_action)
		return INDIGO_OK;

	switch (action) {
	case INDIGO_DRIVER_INIT:
		last_action = action;
		if (device == NULL) {
			device = malloc(sizeof(indigo_device));
			assert(device != NULL);
			memcpy(device, &wheel_template, sizeof(indigo_device));
			optec_private_data *private_data = malloc(sizeof(optec_private_data));
			assert(private_data != NULL);
			memset(private_data, 0, sizeof(optec_private_data));
			device->private_data = private_data;
			indigo_attach_device(device);
		}
		break;

	case INDIGO_DRIVER_SHUTDOWN:
		last_action = action;
		if (device != NULL) {
			indigo_detach_device(device);
			free(device->private_data);
			free(device);
			device = NULL;
		}
		break;

	case INDIGO_DRIVER_INFO:
		break;
	}

	return INDIGO_OK;
}
