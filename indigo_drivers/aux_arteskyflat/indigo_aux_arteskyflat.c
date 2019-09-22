// Copyright (c) 2019 CloudMakers, s. r. o.
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

/** INDIGO Artesky Flat Box USB aux driver
 \file indigo_aux_arteskyflat.c
 */

#define DRIVER_VERSION 0x0001
#define DRIVER_NAME "indigo_aux_arteskyflat"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <termios.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include <indigo/indigo_driver_xml.h>
#include <indigo/indigo_io.h>
#include "indigo_aux_arteskyflat.h"

#define PRIVATE_DATA												((arteskyflat_private_data *)device->private_data)

#define AUX_LIGHT_INTENSITY_PROPERTY				(PRIVATE_DATA->light_intensity_property)
#define AUX_LIGHT_INTENSITY_ITEM						(AUX_LIGHT_INTENSITY_PROPERTY->items+0)

typedef struct {
	int handle;
	indigo_property *light_intensity_property;
	pthread_mutex_t mutex;
} arteskyflat_private_data;

static bool artesky_command(int handle, char *command, char *response) {
	int result = indigo_write(handle, command, strlen(command));
	result |= indigo_write(handle, "\n", 1);
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "%d <- %s (%s)", handle, command, result ? "OK" : strerror(errno));
	if (result) {
		result = indigo_read_line(handle, response, 10) > 0;
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "%d -> %s (%s)", handle, response, result ? "OK" : strerror(errno));
	}
	return result;
}

// -------------------------------------------------------------------------------- INDIGO aux device implementation

static indigo_result aux_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_aux_attach(device, DRIVER_VERSION, INDIGO_INTERFACE_AUX_LIGHTBOX) == INDIGO_OK) {
		// -------------------------------------------------------------------------------- AUX_LIGHT_INTENSITY
		AUX_LIGHT_INTENSITY_PROPERTY = indigo_init_number_property(NULL, device->name, AUX_LIGHT_INTENSITY_PROPERTY_NAME, AUX_MAIN_GROUP, "Light intensity", INDIGO_OK_STATE, INDIGO_RW_PERM, 1);
		if (AUX_LIGHT_INTENSITY_PROPERTY == NULL)
			return INDIGO_FAILED;
		indigo_init_number_item(AUX_LIGHT_INTENSITY_ITEM, AUX_LIGHT_INTENSITY_ITEM_NAME, "Intensity", 0, 255, 1, 0);
		strcpy(AUX_LIGHT_INTENSITY_ITEM->number.format, "%g");
		// -------------------------------------------------------------------------------- DEVICE_PORT, DEVICE_PORTS
		DEVICE_PORT_PROPERTY->hidden = false;
		DEVICE_PORTS_PROPERTY->hidden = false;
#ifdef INDIGO_MACOS
		for (int i = 0; i < DEVICE_PORTS_PROPERTY->count; i++) {
			if (!strncmp(DEVICE_PORTS_PROPERTY->items[i].name, "/dev/cu.usbmodem", 16)) {
				strncpy(DEVICE_PORT_ITEM->text.value, DEVICE_PORTS_PROPERTY->items[i].name, INDIGO_VALUE_SIZE);
				break;
			}
		}
#endif
#ifdef INDIGO_LINUX
		strcpy(DEVICE_PORT_ITEM->text.value, "/dev/ttyARTESKYFLAT");
#endif
		// --------------------------------------------------------------------------------
		pthread_mutex_init(&PRIVATE_DATA->mutex, NULL);
		INDIGO_DEVICE_ATTACH_LOG(DRIVER_NAME, device->name);
		return indigo_aux_enumerate_properties(device, NULL, NULL);
	}
	return INDIGO_FAILED;
}

static indigo_result aux_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property) {
	if (IS_CONNECTED) {
		if (indigo_property_match(AUX_LIGHT_INTENSITY_PROPERTY, property))
			indigo_define_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
	}
	return indigo_aux_enumerate_properties(device, NULL, NULL);
}

static void aux_connection_handler(indigo_device *device) {
	char command[16], response[16];
	pthread_mutex_lock(&PRIVATE_DATA->mutex);
	if (CONNECTION_CONNECTED_ITEM->sw.value) {
		CONNECTION_PROPERTY->state = INDIGO_BUSY_STATE;
		indigo_update_property(device, CONNECTION_PROPERTY, NULL);
		PRIVATE_DATA->handle = indigo_open_serial(DEVICE_PORT_ITEM->text.value);
		if (PRIVATE_DATA->handle > 0) {
			INDIGO_DRIVER_LOG(DRIVER_NAME, "Connected on %s", DEVICE_PORT_ITEM->text.value);
			if (artesky_command(PRIVATE_DATA->handle, ">L000", response) && !strcmp(response, "*L19000")) {
				INDIGO_DRIVER_LOG(DRIVER_NAME, "Artesky Flat Box detected");
			} else {
				INDIGO_DRIVER_ERROR(DRIVER_NAME, "Handshake failed");
				close(PRIVATE_DATA->handle);
				PRIVATE_DATA->handle = 0;
			}
		}
		if (PRIVATE_DATA->handle > 0) {
			sprintf(command, ">B%03d", (int)(AUX_LIGHT_INTENSITY_ITEM->number.value));
			if (artesky_command(PRIVATE_DATA->handle, command, response))
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
			else
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_define_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
			CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
		} else {
			INDIGO_DRIVER_ERROR(DRIVER_NAME, "Failed to connect to %s", DEVICE_PORT_ITEM->text.value);
			CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
		}
	} else {
		indigo_delete_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
		artesky_command(PRIVATE_DATA->handle, ">B000", response);
		artesky_command(PRIVATE_DATA->handle, ">D000", response);
		close(PRIVATE_DATA->handle);
		PRIVATE_DATA->handle = 0;
		INDIGO_DRIVER_LOG(DRIVER_NAME, "Disconnected");
		CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
	}
	indigo_aux_change_property(device, NULL, CONNECTION_PROPERTY);
	pthread_mutex_unlock(&PRIVATE_DATA->mutex);
}

static void aux_intensity_handler(indigo_device *device) {
	pthread_mutex_lock(&PRIVATE_DATA->mutex);
	if (AUX_LIGHT_INTENSITY_PROPERTY->state != INDIGO_BUSY_STATE) {
		char command[16],	response[16];
		sprintf(command, ">B%03d", (int)(AUX_LIGHT_INTENSITY_ITEM->number.value));
		if (artesky_command(PRIVATE_DATA->handle, command, response))
			AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
		else
			AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
	}
	indigo_update_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
	pthread_mutex_unlock(&PRIVATE_DATA->mutex);
}

static indigo_result aux_change_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	assert(DEVICE_CONTEXT != NULL);
	assert(property != NULL);
	if (indigo_property_match(CONNECTION_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- CONNECTION
		indigo_property_copy_values(CONNECTION_PROPERTY, property, false);
		indigo_set_timer(device, 0, aux_connection_handler);
		return INDIGO_OK;
		// -------------------------------------------------------------------------------- X_CCD_EXPOSURE
	} else if (indigo_property_match(AUX_LIGHT_INTENSITY_PROPERTY, property)) {
		indigo_property_copy_values(AUX_LIGHT_INTENSITY_PROPERTY, property, false);
		indigo_set_timer(device, 0, aux_intensity_handler);
		return INDIGO_OK;
		// -------------------------------------------------------------------------------- CONFIG
	} else if (indigo_property_match(CONFIG_PROPERTY, property)) {
		if (indigo_switch_match(CONFIG_SAVE_ITEM, property)) {
			indigo_save_property(device, NULL, AUX_LIGHT_INTENSITY_PROPERTY);
		}
	}
	return indigo_aux_change_property(device, client, property);
}

static indigo_result aux_detach(indigo_device *device) {
	assert(device != NULL);
	if (CONNECTION_CONNECTED_ITEM->sw.value) {
		indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
		aux_connection_handler(device);
	}
	indigo_release_property(AUX_LIGHT_INTENSITY_PROPERTY);
	pthread_mutex_destroy(&PRIVATE_DATA->mutex);
	INDIGO_DEVICE_DETACH_LOG(DRIVER_NAME, device->name);
	return indigo_aux_detach(device);
}

// -------------------------------------------------------------------------------- INDIGO driver implementation

indigo_result indigo_aux_arteskyflat(indigo_driver_action action, indigo_driver_info *info) {
	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;
	static arteskyflat_private_data *private_data = NULL;
	static indigo_device *aux = NULL;
	
	static indigo_device aux_template = INDIGO_DEVICE_INITIALIZER(
		"Artesky Flat Box",
		aux_attach,
		aux_enumerate_properties,
		aux_change_property,
		NULL,
		aux_detach
		);
	
	SET_DRIVER_INFO(info, "Artesky Flat Box USB", __FUNCTION__, DRIVER_VERSION, false, last_action);
	
	if (action == last_action)
		return INDIGO_OK;
	
	switch (action) {
		case INDIGO_DRIVER_INIT:
			last_action = action;
			private_data = malloc(sizeof(arteskyflat_private_data));
			assert(private_data != NULL);
			memset(private_data, 0, sizeof(arteskyflat_private_data));
			aux = malloc(sizeof(indigo_device));
			assert(aux != NULL);
			memcpy(aux, &aux_template, sizeof(indigo_device));
			aux->private_data = private_data;
			indigo_attach_device(aux);
			break;
			
		case INDIGO_DRIVER_SHUTDOWN:
			last_action = action;
			if (aux != NULL) {
				indigo_detach_device(aux);
				free(aux);
				aux = NULL;
			}
			if (private_data != NULL) {
				free(private_data);
				private_data = NULL;
			}
			break;
			
		case INDIGO_DRIVER_INFO:
			break;
	}
	
	return INDIGO_OK;
}
