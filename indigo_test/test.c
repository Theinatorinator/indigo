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
// 2.0 by Peter Polakovic <peter.polakovic@cloudmakers.eu>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "indigo_bus.h"
#include "indigo_client.h"
#include "ccd_simulator/indigo_ccd_simulator.h"


#define CCD_SIMULATOR "CCD Imager Simulator @ indigosky"

static bool connected = false;

static indigo_result test_attach(indigo_client *client) {
	indigo_log("attached to INDI bus...");
	indigo_enumerate_properties(client, &INDIGO_ALL_PROPERTIES);
	return INDIGO_OK;
}

static indigo_result test_define_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	if (strcmp(property->device, CCD_SIMULATOR))
		return INDIGO_OK;
	if (!strcmp(property->name, CONNECTION_PROPERTY_NAME)) {
		indigo_device_connect(client, CCD_SIMULATOR);
		return INDIGO_OK;
	}
	if (!strcmp(property->name, CCD_IMAGE_PROPERTY_NAME)) {
		indigo_enable_blob(client, property, INDIGO_ENABLE_BLOB_URL);
	}
	return INDIGO_OK;
}

static indigo_result test_update_property(indigo_client *client, indigo_device *device, indigo_property *property, const char *message) {
	if (strcmp(property->device, CCD_SIMULATOR))
		return INDIGO_OK;
	if (!strcmp(property->name, CONNECTION_PROPERTY_NAME) && property->state == INDIGO_OK_STATE) {
		if (indigo_get_switch(property, CONNECTION_CONNECTED_ITEM_NAME)) {
			if (!connected) {
				connected = true;
				indigo_log("connected...");
				static const char * items[] = { CCD_EXPOSURE_ITEM_NAME };
				static double values[] = { 3.0 };
				indigo_change_number_property(client, CCD_SIMULATOR, CCD_EXPOSURE_PROPERTY_NAME, 1, items, values);
			}
		} else {
			if (connected) {
				indigo_log("disconnected...");
				indigo_stop();
				connected = false;
			}
		}
		return INDIGO_OK;
	}
	if (!strcmp(property->name, CCD_EXPOSURE_PROPERTY_NAME)) {
		if (property->state == INDIGO_BUSY_STATE) {
			indigo_log("exposure %gs...", property->items[0].number.value);
		} else if (property->state == INDIGO_OK_STATE) {
			indigo_log("exposure done...");
		}
		return INDIGO_OK;
	}
	if (!strcmp(property->name, CCD_IMAGE_PROPERTY_NAME) && property->state == INDIGO_OK_STATE) {
		indigo_populate_http_blob_item(&property->items[0]);
		indigo_log("image received (%d bytes)...", property->items[0].blob.size);
		indigo_device_disconnect(client, CCD_SIMULATOR);
	}
	return INDIGO_OK;
}

static indigo_result test_detach(indigo_client *client) {
	indigo_log("detached from INDI bus...");
	return INDIGO_OK;
}

static indigo_client test = {
	"Test", false, NULL, INDIGO_OK, INDIGO_VERSION_CURRENT, NULL,
	test_attach,
	test_define_property,
	test_update_property,
	NULL,
	NULL,
	test_detach
};


int main(int argc, const char * argv[]) {
	indigo_main_argc = argc;
	indigo_main_argv = argv;
	indigo_start();
	indigo_set_log_level(INDIGO_LOG_INFO);

	indigo_server_entry *server;
	indigo_connect_server("indigosky", "192.168.1.40", 7624, &server); // Check IP adress!!!
	indigo_attach_client(&test);
	sleep(10);
	indigo_disconnect_server(server);
	indigo_stop();
	return 0;
}

