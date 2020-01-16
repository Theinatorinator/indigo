# Basics of INDIGO Driver Development
Revision: 16.01.2020 (early draft)

Author: **Rumen G.Bogdanovski**

e-mail: *rumen@skyarchive.org*

## INDIGO Concepts

INDIGO is a platform for the communication between software entities over a software **bus**. These entities are typically either in a **device** or a **client** role, but there are special entities often referred as **agents** which are in both **device** and **client** role. A piece of code registering one or more **devices** on the **bus** is referred as the **driver**.

INDIGO is asynchronous in its nature, so to be able to communicate over the **bus**, the client and device have to register a structure containing pointers to **callback functions** which are called by the INDIGO **bus** upon specific events.

The communication between different entities attached to the **bus** is done by INDIGO **messages** which contain **property** events. Properties on the other hand contain one or more **items** of a specified type. One can think of the properties as both set of variables and routines. Set of variables as they may store values like the *width* and *height* of the CCD frame or routines like *start 1 second exposure*. Messages sent over the INDIGO **bus** are abstraction of INDI messages.

The messages sent from the **device** to a **client** may contain one of the following events:
- **definition** of a property - lets the client know that the property can be used, the message contains the property definition: name, type, list of items etc...
- **update** of property item values - lets the client know that certain property items have changed their values.
- **deletion** of a property - lets the client know that the property is removed and can not be used any more.

The messages sent from a **client** to the **device** can be:
- **request for definition** of available properties - client can request the definitions of one or all available properties associated with the device.
- **request for change** of property item values - client can request change of the values of one or several property items.

There are three classes of properties: mandatory, optional, and device specific. Each device class has a set of mandatory properties that should always be defined, a set of optional properties that may or may not be defined and a set of **device** specific properties which may not exist if there is no need. The list of the well known properties is available in [PROPERTIES.md](https://github.com/indigo-astronomy/indigo/blob/master/indigo_docs/PROPERTIES.md).

Different instances of the INDIGO **bus** can be connected in a hierarchical structure, but from a **driver** or a **client** point of view it is fully transparent.

INDIGO uses XML or JSON transport protocols for message passing. The description of the INDIGO protocols used for communication is available in [PROTOCOLS.md](https://github.com/indigo-astronomy/indigo/blob/master/indigo_docs/PROTOCOLS.md).

## Basics of INDIGO API

A basic common API shared by both **driver** and **client** roles is defined in [indigo_bus.h](https://github.com/indigo-astronomy/indigo/blob/master/indigo_libs/indigo/indigo_bus.h). The most important structures are:
- *indigo_item* - definition of property item container
- *indigo_property* - definition of property container
- *indigo_device* - definition of a logical device made available by *driver* containing both driver private data and pointers to callback functions
- *indigo_client* - definition of a client containing both client private data and pointers to callback functions

The **bus** instance should be initialized and started by *indigo_start()* call and stopped by *indigo_stop()* call.
A **device** should be attached to the **bus** by calling *indigo_attach_device()* and detached with *indigo_detach_device()* call.

There are four functions that cam be used by the **device** to send messages to the **client**:
- *indigo_define_property()* - defines a new property
- *indigo_update_property()* - one or more item values of the property are changed
- *indigo_delete_property()* - property is not needed any more and shall not be used
- *indigo_send_message()*  - broadcast a human readable text message

For structure definitions and function prototypes please refer to [indigo_bus.h](https://github.com/indigo-astronomy/indigo/blob/master/indigo_libs/indigo/indigo_bus.h) and [indigo_driver.h](https://github.com/indigo-astronomy/indigo/blob/master/indigo_libs/indigo/indigo_driver.h).

## Types of INDIGO drivers

There are three types of INDIGO drivers. The build system of INDIGO automatically produces the three of them:
- **dynamic driver** - driver that can be loaded and unloaded at any given time
- **static driver** - this driver can be statically linked in the application it can not be unloaded or loaded however they can be enabled and disabled on the fly.
- **executable driver** - these are standard executable programs that communicate over pipes (*stdin* and *stdout*), **legacy** INDI style drivers that can be loaded on startup and they are supported only for INDI compatibility.

The **dynamic drivers** and the **static drivers** are orders of magnitude more effective, as they use shared memory for communication compared to **executable drivers** which use pipes. Therefore they should always be preferred.

In INDIGO the drivers can be used remotely using the INDIGO server or the **clients** can load them locally which makes the communication even faster sacrificing the network capability. However this is not exactly correct as the **client** can also act as a **server** for the locally connected devices and some remote **client** can also use these devices.

## Anatomy of the INDIGO driver
Like every software program the INDIGO drivers have and entry point. As **executable drivers** are standalone programs their entry point is *int main()*, but this is not the case with **dynamic drivers** and **static drivers**. They need to have a function called with the name of the driver for example if the driver is a CCD driver by convention the driver name should start with **indigo_ccd_** flowed by the model or the vendor name or abbreviation for example **indigo_ccd_atik** - the driver for Atik cameras has an entry point *indigo_ccd_atik()* or **indigo_ccd_asi** - the driver for ZWO ASI cameras has an entry point *indigo_ccd_asi()*. The prototype of the driver entry point is:

```C
indigo_result indigo_XXX_YYY(indigo_driver_action action, indigo_driver_info *info);
```
It accepts 2 parameters *action* which can be *INDIGO_DRIVER_INIT*, *INDIGO_DRIVER_SHUTDOWN* or *INDIGO_DRIVER_INFO*. The framework calls this function with those parameters at driver loading, unloading and when only driver info is requested. it is a good idea to populate *indigo_driver_info* sructure at every call. Here is an example code how this function may look like for the Atik filter wheel driver(**indigo_wheel_atik**):

```C
indigo_result indigo_wheel_atik(indigo_driver_action action, indigo_driver_info *info) {
	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;

	SET_DRIVER_INFO(info, "Atik Filter Wheel", __FUNCTION__, DRIVER_VERSION, false, last_action);

	if (action == last_action)
		return INDIGO_OK;

	switch (action) {
	case INDIGO_DRIVER_INIT:
		last_action = action;

		/* Initialize the exported devices here with indigo_attach_device(),
		   if the driver does not support hot-plug or register hot-plug
		   callback which will be called when device is connected.
		*/

		break;

	case INDIGO_DRIVER_SHUTDOWN:
		last_action = action;

		/* Detach devices if hot-plug is not supported with indigo_detach_device()
		   or deregister the hot-plug callback in case hot-plug is supported.
		   and release all driver resources.
		*/

		break;

	case INDIGO_DRIVER_INFO:
		/* info is already initialized at the beginning */
		break;
	}

	return INDIGO_OK;
}
```

The main function for the **executable driver** is pretty much a boiler plate code that connects to the INDIGO **bus** and calls the driver entrypoint with *INDIGO_DRIVER_INIT* and when done it calls it again with *INDIGO_DRIVER_SHUTDOWN*:

```C
int main(int argc, const char * argv[]) {
	indigo_main_argc = argc;
	indigo_main_argv = argv;
	indigo_client *protocol_adapter = indigo_xml_device_adapter(0, 1);
	indigo_start();
	indigo_wheel_atik(INDIGO_DRIVER_INIT, NULL);
	indigo_attach_client(protocol_adapter);
	indigo_xml_parse(NULL, protocol_adapter);
	indigo_wheel_atik(INDIGO_DRIVER_SHUTDOWN, NULL);
	indigo_stop();
	return 0;
}
```

**TBD**

## Device and Property Handling

**TBD**

### Properties

Standard property names are defined in [indigo_names.h](https://github.com/indigo-astronomy/indigo/blob/master/indigo_libs/indigo/indigo_names.h)

Properties can be in one of the four states:
- *INDIGO_IDLE_STATE* - the values may not be initialized
- *INDIGO_OK_STATE* - the property item values are valid and it is save to read or set them
- *INDIGO_BUSY_STATE* - the values are not reliable, some operation is in progress (like exposure is in progress)
- *INDIGO_ALERT_STATE* - the values are not reliable, some operation has failed or the values set are not valid

Each property has predefined type which is one of the following:
- *INDIGO_TEXT_VECTOR* - strings of limited width
-	*INDIGO_NUMBER_VECTOR* - floating point numbers with defined min and max values and increment
-	*INDIGO_SWITCH_VECTOR* - logical values representing “on” and “off” state, there are several behavior rules for this type: *INDIGO_ONE_OF_MANY_RULE* (only one switch can be "on" at a time), *INDIGO_AT_MOST_ONE_RULE* (none or one switch can be "on" at a time) and *INDIGO_ANY_OF_MANY_RULE* (independent checkbox group)
-	*INDIGO_LIGHT_VECTOR* - status values with four possible values *INDIGO_IDLE_STATE*, *INDIGO_OK_STATE*, *INDIGO_BUSY_STATE* and *INDIGO_ALERT_STATE*
-	*INDIGO_BLOB_VECTOR* - binary data of any type and any length usually image data

Properties have permissions assigned to them:
- *INDIGO_RO_PERM* - Read only permission, which means that the client can not modify their item values
- *INDIGO_RW_PERM* - Read and write permission, client can read the value and can change it
- *INDIGO_WO_PERM* - Write only permission, client can change it but can not read its value (can be used for passwords)

**TBD**

### Binary Large Objects aka Image Properties

**TBD**

### Devices

**TBD**

## INDIGO driver - Example

Driver header file **indigo_wheel_atik.h**:

```C
/** INDIGO Atik filter wheel driver
 \file indigo_wheel_atik.h
 */

#ifndef wheel_atik_h
#define wheel_atik_h

#include <indigo/indigo_driver.h>
#include <indigo/indigo_wheel_driver.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Register Atik filter wheel hot-plug callback
 */

extern indigo_result indigo_wheel_atik(indigo_driver_action action, indigo_driver_info *info);

#ifdef __cplusplus
}
#endif

#endif /* wheel_atik_h */
```

Main driver source file **indigo_wheel_atik.c**:

```C
/** INDIGO Atik filter wheel driver
 \file indigo_wheel_atik.c
 */

#define DRIVER_VERSION 0x0001
#define DRIVER_NAME "indigo_wheel_atik"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>

#if defined(INDIGO_FREEBSD)
#include <libusb.h>
#else
#include <libusb-1.0/libusb.h>
#endif

#include <libatik.h>

#include <indigo/indigo_driver_xml.h>

#include "indigo_wheel_atik.h"

// ------------------------------------------- ATIK USB interface implementation

#define ATIK_VENDOR_ID                  0x04D8
#define ATIK_PRODUC_ID                  0x003F

#define PRIVATE_DATA        ((atik_private_data *)device->private_data)

typedef struct {
	hid_device *handle;
	int slot_count, current_slot;
} atik_private_data;

// ------------------------------------------- INDIGO Wheel device implementation

static void wheel_timer_callback(indigo_device *device) {
	libatik_wheel_query(PRIVATE_DATA->handle, &PRIVATE_DATA->slot_count, &PRIVATE_DATA->current_slot);
	WHEEL_SLOT_ITEM->number.value = PRIVATE_DATA->current_slot;
	if (WHEEL_SLOT_ITEM->number.value == WHEEL_SLOT_ITEM->number.target) {
		WHEEL_SLOT_PROPERTY->state = INDIGO_OK_STATE;
	} else {
		indigo_set_timer(device, 0.5, wheel_timer_callback);
	}
	indigo_update_property(device, WHEEL_SLOT_PROPERTY, NULL);
}

static indigo_result wheel_attach(indigo_device *device) {
	assert(device != NULL);
	assert(PRIVATE_DATA != NULL);
	if (indigo_wheel_attach(device, DRIVER_VERSION) == INDIGO_OK) {
		INDIGO_DEVICE_ATTACH_LOG(DRIVER_NAME, device->name);
		return indigo_wheel_enumerate_properties(device, NULL, NULL);
	}
	return INDIGO_FAILED;
}

static indigo_result wheel_change_property(indigo_device *device,
	indigo_client *client, indigo_property *property) {

	assert(device != NULL);
	assert(DEVICE_CONTEXT != NULL);
	assert(property != NULL);
	if (indigo_property_match(CONNECTION_PROPERTY, property)) {
		// -------------------------------------------------------------- CONNECTION
		indigo_property_copy_values(CONNECTION_PROPERTY, property, false);
		if (CONNECTION_CONNECTED_ITEM->sw.value) {
			if ((PRIVATE_DATA->handle = hid_open(ATIK_VENDOR_ID, ATIK_PRODUC_ID, NULL)) != NULL) {
				INDIGO_DRIVER_DEBUG(DRIVER_NAME, "hid_open ->  ok");
				while (true) {
					libatik_wheel_query(
						PRIVATE_DATA->handle,
						&PRIVATE_DATA->slot_count,
						&PRIVATE_DATA->current_slot
					);
					if (PRIVATE_DATA->slot_count > 0 && PRIVATE_DATA->slot_count <= 9)
						break;
					  indigo_usleep(ONE_SECOND_DELAY);
				}
				WHEEL_SLOT_ITEM->number.max =
				WHEEL_SLOT_NAME_PROPERTY->count =
				WHEEL_SLOT_OFFSET_PROPERTY->count = PRIVATE_DATA->slot_count;
				WHEEL_SLOT_ITEM->number.value = PRIVATE_DATA->current_slot;
				CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
			} else {
				INDIGO_DRIVER_DEBUG(DRIVER_NAME, "hid_open ->  failed");
				CONNECTION_PROPERTY->state = INDIGO_ALERT_STATE;
				indigo_set_switch(CONNECTION_PROPERTY, CONNECTION_DISCONNECTED_ITEM, true);
			}
		} else {
			hid_close(PRIVATE_DATA->handle);
			CONNECTION_PROPERTY->state = INDIGO_OK_STATE;
		}
	} else if (indigo_property_match(WHEEL_SLOT_PROPERTY, property)) {
		// -------------------------------------------------------------- WHEEL_SLOT
		indigo_property_copy_values(WHEEL_SLOT_PROPERTY, property, false);
		if (WHEEL_SLOT_ITEM->number.value < 1 ||
			WHEEL_SLOT_ITEM->number.value > WHEEL_SLOT_ITEM->number.max) {
			WHEEL_SLOT_PROPERTY->state = INDIGO_ALERT_STATE;
		} else if (WHEEL_SLOT_ITEM->number.value == PRIVATE_DATA->current_slot) {
			WHEEL_SLOT_PROPERTY->state = INDIGO_OK_STATE;
		} else {
			WHEEL_SLOT_PROPERTY->state = INDIGO_BUSY_STATE;
			libatik_wheel_set(PRIVATE_DATA->handle, (int)WHEEL_SLOT_ITEM->number.value);
			indigo_set_timer(device, 0.5, wheel_timer_callback);
		}
		indigo_update_property(device, WHEEL_SLOT_PROPERTY, NULL);
		return INDIGO_OK;
		// -------------------------------------------------------------------------
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

// ------------------------------------------------------------ hot-plug support

static indigo_device *device = NULL;

static int hotplug_callback(libusb_context *ctx, libusb_device *dev,
	libusb_hotplug_event event, void *user_data) {

	static indigo_device wheel_template = INDIGO_DEVICE_INITIALIZER(
		"ATIK Filter Wheel",
		wheel_attach,
		indigo_wheel_enumerate_properties,
		wheel_change_property,
		NULL,
		wheel_detach
	);

	switch (event) {
		case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED: {
			if (device != NULL) {
				return 0;
			}
			device = malloc(sizeof(indigo_device));
			assert(device != NULL);
			memcpy(device, &wheel_template, sizeof(indigo_device));
			atik_private_data *private_data = malloc(sizeof(atik_private_data));
			assert(private_data != NULL);
			memset(private_data, 0, sizeof(atik_private_data));
			device->private_data = private_data;
			indigo_attach_device(device);
			break;
		}
		case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT: {
			if (device == NULL) {
				return 0;
			}
			indigo_detach_device(device);
			free(device->private_data);
			free(device);
			device = NULL;
		}
	}
	return 0;
};

static libusb_hotplug_callback_handle callback_handle;

indigo_result indigo_wheel_atik(indigo_driver_action action, indigo_driver_info *info) {
	atik_log = indigo_debug;
	static indigo_driver_action last_action = INDIGO_DRIVER_SHUTDOWN;

	SET_DRIVER_INFO(info, "Atik Filter Wheel", __FUNCTION__, DRIVER_VERSION, false, last_action);

	if (action == last_action)
		return INDIGO_OK;

	switch (action) {
	case INDIGO_DRIVER_INIT:
		last_action = action;
		device = NULL;
		hid_init();
		indigo_start_usb_event_handler();
		int rc = libusb_hotplug_register_callback(
			NULL,
			LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT,
			LIBUSB_HOTPLUG_ENUMERATE,
			ATIK_VENDOR_ID,
			ATIK_PRODUC_ID,
			LIBUSB_HOTPLUG_MATCH_ANY,
			hotplug_callback,
			NULL,
			&callback_handle
		);
		INDIGO_DRIVER_DEBUG(DRIVER_NAME,
			"libusb_hotplug_register_callback ->  %s", rc < 0 ? libusb_error_name(rc) : "OK");
		return rc >= 0 ? INDIGO_OK : INDIGO_FAILED;

	case INDIGO_DRIVER_SHUTDOWN:
		last_action = action;
		libusb_hotplug_deregister_callback(NULL, callback_handle);
		INDIGO_DRIVER_DEBUG(DRIVER_NAME, "libusb_hotplug_deregister_callback");
		if (device)
			hotplug_callback(NULL, NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, NULL);
		break;

	case INDIGO_DRIVER_INFO:
		break;
	}

	return INDIGO_OK;
}
```

File containg main fuction needed only for the executable driver  **indigo_wheel_atik_main.c**:

```C
/** INDIGO Atik filter wheel driver main
 \file indigo_wheel_atik_main.c
 */

#include <stdio.h>
#include <string.h>

#include <indigo/indigo_driver_xml.h>
#include "indigo_wheel_atik.h"

int main(int argc, const char * argv[]) {
	indigo_main_argc = argc;
	indigo_main_argv = argv;
	indigo_client *protocol_adapter = indigo_xml_device_adapter(0, 1);
	indigo_start();
	indigo_wheel_atik(INDIGO_DRIVER_INIT, NULL);
	indigo_attach_client(protocol_adapter);
	indigo_xml_parse(NULL, protocol_adapter);
	indigo_wheel_atik(INDIGO_DRIVER_SHUTDOWN, NULL);
	indigo_stop();
	return 0;
}
```

## More Driver Examples

An open source examples of driver API usage can be found in INDIGO driver libraries:

1. [INDIGO common drivers](https://github.com/indigo-astronomy/indigo/tree/master/indigo_drivers)
1. [INDIGO Linux specific drivers](https://github.com/indigo-astronomy/indigo/tree/master/indigo_linux_drivers)
1. [INDIGO MacOS specific drivers](https://github.com/indigo-astronomy/indigo/tree/master/indigo_mac_drivers)
1. [INDIGO optional drivers](https://github.com/indigo-astronomy/indigo/tree/master/indigo_optional_drivers)
