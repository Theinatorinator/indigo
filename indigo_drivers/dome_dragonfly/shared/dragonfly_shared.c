// Copyright (C) 2020 Rumen G. Bogdanovski
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
// 2.0 by Rumen G. Bogdanovski

/** INDIGO Lunatico Dragonfly driver
 \file dragonfly_shared.c
 */

#define DOME_DRAGONFLY_NAME    "Dome Dragonfly"
#define AUX_DRAGONFLY_NAME     "Dragonfly Controller"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <sys/time.h>

#if defined(INDIGO_FREEBSD)
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#include <indigo/indigo_driver_xml.h>

#include <indigo/indigo_io.h>
#include <indigo/indigo_client.h>
#include <indigo/indigo_dome_driver.h>
#include <indigo/indigo_aux_driver.h>

#define DEFAULT_BAUDRATE            "115200"

#define ROTATOR_SPEED 1

#define MAX_LOGICAL_DEVICES  2
#define MAX_PHYSICAL_DEVICES 4

#define DEVICE_CONNECTED_MASK            0x80
#define L_DEVICE_INDEX_MASK              0x0F

#define DEVICE_CONNECTED                 (device->gp_bits & DEVICE_CONNECTED_MASK)

#define set_connected_flag(dev)          ((dev)->gp_bits |= DEVICE_CONNECTED_MASK)
#define clear_connected_flag(dev)        ((dev)->gp_bits &= ~DEVICE_CONNECTED_MASK)

#define get_locical_device_index(dev)              ((dev)->gp_bits & L_DEVICE_INDEX_MASK)
#define set_logical_device_index(dev, index)       ((dev)->gp_bits = ((dev)->gp_bits & ~L_DEVICE_INDEX_MASK) | (L_DEVICE_INDEX_MASK & index))

#define device_exists(p_device_index, l_device_index) (device_data[p_device_index].device[l_device_index] != NULL)

#define PRIVATE_DATA                      ((lunatico_private_data *)device->private_data)
#define DEVICE_DATA                       (PRIVATE_DATA->device_data[get_locical_device_index(device)])

#define AUX_RELAYS_GROUP	"Relay control"

#define AUX_OUTLET_NAMES_PROPERTY      (DEVICE_DATA.outlet_names_property)
#define AUX_OUTLET_NAME_1_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 0)
#define AUX_OUTLET_NAME_2_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 1)
#define AUX_OUTLET_NAME_3_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 2)
#define AUX_OUTLET_NAME_4_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 3)
#define AUX_OUTLET_NAME_5_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 4)
#define AUX_OUTLET_NAME_6_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 5)
#define AUX_OUTLET_NAME_7_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 6)
#define AUX_OUTLET_NAME_8_ITEM         (AUX_OUTLET_NAMES_PROPERTY->items + 7)

#define AUX_GPIO_OUTLET_PROPERTY      (DEVICE_DATA.gpio_outlet_property)
#define AUX_GPIO_OUTLET_1_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 0)
#define AUX_GPIO_OUTLET_2_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 1)
#define AUX_GPIO_OUTLET_3_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 2)
#define AUX_GPIO_OUTLET_4_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 3)
#define AUX_GPIO_OUTLET_5_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 4)
#define AUX_GPIO_OUTLET_6_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 5)
#define AUX_GPIO_OUTLET_7_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 6)
#define AUX_GPIO_OUTLET_8_ITEM        (AUX_GPIO_OUTLET_PROPERTY->items + 7)

#define AUX_OUTLET_PULSE_LENGTHS_PROPERTY      (DEVICE_DATA.gpio_outlet_pulse_property)
#define AUX_OUTLET_PULSE_LENGTHS_1_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 0)
#define AUX_OUTLET_PULSE_LENGTHS_2_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 1)
#define AUX_OUTLET_PULSE_LENGTHS_3_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 2)
#define AUX_OUTLET_PULSE_LENGTHS_4_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 3)
#define AUX_OUTLET_PULSE_LENGTHS_5_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 4)
#define AUX_OUTLET_PULSE_LENGTHS_6_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 5)
#define AUX_OUTLET_PULSE_LENGTHS_7_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 6)
#define AUX_OUTLET_PULSE_LENGTHS_8_ITEM        (AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + 7)

#define AUX_SENSORS_GROUP	"Sensors"

#define AUX_SENSOR_NAMES_PROPERTY      (DEVICE_DATA.sensor_names_property)
#define AUX_SENSOR_NAME_1_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 0)
#define AUX_SENSOR_NAME_2_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 1)
#define AUX_SENSOR_NAME_3_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 2)
#define AUX_SENSOR_NAME_4_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 3)
#define AUX_SENSOR_NAME_5_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 4)
#define AUX_SENSOR_NAME_6_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 5)
#define AUX_SENSOR_NAME_7_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 6)
#define AUX_SENSOR_NAME_8_ITEM         (AUX_SENSOR_NAMES_PROPERTY->items + 7)

#define AUX_GPIO_SENSORS_PROPERTY     (DEVICE_DATA.sensors_property)
#define AUX_GPIO_SENSOR_1_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 0)
#define AUX_GPIO_SENSOR_2_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 1)
#define AUX_GPIO_SENSOR_3_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 2)
#define AUX_GPIO_SENSOR_4_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 3)
#define AUX_GPIO_SENSOR_5_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 4)
#define AUX_GPIO_SENSOR_6_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 5)
#define AUX_GPIO_SENSOR_7_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 6)
#define AUX_GPIO_SENSOR_8_ITEM        (AUX_GPIO_SENSORS_PROPERTY->items + 7)

typedef enum {
	TYPE_DOME    = 0,
	TYPE_AUX     = 1
} device_type_t;

typedef struct {
	device_type_t device_type;
	double prev_temp;
	bool relay_active[8];
	indigo_timer *relay_timers[8];
	indigo_timer *temperature_timer;
	indigo_timer *sensors_timer;
	indigo_property *outlet_names_property,
	                *gpio_outlet_property,
	                *gpio_outlet_pulse_property,
	                *sensor_names_property,
	                *sensors_property;
} logical_device_data;

typedef struct {
	int handle;
	int count_open;
	int port_count;
	bool udp;
	pthread_mutex_t port_mutex;
	logical_device_data device_data[MAX_LOGICAL_DEVICES];
} lunatico_private_data;

typedef struct {
	indigo_device *device[MAX_LOGICAL_DEVICES];
	lunatico_private_data *private_data;
} lunatico_device_data;

static void compensate_focus(indigo_device *device, double new_temp);

static lunatico_device_data device_data[MAX_PHYSICAL_DEVICES] = {0};

static void create_port_device(int p_device_index, int l_device_index, device_type_t type);
static void delete_port_device(int p_device_index, int l_device_index);

/* Linatico Astronomia device Commands ======================================================================== */

#define LUNATICO_CMD_LEN 100

typedef enum {
	MODEL_SELETEK = 1,
	MODEL_ARMADILLO = 2,
	MODEL_PLATYPUS = 3,
	MODEL_DRAGONFLY = 4,
	MODEL_LIMPET = 5
} lunatico_model_t;

#define NO_TEMP_READING                (-25)


static bool lunatico_command(indigo_device *device, const char *command, char *response, int max, int sleep) {
	char c;
	char buff[LUNATICO_CMD_LEN];
	struct timeval tv;
	pthread_mutex_lock(&PRIVATE_DATA->port_mutex);
	// flush
	while (true) {
		fd_set readout;
		FD_ZERO(&readout);
		FD_SET(PRIVATE_DATA->handle, &readout);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		long result = select(PRIVATE_DATA->handle+1, &readout, NULL, NULL, &tv);
		if (result == 0)
			break;
		if (result < 0) {
			pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
			return false;
		}
		if (PRIVATE_DATA->udp) {
			result = read(PRIVATE_DATA->handle, buff, LUNATICO_CMD_LEN);
			if (result < 1) {
				pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
				return false;
			}
			break;
		} else {
			result = read(PRIVATE_DATA->handle, &c, 1);
			if (result < 1) {
				pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
				return false;
			}
		}
	}

	// write command
	indigo_write(PRIVATE_DATA->handle, command, strlen(command));
	if (sleep > 0)
		usleep(sleep);

	// read responce
	if (response != NULL) {
		int index = 0;
		int timeout = 3;
		while (index < max) {
			fd_set readout;
			FD_ZERO(&readout);
			FD_SET(PRIVATE_DATA->handle, &readout);
			tv.tv_sec = timeout;
			tv.tv_usec = 100000;
			timeout = 0;
			long result = select(PRIVATE_DATA->handle+1, &readout, NULL, NULL, &tv);
			if (result <= 0)
				break;
			if (PRIVATE_DATA->udp) {
				result = read(PRIVATE_DATA->handle, response, LUNATICO_CMD_LEN);
				if (result < 1) {
					pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
					INDIGO_DRIVER_ERROR(DRIVER_NAME, "Failed to read from %s -> %s (%d)", DEVICE_PORT_ITEM->text.value, strerror(errno), errno);
					return false;
				}
				index = result;
				break;
			} else {
				result = read(PRIVATE_DATA->handle, &c, 1);
				if (result < 1) {
					pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
					INDIGO_DRIVER_ERROR(DRIVER_NAME, "Failed to read from %s -> %s (%d)", DEVICE_PORT_ITEM->text.value, strerror(errno), errno);
					return false;
				}
				response[index++] = c;
				if (c == '#') break;
			}
		}
		response[index] = '\0';
	}
	pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "Command %s -> %s", command, response != NULL ? response : "NULL");
	return true;
}


static bool lunatico_get_info(indigo_device *device, char *board, char *firmware) {
	if(!board || !firmware) return false;

	const char *operative[3] = { "", "Bootloader", "Error" };
	const char *models[6] = { "Error", "Seletek", "Armadillo", "Platypus", "Dragonfly", "Limpet" };
	int fwmaj, fwmin, model, oper, data;
	char response[LUNATICO_CMD_LEN]={0};
	if (lunatico_command(device, "!seletek version#", response, sizeof(response), 0)) {
		// !seletek version:2510#
		int parsed = sscanf(response, "!seletek version:%d#", &data);
		if (parsed != 1) return false;
		oper = data / 10000;		// 0 normal, 1 bootloader
		model = (data / 1000) % 10;	// 1 seletek, etc.
		fwmaj = (data / 100) % 10;
		fwmin = (data % 100);
		if (oper >= 2) oper = 2;
		if (model > 5) model = 0;
		sprintf(board, "%s", models[model]);
		sprintf(firmware, "%d.%d", fwmaj, fwmin);

		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "!seletek version# -> %s = %s %s", response, board, firmware);
		return true;
	}
	INDIGO_DRIVER_ERROR(DRIVER_NAME, "NO response");
	return false;
}


static bool lunatico_command_get_result(indigo_device *device, const char *command, int32_t *result) {
	if (!result) return false;

	char response[LUNATICO_CMD_LEN]={0};
	char response_prefix[LUNATICO_CMD_LEN];
	char format[LUNATICO_CMD_LEN];

	if (lunatico_command(device, command, response, sizeof(response), 0)) {
		strncpy(response_prefix, command, LUNATICO_CMD_LEN);
		char *p = strrchr(response_prefix, '#');
		if (p) *p = ':';
		sprintf(format, "%s%%d#", response_prefix);
		int parsed = sscanf(response, format, result);
		if (parsed != 1) return false;
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "%s -> %s = %d", command, response, *result);
		return true;
	}
	INDIGO_DRIVER_ERROR(DRIVER_NAME, "NO response");
	return false;
}

static bool lunatico_read_sensor(indigo_device *device, int pin, int *sensor_value) {
	return true;
}

static bool lunatico_analog_read_sensor(indigo_device *device, int sensor, int *sensor_value) {
	if (!sensor_value) return false;

	char command[LUNATICO_CMD_LEN];
	int value;

	if (sensor < 0 || sensor > 8) return false;

	snprintf(command, LUNATICO_CMD_LEN, "!relio snanrd 0 %d#", sensor);
	if (!lunatico_command_get_result(device, command, &value)) return false;
	if (value >= 0) {
		*sensor_value = value;
		return true;
	}
	return false;
}

static bool lunatico_analog_read_sensors(indigo_device *device, int *sensors) {
	char response[LUNATICO_CMD_LEN]={0};
	char format[LUNATICO_CMD_LEN];
	int isensors[8];

	if (lunatico_command(device, "!relio snanrd 0 0 7#", response, sizeof(response), 0)) {
		sprintf(format, "!relio snanrd 0 0 7:%%d,%%d,%%d,%%d,%%d,%%d,%%d,%%d#");
		int parsed = sscanf(response, format, isensors, isensors+1, isensors+2, isensors+3, isensors+4, isensors+5, isensors+6, isensors+7);
		if (parsed != 8) return false;
		sensors[0] = isensors[0];
		sensors[1] = isensors[1];
		sensors[2] = isensors[2];
		sensors[3] = isensors[3];
		sensors[4] = isensors[4];
		sensors[5] = isensors[5];
		sensors[6] = isensors[6];
		sensors[7] = isensors[7];
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "-> %s = %d %d %d %d %d %d %d %d", response, sensors[0], sensors[1], sensors[2], sensors[3], sensors[4], sensors[5], sensors[6], sensors[7]);
		return true;
	}
	INDIGO_DRIVER_ERROR(DRIVER_NAME, "NO response");
	return false;
}

static bool lunatico_digital_read_sensor(indigo_device *device, int sensor, bool *sensor_value) {
	if (!sensor_value) return false;

	char command[LUNATICO_CMD_LEN];
	int value;

	if (sensor < 0 || sensor > 8) return false;

	snprintf(command, LUNATICO_CMD_LEN, "!relio sndgrd 0 %d#", sensor);
	if (!lunatico_command_get_result(device, command, &value)) return false;
	if (value >= 0) {
		*sensor_value = (bool)value;
		return true;
	}
	return false;
}


static bool lunatico_read_relay(indigo_device *device, int relay, bool *enabled) {
	if (!enabled) return false;

	char command[LUNATICO_CMD_LEN];
	int value;

	if (relay < 0 || relay > 8) return false;

	snprintf(command, LUNATICO_CMD_LEN, "!relio rldgrd 0 %d#", relay);
	if (!lunatico_command_get_result(device, command, &value)) return false;
	if (value >= 0) {
		*enabled = (bool)value;
		return true;
	}
	return false;
}


static bool lunatico_read_relays(indigo_device *device, bool *relays) {
	char response[LUNATICO_CMD_LEN]={0};
	char format[LUNATICO_CMD_LEN];
	int irelays[8];

	if (lunatico_command(device, "!relio rldgrd 0 0 7#", response, sizeof(response), 0)) {
		sprintf(format, "!relio rldgrd 0 0 7:%%d,%%d,%%d,%%d,%%d,%%d,%%d,%%d#");
		int parsed = sscanf(response, format, irelays, irelays+1, irelays+2, irelays+3, irelays+4, irelays+5, irelays+6, irelays+7);
		if (parsed != 8) return false;
		relays[0] = (bool)irelays[0];
		relays[1] = (bool)irelays[1];
		relays[2] = (bool)irelays[2];
		relays[3] = (bool)irelays[3];
		relays[4] = (bool)irelays[4];
		relays[5] = (bool)irelays[5];
		relays[6] = (bool)irelays[6];
		relays[7] = (bool)irelays[7];
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "-> %s = %d %d %d %d %d %d %d %d", response, relays[0], relays[1], relays[2], relays[3], relays[4], relays[5], relays[6], relays[7]);
		return true;
	}
	INDIGO_DRIVER_ERROR(DRIVER_NAME, "NO response");
	return false;
}


static bool lunatico_set_relay(indigo_device *device, int relay, bool enable) {
	char command[LUNATICO_CMD_LEN];
	int res;
	if (relay < 0 || relay > 8) return false;

	snprintf(command, LUNATICO_CMD_LEN, "!relio rlset 0 %d %d#", relay, enable ? 1 : 0);
	if (!lunatico_command_get_result(device, command, &res)) return false;
	if (res < 0) return false;
	return true;
}


static bool lunatico_pulse_relay(indigo_device *device, int relay, uint32_t lenms) {
	char command[LUNATICO_CMD_LEN];
	int res;
	if (relay < 0 || relay > 8) return false;

	snprintf(command, LUNATICO_CMD_LEN, "!relio rlpulse 0 %d %d#", relay, lenms);
	if (!lunatico_command_get_result(device, command, &res)) return false;
	if (res < 0) return false;
	return true;
}


// --------------------------------------------------------------------------------- Common stuff

static bool lunatico_open(indigo_device *device) {
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "OPEN REQUESTED: %d -> %d, count_open = %d", PRIVATE_DATA->handle, DEVICE_CONNECTED, PRIVATE_DATA->count_open);
	if (DEVICE_CONNECTED) return false;

	pthread_mutex_lock(&PRIVATE_DATA->port_mutex);
	if (PRIVATE_DATA->count_open++ == 0) {
		if (indigo_try_global_lock(device) != INDIGO_OK) {
			PRIVATE_DATA->count_open--;
			pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
			INDIGO_DRIVER_ERROR(DRIVER_NAME, "indigo_try_global_lock(): failed to get lock.");
			return false;
		}

		char url[INDIGO_VALUE_SIZE];
		if (strstr(DEVICE_PORT_ITEM->text.value, "://")) {
			strncpy(url, DEVICE_PORT_ITEM->text.value, INDIGO_VALUE_SIZE);
		} else {
			snprintf(url, INDIGO_VALUE_SIZE, "udp://%s", DEVICE_PORT_ITEM->text.value);
		}
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "Opening network device on host: %s", DEVICE_PORT_ITEM->text.value);
		indigo_network_protocol proto = INDIGO_PROTOCOL_UDP;
		PRIVATE_DATA->handle = indigo_open_network_device(url, 10000, &proto);
		PRIVATE_DATA->udp = true;

		if (PRIVATE_DATA->handle < 0) {
			INDIGO_DRIVER_ERROR(DRIVER_NAME, "Opening device %s: failed", DEVICE_PORT_ITEM->text.value);
			CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
			indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
			indigo_update_property(device, CONNECTION_PROPERTY, NULL);
			indigo_global_unlock(device);
			PRIVATE_DATA->count_open--;
			pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
			return false;
		}
	}
	set_connected_flag(device);
	pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
	return true;
}


static void lunatico_close(indigo_device *device) {
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "CLOSE REQUESTED: %d -> %d, count_open = %d", PRIVATE_DATA->handle, DEVICE_CONNECTED, PRIVATE_DATA->count_open);
	if (!DEVICE_CONNECTED) return;

	pthread_mutex_lock(&PRIVATE_DATA->port_mutex);
	if (--PRIVATE_DATA->count_open == 0) {
		close(PRIVATE_DATA->handle);
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "close(%d)", PRIVATE_DATA->handle);
		indigo_global_unlock(device);
		PRIVATE_DATA->handle = 0;
	}
	clear_connected_flag(device);
	pthread_mutex_unlock(&PRIVATE_DATA->port_mutex);
}


static int lunatico_init_properties(indigo_device *device) {
	// -------------------------------------------------------------------------- LA_MODEL_PROPERTY
	// -------------------------------------------------------------------------------- SIMULATION
	SIMULATION_PROPERTY->hidden = true;
	// -------------------------------------------------------------------------------- DEVICE_PORT
	DEVICE_PORT_PROPERTY->hidden = false;
	// --------------------------------------------------------------------------------
	INFO_PROPERTY->count = 5;
	// -------------------------------------------------------------------------------- OUTLET_NAMES
	AUX_OUTLET_NAMES_PROPERTY = indigo_init_text_property(NULL, device->name, AUX_OUTLET_NAMES_PROPERTY_NAME, AUX_RELAYS_GROUP, "Relay names", INDIGO_OK_STATE, INDIGO_RW_PERM, 8);
	if (AUX_OUTLET_NAMES_PROPERTY == NULL)
		return INDIGO_FAILED;
	indigo_init_text_item(AUX_OUTLET_NAME_1_ITEM, AUX_GPIO_OUTLET_NAME_1_ITEM_NAME, "Relay 1", "Relay #1");
	indigo_init_text_item(AUX_OUTLET_NAME_2_ITEM, AUX_GPIO_OUTLET_NAME_2_ITEM_NAME, "Relay 2", "Relay #2");
	indigo_init_text_item(AUX_OUTLET_NAME_3_ITEM, AUX_GPIO_OUTLET_NAME_3_ITEM_NAME, "Relay 3", "Relay #3");
	indigo_init_text_item(AUX_OUTLET_NAME_4_ITEM, AUX_GPIO_OUTLET_NAME_4_ITEM_NAME, "Relay 4", "Relay #4");
	indigo_init_text_item(AUX_OUTLET_NAME_5_ITEM, AUX_GPIO_OUTLET_NAME_5_ITEM_NAME, "Relay 5", "Relay #5");
	indigo_init_text_item(AUX_OUTLET_NAME_6_ITEM, AUX_GPIO_OUTLET_NAME_6_ITEM_NAME, "Relay 6", "Relay #6");
	indigo_init_text_item(AUX_OUTLET_NAME_7_ITEM, AUX_GPIO_OUTLET_NAME_7_ITEM_NAME, "Relay 7", "Relay #7");
	indigo_init_text_item(AUX_OUTLET_NAME_8_ITEM, AUX_GPIO_OUTLET_NAME_8_ITEM_NAME, "Relay 8", "Relay #8");
	if (DEVICE_DATA.device_type != TYPE_AUX) AUX_OUTLET_NAMES_PROPERTY->hidden = true;
	// -------------------------------------------------------------------------------- GPIO OUTLETS
	AUX_GPIO_OUTLET_PROPERTY = indigo_init_switch_property(NULL, device->name, AUX_GPIO_OUTLETS_PROPERTY_NAME, AUX_RELAYS_GROUP, "Relay outlets", INDIGO_OK_STATE, INDIGO_RW_PERM, INDIGO_ANY_OF_MANY_RULE, 8);
	if (AUX_GPIO_OUTLET_PROPERTY == NULL)
		return INDIGO_FAILED;
	indigo_init_switch_item(AUX_GPIO_OUTLET_1_ITEM, AUX_GPIO_OUTLETS_OUTLET_1_ITEM_NAME, "Relay #1", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_2_ITEM, AUX_GPIO_OUTLETS_OUTLET_2_ITEM_NAME, "Relay #2", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_3_ITEM, AUX_GPIO_OUTLETS_OUTLET_3_ITEM_NAME, "Relay #3", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_4_ITEM, AUX_GPIO_OUTLETS_OUTLET_4_ITEM_NAME, "Relay #4", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_5_ITEM, AUX_GPIO_OUTLETS_OUTLET_5_ITEM_NAME, "Relay #5", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_6_ITEM, AUX_GPIO_OUTLETS_OUTLET_6_ITEM_NAME, "Relay #6", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_7_ITEM, AUX_GPIO_OUTLETS_OUTLET_7_ITEM_NAME, "Relay #7", false);
	indigo_init_switch_item(AUX_GPIO_OUTLET_8_ITEM, AUX_GPIO_OUTLETS_OUTLET_8_ITEM_NAME, "Relay #8", false);
	if (DEVICE_DATA.device_type != TYPE_AUX) AUX_GPIO_OUTLET_PROPERTY->hidden = true;
	// -------------------------------------------------------------------------------- GPIO PULSE OUTLETS
	AUX_OUTLET_PULSE_LENGTHS_PROPERTY = indigo_init_number_property(NULL, device->name, "AUX_OUTLET_PULSE_LENGTHS", AUX_RELAYS_GROUP, "Relay pulse lengths(ms)", INDIGO_OK_STATE, INDIGO_RW_PERM, 8);
	if (AUX_OUTLET_PULSE_LENGTHS_PROPERTY == NULL)
		return INDIGO_FAILED;
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_1_ITEM, AUX_GPIO_OUTLETS_OUTLET_1_ITEM_NAME, "Relay #1", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_2_ITEM, AUX_GPIO_OUTLETS_OUTLET_2_ITEM_NAME, "Relay #2", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_3_ITEM, AUX_GPIO_OUTLETS_OUTLET_3_ITEM_NAME, "Relay #3", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_4_ITEM, AUX_GPIO_OUTLETS_OUTLET_4_ITEM_NAME, "Relay #4", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_5_ITEM, AUX_GPIO_OUTLETS_OUTLET_5_ITEM_NAME, "Relay #5", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_6_ITEM, AUX_GPIO_OUTLETS_OUTLET_6_ITEM_NAME, "Relay #6", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_7_ITEM, AUX_GPIO_OUTLETS_OUTLET_7_ITEM_NAME, "Relay #7", 0, 100000, 100, 0);
	indigo_init_number_item(AUX_OUTLET_PULSE_LENGTHS_8_ITEM, AUX_GPIO_OUTLETS_OUTLET_8_ITEM_NAME, "Relay #8", 0, 100000, 100, 0);
	if (DEVICE_DATA.device_type != TYPE_AUX) AUX_OUTLET_PULSE_LENGTHS_PROPERTY->hidden = true;
	// -------------------------------------------------------------------------------- SENSOR_NAMES
	AUX_SENSOR_NAMES_PROPERTY = indigo_init_text_property(NULL, device->name, AUX_SENSOR_NAMES_PROPERTY_NAME, AUX_SENSORS_GROUP, "Sensor names", INDIGO_OK_STATE, INDIGO_RW_PERM, 8);
	if (AUX_SENSOR_NAMES_PROPERTY == NULL)
		return INDIGO_FAILED;
	indigo_init_text_item(AUX_SENSOR_NAME_1_ITEM, AUX_GPIO_SENSOR_NAME_1_ITEM_NAME, "Sensor 1", "Sensor #1");
	indigo_init_text_item(AUX_SENSOR_NAME_2_ITEM, AUX_GPIO_SENSOR_NAME_2_ITEM_NAME, "Sensor 2", "Sensor #2");
	indigo_init_text_item(AUX_SENSOR_NAME_3_ITEM, AUX_GPIO_SENSOR_NAME_3_ITEM_NAME, "Sensor 3", "Sensor #3");
	indigo_init_text_item(AUX_SENSOR_NAME_4_ITEM, AUX_GPIO_SENSOR_NAME_4_ITEM_NAME, "Sensor 4", "Sensor #4");
	indigo_init_text_item(AUX_SENSOR_NAME_5_ITEM, AUX_GPIO_SENSOR_NAME_5_ITEM_NAME, "Sensor 5", "Sensor #5");
	indigo_init_text_item(AUX_SENSOR_NAME_6_ITEM, AUX_GPIO_SENSOR_NAME_6_ITEM_NAME, "Sensor 6", "Sensor #6");
	indigo_init_text_item(AUX_SENSOR_NAME_7_ITEM, AUX_GPIO_SENSOR_NAME_7_ITEM_NAME, "Sensor 7", "Sensor #7");
	indigo_init_text_item(AUX_SENSOR_NAME_8_ITEM, AUX_GPIO_SENSOR_NAME_8_ITEM_NAME, "Sensor 8", "Sensor #8");
	if (DEVICE_DATA.device_type != TYPE_AUX) AUX_SENSOR_NAMES_PROPERTY->hidden = true;
	// -------------------------------------------------------------------------------- GPIO_SENSORS
	AUX_GPIO_SENSORS_PROPERTY = indigo_init_number_property(NULL, device->name, AUX_GPIO_SENSORS_PROPERTY_NAME, AUX_SENSORS_GROUP, "Sensors", INDIGO_OK_STATE, INDIGO_RO_PERM, 8);
	if (AUX_GPIO_SENSORS_PROPERTY == NULL)
		return INDIGO_FAILED;
	indigo_init_number_item(AUX_GPIO_SENSOR_1_ITEM, AUX_GPIO_SENSOR_NAME_1_ITEM_NAME, "Sensor #1", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_2_ITEM, AUX_GPIO_SENSOR_NAME_2_ITEM_NAME, "Sensor #2", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_3_ITEM, AUX_GPIO_SENSOR_NAME_3_ITEM_NAME, "Sensor #3", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_4_ITEM, AUX_GPIO_SENSOR_NAME_4_ITEM_NAME, "Sensor #4", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_5_ITEM, AUX_GPIO_SENSOR_NAME_5_ITEM_NAME, "Sensor #5", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_6_ITEM, AUX_GPIO_SENSOR_NAME_6_ITEM_NAME, "Sensor #6", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_7_ITEM, AUX_GPIO_SENSOR_NAME_7_ITEM_NAME, "Sensor #7", 0, 1024, 1, 0);
	indigo_init_number_item(AUX_GPIO_SENSOR_8_ITEM, AUX_GPIO_SENSOR_NAME_8_ITEM_NAME, "Sensor #8", 0, 1024, 1, 0);
	if (DEVICE_DATA.device_type != TYPE_AUX) AUX_GPIO_SENSORS_PROPERTY->hidden = true;
	//---------------------------------------------------------------------------
	indigo_define_property(device, AUX_OUTLET_NAMES_PROPERTY, NULL);
	indigo_define_property(device, AUX_SENSOR_NAMES_PROPERTY, NULL);
	return INDIGO_OK;
}


static indigo_result lunatico_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property) {
	if (IS_CONNECTED) {
		if (indigo_property_match(AUX_GPIO_OUTLET_PROPERTY, property))
			indigo_define_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
		if (indigo_property_match(AUX_OUTLET_PULSE_LENGTHS_PROPERTY, property))
			indigo_define_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
		if (indigo_property_match(AUX_GPIO_SENSORS_PROPERTY, property))
			indigo_define_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
	}
	if (indigo_property_match(AUX_OUTLET_NAMES_PROPERTY, property))
		indigo_define_property(device, AUX_OUTLET_NAMES_PROPERTY, NULL);
	if (indigo_property_match(AUX_SENSOR_NAMES_PROPERTY, property))
		indigo_define_property(device, AUX_SENSOR_NAMES_PROPERTY, NULL);
	return INDIGO_OK;
}


static void lunatico_init_device(indigo_device *device) {
	char board[LUNATICO_CMD_LEN] = "N/A";
	char firmware[LUNATICO_CMD_LEN] = "N/A";
	uint32_t value;
	if (lunatico_get_info(device, board, firmware)) {
		strncpy(INFO_DEVICE_MODEL_ITEM->text.value, board, INDIGO_VALUE_SIZE);
		strncpy(INFO_DEVICE_FW_REVISION_ITEM->text.value, firmware, INDIGO_VALUE_SIZE);
		indigo_update_property(device, INFO_PROPERTY, NULL);
	}
}


static indigo_result lunatico_detach(indigo_device *device) {
	assert(device != NULL);
	if (CONNECTION_CONNECTED_ITEM->sw.value)
		indigo_device_disconnect(NULL, device->name);
	lunatico_close(device);
	indigo_device_disconnect(NULL, device->name);
	indigo_release_property(AUX_GPIO_OUTLET_PROPERTY);
	indigo_release_property(AUX_OUTLET_PULSE_LENGTHS_PROPERTY);
	indigo_release_property(AUX_GPIO_SENSORS_PROPERTY);
	INDIGO_DEVICE_DETACH_LOG(DRIVER_NAME, device->name);

	indigo_delete_property(device, AUX_OUTLET_NAMES_PROPERTY, NULL);
	indigo_release_property(AUX_OUTLET_NAMES_PROPERTY);

	indigo_delete_property(device, AUX_SENSOR_NAMES_PROPERTY, NULL);
	indigo_release_property(AUX_SENSOR_NAMES_PROPERTY);
	return INDIGO_OK;
}


static void lunatico_save_properties(indigo_device *device) {
	indigo_save_property(device, NULL, AUX_OUTLET_NAMES_PROPERTY);
	indigo_save_property(device, NULL, AUX_SENSOR_NAMES_PROPERTY);
}


static void lunatico_delete_properties(indigo_device *device) {
	indigo_delete_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	indigo_delete_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
	indigo_delete_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
}


static indigo_result lunatico_common_update_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	if (indigo_property_match(CONFIG_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- CONFIG
		if (indigo_switch_match(CONFIG_SAVE_ITEM, property)) {
			lunatico_save_properties(device);
		}
	}
	return INDIGO_OK;
}

// --------------------------------------------------------------------------------- INDIGO AUX RELAYS device implementation

static void sensors_timer_callback(indigo_device *device) {
	int sensor_value;
	bool success;
	int sensors[8];

	if (!lunatico_analog_read_sensors(device, sensors)) {
		AUX_GPIO_SENSORS_PROPERTY->state = INDIGO_ALERT_STATE;
	} else {
		for (int i = 0; i < 8; i++) {
			(AUX_GPIO_SENSORS_PROPERTY->items + i)->number.value = (double)sensors[i];
		}
		AUX_GPIO_SENSORS_PROPERTY->state = INDIGO_OK_STATE;
	}
	indigo_update_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
	indigo_reschedule_timer(device, 1, &DEVICE_DATA.sensors_timer);
}

static pthread_mutex_t pulse_mutex = PTHREAD_MUTEX_INITIALIZER;

static void relay_1_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[0] = false;
	AUX_GPIO_OUTLET_1_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_2_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[1] = false;
	AUX_GPIO_OUTLET_2_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_3_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[2] = false;
	AUX_GPIO_OUTLET_3_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_4_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[3] = false;
	AUX_GPIO_OUTLET_4_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_5_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[4] = false;
	AUX_GPIO_OUTLET_5_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_6_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[5] = false;
	AUX_GPIO_OUTLET_6_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_7_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[6] = false;
	AUX_GPIO_OUTLET_7_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}

static void relay_8_timer_callback(indigo_device *device) {
	pthread_mutex_lock(&pulse_mutex);
	DEVICE_DATA.relay_active[7] = false;
	AUX_GPIO_OUTLET_8_ITEM->sw.value = false;
	indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
	pthread_mutex_unlock(&pulse_mutex);
}


static void (*relay_timer_callbacks[])(indigo_device*) = {
	relay_1_timer_callback,
	relay_2_timer_callback,
	relay_3_timer_callback,
	relay_4_timer_callback,
	relay_5_timer_callback,
	relay_6_timer_callback,
	relay_7_timer_callback,
	relay_8_timer_callback
};


static bool set_gpio_outlets(indigo_device *device) {
	bool success = true;
	bool relay_value[8];

	if (!lunatico_read_relays(device, relay_value)) {
		INDIGO_DRIVER_ERROR(DRIVER_NAME, "lunatico_read_relays(%d) failed", PRIVATE_DATA->handle);
		return false;
	}

	for (int i = 0; i < 8; i++) {
		if ((AUX_GPIO_OUTLET_PROPERTY->items + i)->sw.value != relay_value[i]) {
			if (((AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + i)->number.value > 0) && (AUX_GPIO_OUTLET_PROPERTY->items + i)->sw.value && !DEVICE_DATA.relay_active[i]) {
				if (!lunatico_pulse_relay(device, i, (int)(AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + i)->number.value)) {
					INDIGO_DRIVER_ERROR(DRIVER_NAME, "lunatico_pulse_relay(%d) failed", PRIVATE_DATA->handle);
					success = false;
				} else {
					DEVICE_DATA.relay_active[i] = true;
					DEVICE_DATA.relay_timers[i] = indigo_set_timer(device, ((AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + i)->number.value+20)/1000.0, relay_timer_callbacks[i]);
				}
			} else if ((AUX_OUTLET_PULSE_LENGTHS_PROPERTY->items + i)->number.value == 0 || (!(AUX_GPIO_OUTLET_PROPERTY->items + i)->sw.value && !DEVICE_DATA.relay_active[i])) {
				if (!lunatico_set_relay(device, i, (AUX_GPIO_OUTLET_PROPERTY->items + i)->sw.value)) {
					INDIGO_DRIVER_ERROR(DRIVER_NAME, "lunatico_set_relay(%d) failed", PRIVATE_DATA->handle);
					success = false;
				}
			}
		}
	}

	return success;
}


static indigo_result aux_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property) {
	lunatico_enumerate_properties(device, client, property);
	return indigo_aux_enumerate_properties(device, NULL, NULL);
}


static indigo_result aux_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_aux_attach(device, DRIVER_VERSION, INDIGO_INTERFACE_AUX_GPIO) == INDIGO_OK) {
		// --------------------------------------------------------------------------------
		if (lunatico_init_properties(device) != INDIGO_OK) return INDIGO_FAILED;
		INDIGO_DEVICE_ATTACH_LOG(DRIVER_NAME, device->name);
		return indigo_aux_enumerate_properties(device, NULL, NULL);
	}
	return INDIGO_FAILED;
}


static indigo_result aux_change_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	assert(DEVICE_CONTEXT != NULL);
	assert(property != NULL);
	if (indigo_property_match(CONNECTION_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- CONNECTION
		indigo_property_copy_values(CONNECTION_PROPERTY, property, false);
		if (CONNECTION_CONNECTED_ITEM->sw.value) {
			if (!DEVICE_CONNECTED) {
				CONNECTION_PROPERTY->state = INDIGO_BUSY_STATE;
				indigo_update_property(device, CONNECTION_PROPERTY, NULL);
				if (lunatico_open(device)) {
					char board[LUNATICO_CMD_LEN] = "N/A";
					char firmware[LUNATICO_CMD_LEN] = "N/A";
					if (lunatico_get_info(device, board, firmware)) {
						strncpy(INFO_DEVICE_MODEL_ITEM->text.value, board, INDIGO_VALUE_SIZE);
						strncpy(INFO_DEVICE_FW_REVISION_ITEM->text.value, firmware, INDIGO_VALUE_SIZE);
						indigo_update_property(device, INFO_PROPERTY, NULL);
					} else {
						CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
						indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, false);
						lunatico_close(device);
					}
					indigo_define_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
					indigo_define_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
					indigo_define_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
					DEVICE_DATA.sensors_timer = indigo_set_timer(device, 0, sensors_timer_callback);
					CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
				}
			}
		} else {
			if (DEVICE_CONNECTED) {
				indigo_cancel_timer(device, &DEVICE_DATA.sensors_timer);
				lunatico_delete_properties(device);
				lunatico_close(device);
				CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
			}
		}
	} else if (indigo_property_match(AUX_OUTLET_NAMES_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- X_AUX_OUTLET_NAMES
		indigo_property_copy_values(AUX_OUTLET_NAMES_PROPERTY, property, false);
		if (IS_CONNECTED) {
			indigo_delete_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
			indigo_delete_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
		}
		snprintf(AUX_GPIO_OUTLET_1_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_1_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_2_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_2_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_3_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_3_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_4_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_4_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_5_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_5_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_6_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_6_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_7_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_7_ITEM->text.value);
		snprintf(AUX_GPIO_OUTLET_8_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_8_ITEM->text.value);

		snprintf(AUX_OUTLET_PULSE_LENGTHS_1_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_1_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_2_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_2_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_3_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_3_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_4_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_4_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_5_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_5_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_6_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_6_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_7_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_7_ITEM->text.value);
		snprintf(AUX_OUTLET_PULSE_LENGTHS_8_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_OUTLET_NAME_8_ITEM->text.value);

		AUX_OUTLET_NAMES_PROPERTY->state = INDIGO_OK_STATE;
		if (IS_CONNECTED) {
			indigo_define_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
			indigo_define_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
		}
		indigo_update_property(device, AUX_OUTLET_NAMES_PROPERTY, NULL);
		return INDIGO_OK;
	} else if (indigo_property_match(AUX_GPIO_OUTLET_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- AUX_GPIO_OUTLET
		indigo_property_copy_values(AUX_GPIO_OUTLET_PROPERTY, property, false);
		if (!IS_CONNECTED) return INDIGO_OK;

		if (set_gpio_outlets(device) == true) {
			AUX_GPIO_OUTLET_PROPERTY->state = INDIGO_OK_STATE;
		} else {
			AUX_GPIO_OUTLET_PROPERTY->state = INDIGO_ALERT_STATE;
		}
		indigo_update_property(device, AUX_GPIO_OUTLET_PROPERTY, NULL);
		return INDIGO_OK;
	} else if (indigo_property_match(AUX_OUTLET_PULSE_LENGTHS_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- AUX_OUTLET_PULSE_LENGTHS
		indigo_property_copy_values(AUX_OUTLET_PULSE_LENGTHS_PROPERTY, property, false);
		if (!IS_CONNECTED) return INDIGO_OK;
		indigo_update_property(device, AUX_OUTLET_PULSE_LENGTHS_PROPERTY, NULL);
		return INDIGO_OK;
	} else if (indigo_property_match(AUX_SENSOR_NAMES_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- AUX_SENSOR_NAMES
		indigo_property_copy_values(AUX_SENSOR_NAMES_PROPERTY, property, false);
		if (IS_CONNECTED) {
			indigo_delete_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
		}
		snprintf(AUX_GPIO_SENSOR_1_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_1_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_2_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_2_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_3_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_3_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_4_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_4_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_5_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_5_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_6_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_6_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_7_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_7_ITEM->text.value);
		snprintf(AUX_GPIO_SENSOR_8_ITEM->label, INDIGO_NAME_SIZE, "%s", AUX_SENSOR_NAME_8_ITEM->text.value);
		AUX_SENSOR_NAMES_PROPERTY->state = INDIGO_OK_STATE;
		if (IS_CONNECTED) {
			indigo_define_property(device, AUX_GPIO_SENSORS_PROPERTY, NULL);
		}
		indigo_update_property(device, AUX_SENSOR_NAMES_PROPERTY, NULL);
		return INDIGO_OK;
	}
	// --------------------------------------------------------------------------------
	lunatico_common_update_property(device, client, property);
	return indigo_aux_change_property(device, client, property);
}


static indigo_result aux_detach(indigo_device *device) {
	lunatico_detach(device);
	return indigo_aux_detach(device);
}

// -------------------------------------------------------------------------------- INDIGO dome device implementation

static void dome_timer_callback(indigo_device *device) {

}

static indigo_result dome_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_dome_attach(device, DRIVER_VERSION) == INDIGO_OK) {
		DEVICE_PORT_PROPERTY->hidden = false;
		// This is a sliding roof -> we need only open and close
		DOME_SPEED_PROPERTY->hidden = true;
		DOME_DIRECTION_PROPERTY->hidden = true;
		DOME_HORIZONTAL_COORDINATES_PROPERTY->hidden = true;
		DOME_EQUATORIAL_COORDINATES_PROPERTY->hidden = true;
		DOME_DIRECTION_PROPERTY->hidden = true;
		DOME_STEPS_PROPERTY->hidden = true;
		DOME_PARK_PROPERTY->hidden = true;
		DOME_DIMENSION_PROPERTY->hidden = true;
		DOME_SLAVING_PROPERTY->hidden = true;
		DOME_SLAVING_PARAMETERS_PROPERTY->hidden = true;

		// --------------------------------------------------------------------------------
		INDIGO_DEVICE_ATTACH_LOG(DRIVER_NAME, device->name);
		return indigo_dome_enumerate_properties(device, NULL, NULL);
	}
	return INDIGO_FAILED;
}

static indigo_result dome_change_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	assert(DEVICE_CONTEXT != NULL);
	assert(property != NULL);
	if (indigo_property_match(CONNECTION_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- CONNECTION
		indigo_property_copy_values(CONNECTION_PROPERTY, property, false);
		if (CONNECTION_CONNECTED_ITEM->sw.value) {
			if (!DEVICE_CONNECTED) {
				CONNECTION_PROPERTY->state = INDIGO_BUSY_STATE;
				indigo_update_property(device, CONNECTION_PROPERTY, NULL);
				if (lunatico_open(device)) {
					char board[LUNATICO_CMD_LEN] = "N/A";
					char firmware[LUNATICO_CMD_LEN] = "N/A";
					if (lunatico_get_info(device, board, firmware)) {
						strncpy(INFO_DEVICE_MODEL_ITEM->text.value, board, INDIGO_VALUE_SIZE);
						strncpy(INFO_DEVICE_FW_REVISION_ITEM->text.value, firmware, INDIGO_VALUE_SIZE);
						indigo_update_property(device, INFO_PROPERTY, NULL);
						CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
					} else {
						CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
						indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, false);
						lunatico_close(device);
					}
				}
			}
		} else {
			if (DEVICE_CONNECTED) {
				indigo_cancel_timer(device, &DEVICE_DATA.sensors_timer);
				lunatico_delete_properties(device);
				lunatico_close(device);
				CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
			}
		}
	} else if (indigo_property_match(DOME_ABORT_MOTION_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- DOME_ABORT_MOTION
		indigo_property_copy_values(DOME_ABORT_MOTION_PROPERTY, property, false);
		DOME_ABORT_MOTION_PROPERTY->state = INDIGO_OK_STATE;
		DOME_ABORT_MOTION_ITEM->sw.value = false;
		indigo_update_property(device, DOME_ABORT_MOTION_PROPERTY, NULL);
		return INDIGO_OK;
	} else if (indigo_property_match(DOME_SHUTTER_PROPERTY, property)) {
		// -------------------------------------------------------------------------------- DOME_SHUTTER
		indigo_property_copy_values(DOME_SHUTTER_PROPERTY, property, false);
		DOME_SHUTTER_PROPERTY->state = INDIGO_OK_STATE;
		indigo_update_property(device, DOME_SHUTTER_PROPERTY, NULL);
		return INDIGO_OK;
		// --------------------------------------------------------------------------------
	}
	return indigo_dome_change_property(device, client, property);
}

static indigo_result dome_detach(indigo_device *device) {
	assert(device != NULL);
	if (CONNECTION_CONNECTED_ITEM->sw.value)
		indigo_device_disconnect(NULL, device->name);
	INDIGO_DEVICE_DETACH_LOG(DRIVER_NAME, device->name);
	return indigo_dome_detach(device);
}


// --------------------------------------------------------------------------------

static int device_number = 0;

static void create_port_device(int p_device_index, int l_device_index, device_type_t device_type) {

	static indigo_device aux_template = INDIGO_DEVICE_INITIALIZER(
		AUX_DRAGONFLY_NAME,
		aux_attach,
		aux_enumerate_properties,
		aux_change_property,
		NULL,
		aux_detach
	);

	static indigo_device dome_template = INDIGO_DEVICE_INITIALIZER(
		DOME_DRAGONFLY_NAME,
		dome_attach,
		indigo_dome_enumerate_properties,
		dome_change_property,
		NULL,
		dome_detach
	);

	if (l_device_index >= MAX_LOGICAL_DEVICES) return;
	if (p_device_index >= MAX_PHYSICAL_DEVICES) return;
	if (device_data[p_device_index].device[l_device_index] != NULL) {
		if ((device_data[p_device_index].private_data) && (device_data[p_device_index].private_data->device_data[l_device_index].device_type == device_type)) {
				return;
		} else {
				delete_port_device(p_device_index, l_device_index);
		}
	}

	if (device_data[p_device_index].private_data == NULL) {
		device_data[p_device_index].private_data = malloc(sizeof(lunatico_private_data));
		assert(device_data[p_device_index].private_data != NULL);
		memset(device_data[p_device_index].private_data, 0, sizeof(lunatico_private_data));
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "ADD: PRIVATE_DATA");
	}

	device_data[p_device_index].device[l_device_index] = malloc(sizeof(indigo_device));
	assert(device_data[p_device_index].device[l_device_index] != NULL);
	if (device_type == TYPE_DOME) {
		memcpy(device_data[p_device_index].device[l_device_index], &dome_template, sizeof(indigo_device));
		sprintf(device_data[p_device_index].device[l_device_index]->name, "%s", DOME_DRAGONFLY_NAME);
		device_data[p_device_index].private_data->device_data[l_device_index].device_type = TYPE_DOME;
	} else {
		memcpy(device_data[p_device_index].device[l_device_index], &aux_template, sizeof(indigo_device));
		sprintf(device_data[p_device_index].device[l_device_index]->name, "%s", AUX_DRAGONFLY_NAME);
		device_data[p_device_index].private_data->device_data[l_device_index].device_type = TYPE_AUX;
	}
	set_logical_device_index(device_data[p_device_index].device[l_device_index], l_device_index);
	device_data[p_device_index].device[l_device_index]->private_data = device_data[p_device_index].private_data;
	indigo_attach_device(device_data[p_device_index].device[l_device_index]);
	INDIGO_DRIVER_DEBUG(DRIVER_NAME, "ADD: Device with port index = %d", get_locical_device_index(device_data[p_device_index].device[l_device_index]));
}


static void delete_port_device(int p_device_index, int l_device_index) {
	if (l_device_index >= MAX_LOGICAL_DEVICES) return;
	if (p_device_index >= MAX_PHYSICAL_DEVICES) return;

	if (device_data[p_device_index].device[l_device_index] != NULL) {
		indigo_detach_device(device_data[p_device_index].device[l_device_index]);
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "REMOVE: Locical Device with index = %d", get_locical_device_index(device_data[p_device_index].device[l_device_index]));
		free(device_data[p_device_index].device[l_device_index]);
		device_data[p_device_index].device[l_device_index] = NULL;
	}

	for (int i = 0; i < MAX_LOGICAL_DEVICES; i++) {
		if (device_data[p_device_index].device[i] != NULL) return;
	}

	if (device_data[p_device_index].private_data != NULL) {
		free(device_data[p_device_index].private_data);
		device_data[p_device_index].private_data = NULL;
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "REMOVE: PRIVATE_DATA");
	}
}


indigo_result DRIVER_ENTRY_POINT(indigo_driver_action action, indigo_driver_info *info) {

	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;

	SET_DRIVER_INFO(info, DRIVER_INFO, __FUNCTION__, DRIVER_VERSION, false, last_action);

	if (action == last_action)
		return INDIGO_OK;

	switch (action) {
	case INDIGO_DRIVER_INIT:
		last_action = action;
		if (indigo_driver_initialized(CONFLICTING_DRIVER)) {
			INDIGO_DRIVER_LOG(DRIVER_NAME, "Conflicting driver %s is already loaded", CONFLICTING_DRIVER);
			last_action = INDIGO_DRIVER_SHUTDOWN;
			return INDIGO_FAILED;
		}
		create_port_device(0, 0, DEFAULT_DEVICE);
		create_port_device(0, 1, TYPE_AUX);
		break;

	case INDIGO_DRIVER_SHUTDOWN:
		last_action = action;
		for (int index = 0; index < MAX_LOGICAL_DEVICES; index++) {
			delete_port_device(0, index);
		}
		break;

	case INDIGO_DRIVER_INFO:
		break;
	}

	return INDIGO_OK;
}
