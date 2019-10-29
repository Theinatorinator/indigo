// Copyright (c) 2019 Rumen G. Bogdanovski
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
// 2.0 by Rumen G. Bogdanovski <rumen@skyarchive.org>

/** INDIGO Lacerta FBC aux driver
 \file indigo_aux_fbc.c
 */

#define DRIVER_VERSION 0x0001
#define DRIVER_NAME "indigo_aux_fbc"

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
#include "indigo_aux_fbc.h"

#define PRIVATE_DATA                                          ((fbc_private_data *)device->private_data)

#define AUX_LIGHT_SWITCH_PROPERTY                             (PRIVATE_DATA->light_switch_property)
#define AUX_LIGHT_SWITCH_ON_ITEM                              (AUX_LIGHT_SWITCH_PROPERTY->items+0)
#define AUX_LIGHT_SWITCH_OFF_ITEM                             (AUX_LIGHT_SWITCH_PROPERTY->items+1)

#define AUX_LIGHT_INTENSITY_PROPERTY                          (PRIVATE_DATA->light_intensity_property)
#define AUX_LIGHT_INTENSITY_ITEM                              (AUX_LIGHT_INTENSITY_PROPERTY->items+0)

/* bring 220-20 in range of 0-100 */
#define CALCULATE_INTENSITY(intensity)                        ((int)floor((100 - (int)(intensity) - 0) * (220 - 20) / (100 - 0) + 20))

typedef struct {
	int handle;
	indigo_property *light_switch_property;
	indigo_property *light_intensity_property;
	pthread_mutex_t mutex;
} fbc_private_data;

static bool fbc_command(int handle, char *command, char *response, int resp_len) {
	if(response) {
		indigo_usleep(20000);
		tcflush(handle, TCIOFLUSH);
	}

	int result = indigo_write(handle, command, strlen(command));
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "%d <- %s (%s)", handle, command, result ? "OK" : strerror(errno));

	if ((result) && (response)) {
		READ_AGAIN:
		*response = 0;
		result = indigo_read_line(handle, response, resp_len) > 0;
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "%d -> %s (%s)", handle, response, result ? "OK" : strerror(errno));
		if ((result) && (!strncmp("D -", response, 3))) goto READ_AGAIN;
	}
	return result;
}

static void aux_intensity_handler(indigo_device *device);
static void aux_switch_handler(indigo_device *device);

// -------------------------------------------------------------------------------- INDIGO aux device implementation

static indigo_result aux_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_aux_attach(device, DRIVER_VERSION, INDIGO_INTERFACE_AUX_LIGHTBOX | INDIGO_INTERFACE_AUX_SHUTTER) == INDIGO_OK) {
		INFO_PROPERTY->count = 5;
		// -------------------------------------------------------------------------------- AUX_LIGHT_SWITCH
		AUX_LIGHT_SWITCH_PROPERTY = indigo_init_switch_property(NULL, device->name, AUX_LIGHT_SWITCH_PROPERTY_NAME, AUX_MAIN_GROUP, "Light (on/off)", INDIGO_OK_STATE, INDIGO_RW_PERM, INDIGO_ONE_OF_MANY_RULE, 2);
		if (AUX_LIGHT_SWITCH_PROPERTY == NULL)
			return INDIGO_FAILED;
		indigo_init_switch_item(AUX_LIGHT_SWITCH_ON_ITEM, AUX_LIGHT_SWITCH_ON_ITEM_NAME, "On", false);
		indigo_init_switch_item(AUX_LIGHT_SWITCH_OFF_ITEM, AUX_LIGHT_SWITCH_OFF_ITEM_NAME, "Off", true);
		// -------------------------------------------------------------------------------- AUX_LIGHT_INTENSITY
		AUX_LIGHT_INTENSITY_PROPERTY = indigo_init_number_property(NULL, device->name, AUX_LIGHT_INTENSITY_PROPERTY_NAME, AUX_MAIN_GROUP, "Light intensity", INDIGO_OK_STATE, INDIGO_RW_PERM, 1);
		if (AUX_LIGHT_INTENSITY_PROPERTY == NULL)
			return INDIGO_FAILED;
		indigo_init_number_item(AUX_LIGHT_INTENSITY_ITEM, AUX_LIGHT_INTENSITY_ITEM_NAME, "Intensity", 0, 100, 1, 0);
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
		strcpy(DEVICE_PORT_ITEM->text.value, "/dev/lacertaFBC");
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
		if (indigo_property_match(AUX_LIGHT_SWITCH_PROPERTY, property))
			indigo_define_property(device, AUX_LIGHT_SWITCH_PROPERTY, NULL);
	}
	return indigo_aux_enumerate_properties(device, NULL, NULL);
}

static void aux_connection_handler(indigo_device *device) {
	char command[160], response[160];
	pthread_mutex_lock(&PRIVATE_DATA->mutex);
	if (CONNECTION_CONNECTED_ITEM->sw.value) {
		CONNECTION_PROPERTY->state = INDIGO_BUSY_STATE;
		indigo_update_property(device, CONNECTION_PROPERTY, NULL);
		for (int i = 0; i < 2; i++) {
			PRIVATE_DATA->handle = indigo_open_serial(DEVICE_PORT_ITEM->text.value);
			if (PRIVATE_DATA->handle > 0) {
				int fc_flag;
				fc_flag = TIOCM_RTS;   /* Modem Constant for RTS pin */
				ioctl(PRIVATE_DATA->handle,TIOCMBIC,&fc_flag);
				fc_flag = TIOCM_CTS;   /* Modem Constant for CTS pin */
				ioctl(PRIVATE_DATA->handle,TIOCMBIC,&fc_flag);


				INDIGO_DRIVER_LOG(DRIVER_NAME, "Connected on %s", DEVICE_PORT_ITEM->text.value);
				if (fbc_command(PRIVATE_DATA->handle, ": I #", response, sizeof(response)) && !strcmp("I FBC", response)) {
					if (fbc_command(PRIVATE_DATA->handle, ": P #", response, sizeof(response))) {
						if (strcmp("P SerialMode", response)) {
							INDIGO_DRIVER_ERROR(DRIVER_NAME, "FBC is not in SerialMode. Turn all knobs to 0 and powercycle the device.");
							indigo_send_message(device, "FBC is not in SerialMode. Turn all knobs to 0 and powercycle the device.");
							close(PRIVATE_DATA->handle);
							PRIVATE_DATA->handle = 0;
							break;
						}
					}
				} else {
					INDIGO_DRIVER_ERROR(DRIVER_NAME, "Handshake failed");
					close(PRIVATE_DATA->handle);
					PRIVATE_DATA->handle = 0;
				}
			}
		}
		if (PRIVATE_DATA->handle > 0) {
			if (fbc_command(PRIVATE_DATA->handle, ": V #", response, sizeof(response))) {
				sscanf(response, "V %s", INFO_DEVICE_FW_REVISION_ITEM->text.value);
				indigo_update_property(device, INFO_PROPERTY, NULL);
			}
			/* fbc does not report intensity and ON/OFF state, so we set it to be consistent */
			/* bring 220-20 in range of 0-100 */

			if (fbc_command(PRIVATE_DATA->handle, ": P #", response, sizeof(response)))
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
			else
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_define_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);

			sprintf(command, ": E 15000 #");
			if (fbc_command(PRIVATE_DATA->handle, command, NULL, 0))
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
			else
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_define_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
			sprintf(command, ": F 15000 #");
			if (fbc_command(PRIVATE_DATA->handle, command, NULL, 0))
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
			else
				AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_define_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);

			/* Stop ilumination or exposure */
			fbc_command(PRIVATE_DATA->handle, ": E 0 #", NULL, 0);
			fbc_command(PRIVATE_DATA->handle, ": F 0 #", NULL, 0);

			indigo_define_property(device, AUX_LIGHT_SWITCH_PROPERTY, NULL);
			CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
		} else {
			INDIGO_DRIVER_ERROR(DRIVER_NAME, "Failed to connect to %s", DEVICE_PORT_ITEM->text.value);
			CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
		}
	} else {
		indigo_delete_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
		indigo_delete_property(device, AUX_LIGHT_SWITCH_PROPERTY, NULL);
		// turn off fbc at disconnect
		//fbc_command(PRIVATE_DATA->handle, "E:0", response, sizeof(response));
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
		sprintf(command, ": B %d #", (int)AUX_LIGHT_INTENSITY_ITEM->number.value);
		if (fbc_command(PRIVATE_DATA->handle, command, response, sizeof(response)))
			AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_OK_STATE;
		else
			AUX_LIGHT_INTENSITY_PROPERTY->state = INDIGO_ALERT_STATE;
	}
	indigo_update_property(device, AUX_LIGHT_INTENSITY_PROPERTY, NULL);
	pthread_mutex_unlock(&PRIVATE_DATA->mutex);
}

static void aux_switch_handler(indigo_device *device) {
	pthread_mutex_lock(&PRIVATE_DATA->mutex);
	if (AUX_LIGHT_SWITCH_PROPERTY->state != INDIGO_BUSY_STATE) {
		char command[16],	response[16];
		sprintf(command, "E:%d", AUX_LIGHT_SWITCH_ON_ITEM->sw.value);
		if (fbc_command(PRIVATE_DATA->handle, command, response, sizeof(response)))
			AUX_LIGHT_SWITCH_PROPERTY->state = INDIGO_OK_STATE;
		else
			AUX_LIGHT_SWITCH_PROPERTY->state = INDIGO_ALERT_STATE;
	}
	indigo_update_property(device, AUX_LIGHT_SWITCH_PROPERTY, NULL);
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
		// -------------------------------------------------------------------------------- AUX_LIGHT_SWITCH
	} else if (indigo_property_match(AUX_LIGHT_SWITCH_PROPERTY, property)) {
		indigo_property_copy_values(AUX_LIGHT_SWITCH_PROPERTY, property, false);
		if (IS_CONNECTED)
			indigo_set_timer(device, 0, aux_switch_handler);
		return INDIGO_OK;
		// -------------------------------------------------------------------------------- AUX_LIGHT_INTENSITY
	} else if (indigo_property_match(AUX_LIGHT_INTENSITY_PROPERTY, property)) {
		indigo_property_copy_values(AUX_LIGHT_INTENSITY_PROPERTY, property, false);
		if (IS_CONNECTED)
			indigo_set_timer(device, 0, aux_intensity_handler);
		return INDIGO_OK;
		// -------------------------------------------------------------------------------- CONFIG
	} else if (indigo_property_match(CONFIG_PROPERTY, property)) {
		if (indigo_switch_match(CONFIG_SAVE_ITEM, property)) {
			indigo_save_property(device, NULL, AUX_LIGHT_INTENSITY_PROPERTY);
			indigo_save_property(device, NULL, AUX_LIGHT_SWITCH_PROPERTY);
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
	indigo_release_property(AUX_LIGHT_SWITCH_PROPERTY);
	pthread_mutex_destroy(&PRIVATE_DATA->mutex);
	INDIGO_DEVICE_DETACH_LOG(DRIVER_NAME, device->name);
	return indigo_aux_detach(device);
}

// -------------------------------------------------------------------------------- INDIGO driver implementation

indigo_result indigo_aux_fbc(indigo_driver_action action, indigo_driver_info *info) {
	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;
	static fbc_private_data *private_data = NULL;
	static indigo_device *aux = NULL;

	static indigo_device aux_template = INDIGO_DEVICE_INITIALIZER(
		"Lacerta FBC",
		aux_attach,
		aux_enumerate_properties,
		aux_change_property,
		NULL,
		aux_detach
		);

	SET_DRIVER_INFO(info, "Lacerta FBC", __FUNCTION__, DRIVER_VERSION, false, last_action);

	if (action == last_action)
		return INDIGO_OK;

	switch (action) {
		case INDIGO_DRIVER_INIT:
			last_action = action;
			private_data = malloc(sizeof(fbc_private_data));
			assert(private_data != NULL);
			memset(private_data, 0, sizeof(fbc_private_data));
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
