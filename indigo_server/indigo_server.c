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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <assert.h>
#include <signal.h>
#include <dns_sd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#ifdef INDIGO_LINUX
#include <sys/prctl.h>
#endif

#include "indigo_bus.h"
#include "indigo_server_tcp.h"
#include "indigo_driver.h"
#include "indigo_client.h"

#include "ccd_simulator/indigo_ccd_simulator.h"
#include "mount_simulator/indigo_mount_simulator.h"
#ifdef STATIC_DRIVERS
#include "ccd_sx/indigo_ccd_sx.h"
#include "wheel_sx/indigo_wheel_sx.h"
#include "ccd_ssag/indigo_ccd_ssag.h"
#include "ccd_asi/indigo_ccd_asi.h"
#include "wheel_asi/indigo_wheel_asi.h"
#include "ccd_atik/indigo_ccd_atik.h"
#include "wheel_atik/indigo_wheel_atik.h"
#include "ccd_qhy/indigo_ccd_qhy.h"
#include "focuser_fcusb/indigo_focuser_fcusb.h"
#include "ccd_iidc/indigo_ccd_iidc.h"
#include "mount_lx200/indigo_mount_lx200.h"
#include "mount_nexstar/indigo_mount_nexstar.h"
#include "wheel_fli/indigo_wheel_fli.h"
#include "focuser_fli/indigo_focuser_fli.h"
#endif

#define MDNS_INDIGO_TYPE    "_indigo._tcp"
#define MDNS_HTTP_TYPE      "_http._tcp"
#define SERVER_NAME         "INDIGO Server"

driver_entry_point static_drivers[] = {
	indigo_ccd_simulator,
	indigo_mount_simulator,
#ifdef STATIC_DRIVERS
	indigo_ccd_sx,
	indigo_wheel_sx,
	indigo_ccd_atik,
	indigo_wheel_atik,
	indigo_ccd_qhy,
	indigo_ccd_ssag,
	indigo_focuser_fcusb,
	indigo_ccd_asi,
	indigo_wheel_asi,
	indigo_ccd_iidc,
	indigo_mount_lx200,
	indigo_mount_nexstar,
	indigo_wheel_fli,
	indigo_focuser_fli,
#endif
	NULL
};

static int first_driver = 2;
static indigo_property *drivers_property;
static indigo_property *servers_property;
static indigo_property *load_property;
static indigo_property *unload_property;
static indigo_property *restart_property;
static DNSServiceRef sd_http;
static DNSServiceRef sd_indigo;

static pid_t server_pid = 0;
static bool keep_server_running = true;
static bool use_sigkill = false;

static indigo_result attach(indigo_device *device);
static indigo_result enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property);
static indigo_result change_property(indigo_device *device, indigo_client *client, indigo_property *property);
static indigo_result detach(indigo_device *device);

static indigo_device server_device = {
	"Server", NULL, INDIGO_OK, INDIGO_VERSION_CURRENT,
	attach,
	enumerate_properties,
	change_property,
	detach
};

static unsigned char ctrl[] = {
#include "ctrl.data"
};

static void server_callback(int count) {
	INDIGO_LOG(indigo_log("%d clients", count));
}

static indigo_result attach(indigo_device *device) {
	assert(device != NULL);
	drivers_property = indigo_init_switch_property(NULL, server_device.name, "DRIVERS", "Main", "Active drivers", INDIGO_IDLE_STATE, INDIGO_RW_PERM, INDIGO_ANY_OF_MANY_RULE, INDIGO_MAX_DRIVERS);
	drivers_property->count = 0;
	for (int i = 0; i < INDIGO_MAX_DRIVERS; i++)
		if (indigo_available_drivers[i].driver != NULL)
			indigo_init_switch_item(&drivers_property->items[drivers_property->count++], indigo_available_drivers[i].description, indigo_available_drivers[i].description, true);
	servers_property = indigo_init_light_property(NULL, server_device.name, "SERVERS", "Main", "Active servers", INDIGO_IDLE_STATE, 2 * INDIGO_MAX_SERVERS);
	servers_property->count = 0;
	for (int i = 0; i < INDIGO_MAX_SERVERS; i++) {
		indigo_server_entry *entry = indigo_available_servers + i;
		if (*entry->host) {
			char buf[INDIGO_NAME_SIZE];
			if (entry->port == 7624)
				strncpy(buf, entry->host, sizeof(buf));
			else
				snprintf(buf, sizeof(buf), "%s:%d", entry->host, entry->port);
			indigo_init_light_item(&servers_property->items[servers_property->count++], buf, buf, INDIGO_IDLE_STATE);
		}
	}
	for (int i = 0; i < INDIGO_MAX_SERVERS; i++) {
		indigo_subprocess_entry *entry = indigo_available_subprocesses + i;
		if (*entry->executable) {
			indigo_init_light_item(&servers_property->items[servers_property->count++], entry->executable, entry->executable, INDIGO_IDLE_STATE);
		}
	}
	load_property = indigo_init_text_property(NULL, server_device.name, "LOAD", "Main", "Load driver", INDIGO_IDLE_STATE, INDIGO_RW_PERM, 1);
	indigo_init_text_item(&load_property->items[0], "DRIVER", "Load driver", "");
	unload_property = indigo_init_text_property(NULL, server_device.name, "UNLOAD", "Main", "Unload driver", INDIGO_IDLE_STATE, INDIGO_RW_PERM, 1);
	indigo_init_text_item(&unload_property->items[0], "DRIVER", "Unload driver", "");
	restart_property = indigo_init_switch_property(NULL, server_device.name, "RESTART", "Main", "Restart", INDIGO_IDLE_STATE, INDIGO_RW_PERM, INDIGO_ANY_OF_MANY_RULE, 1);
	indigo_init_switch_item(restart_property->items, "RESTART", "Restart server", false);
	if (indigo_load_properties(device, false) == INDIGO_FAILED)
		change_property(device, NULL, drivers_property);
	INDIGO_LOG(indigo_log("%s attached", device->name));
	return INDIGO_OK;
}

static indigo_result enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	indigo_define_property(device, drivers_property, NULL);
	if (servers_property->count > 0)
		indigo_define_property(device, servers_property, NULL);
	indigo_define_property(device, load_property, NULL);
	indigo_define_property(device, unload_property, NULL);
	indigo_define_property(device, restart_property, NULL);
	return INDIGO_OK;
}

static indigo_result change_property(indigo_device *device, indigo_client *client, indigo_property *property) {
	assert(device != NULL);
	assert(property != NULL);
	if (indigo_property_match(drivers_property, property)) {
	// -------------------------------------------------------------------------------- DRIVERS
		indigo_property_copy_values(drivers_property, property, false);
		for (int i = 0; i < drivers_property->count; i++)
			if (drivers_property->items[i].sw.value)
				indigo_available_drivers[i].driver(INDIGO_DRIVER_INIT, NULL);
			else
				indigo_available_drivers[i].driver(INDIGO_DRIVER_SHUTDOWN, NULL);
		drivers_property->state = INDIGO_OK_STATE;
		indigo_update_property(device, drivers_property, NULL);
		int handle = 0;
		indigo_save_property(device, &handle, drivers_property);
		close(handle);
	} else if (indigo_property_match(load_property, property)) {
		// -------------------------------------------------------------------------------- LOAD
		indigo_property_copy_values(load_property, property, false);
		if (*load_property->items[0].text.value) {
			if (indigo_load_driver(load_property->items[0].text.value, true) == INDIGO_OK) {
				indigo_delete_property(device, drivers_property, NULL);
				drivers_property->count = 0;
				for (int i = 0; i < INDIGO_MAX_DRIVERS; i++)
					if (indigo_available_drivers[i].driver != NULL)
						indigo_init_switch_item(&drivers_property->items[drivers_property->count++], indigo_available_drivers[i].description, indigo_available_drivers[i].description, true);
				indigo_define_property(device, drivers_property, NULL);
				load_property->state = INDIGO_OK_STATE;
				indigo_update_property(device, load_property, "Driver %s loaded", load_property->items[0].text.value);
			} else {
				load_property->state = INDIGO_ALERT_STATE;
				indigo_update_property(device, load_property, indigo_last_message);
			}
		}
	} else if (indigo_property_match(unload_property, property)) {
		// -------------------------------------------------------------------------------- UNLOAD
		indigo_property_copy_values(unload_property, property, false);
		if (*unload_property->items[0].text.value) {
			if (indigo_unload_driver(unload_property->items[0].text.value) == INDIGO_OK) {
				indigo_delete_property(device, drivers_property, NULL);
				drivers_property->count = 0;
				for (int i = 0; i < INDIGO_MAX_DRIVERS; i++)
					if (indigo_available_drivers[i].driver != NULL)
						indigo_init_switch_item(&drivers_property->items[drivers_property->count++], indigo_available_drivers[i].description, indigo_available_drivers[i].description, true);
				indigo_define_property(device, drivers_property, NULL);
				load_property->state = INDIGO_OK_STATE;
				indigo_update_property(device, unload_property, "Driver %s unloaded", unload_property->items[0].text.value);
			} else {
				load_property->state = INDIGO_ALERT_STATE;
				indigo_update_property(device, unload_property, indigo_last_message);
			}
		}
	} else if (indigo_property_match(restart_property, property)) {
	// -------------------------------------------------------------------------------- RESTART
		indigo_property_copy_values(restart_property, property, false);
		if (restart_property->items[0].sw.value) {
			INDIGO_LOG(indigo_log("Restarting..."));
			indigo_server_shutdown();
		}
	// --------------------------------------------------------------------------------
	}
	return INDIGO_OK;
}

static indigo_result detach(indigo_device *device) {
	assert(device != NULL);
	indigo_delete_property(device, drivers_property, NULL);
	if (servers_property->count > 0)
		indigo_delete_property(device, servers_property, NULL);
	indigo_delete_property(device, load_property, NULL);
	indigo_delete_property(device, unload_property, NULL);
	INDIGO_LOG(indigo_log("%s detached", device->name));
	return INDIGO_OK;
}

static void server_main(int argc, const char * argv[]) {
	indigo_log("INDIGO server %d.%d-%d built on %s", (INDIGO_VERSION_CURRENT >> 8) & 0xFF, INDIGO_VERSION_CURRENT & 0xFF, INDIGO_BUILD, __TIMESTAMP__);

	indigo_start_usb_event_handler();
	
	indigo_start();

	bool use_bonjour = true;
	bool use_control_panel = true;
	
	for (int i = 1; i < argc; i++) {
		if ((!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) && i < argc - 1) {
			indigo_server_tcp_port = atoi(argv[i + 1]);
			i++;
		} else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--enable-simulators")) {
			first_driver = 0;
		} else if ((!strcmp(argv[i], "-r") || !strcmp(argv[i], "--remote-server")) && i < argc - 1) {
			char host[INDIGO_NAME_SIZE];
			strncpy(host, argv[i + 1], INDIGO_NAME_SIZE);
			char *colon = strchr(host, ':');
			int port = 7624;
			if (colon != NULL) {
				*colon++ = 0;
				port = atoi(colon);
			}
			indigo_connect_server(host, port);
			i++;
		} else if ((!strcmp(argv[i], "-i") || !strcmp(argv[i], "--indi-driver")) && i < argc - 1) {
			char executable[INDIGO_NAME_SIZE];
			strncpy(executable, argv[i + 1], INDIGO_NAME_SIZE);
			indigo_start_subprocess(executable);
			i++;
		} else if (!strcmp(argv[i], "-vv") || !strcmp(argv[i], "--enable-trace")) {
			indigo_trace_level = true;
		} else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--enable-debug")) {
			indigo_debug_level = true;
		} else if (!strcmp(argv[i], "-vv") || !strcmp(argv[i], "--enable-trace")) {
			indigo_trace_level = true;
		} else if (!strcmp(argv[i], "-b-") || !strcmp(argv[i], "--disable-bonjour")) {
			use_bonjour = false;
		} else if (!strcmp(argv[i], "-c-") || !strcmp(argv[i], "--disable-control-panel")) {
			use_control_panel = false;
		} else if(argv[i][0] != '-') {
			indigo_load_driver(argv[i], false);
		}
	}
	
	if (use_control_panel)
		indigo_server_add_resource("/ctrl", ctrl, sizeof(ctrl), "text/html");

	if (use_bonjour) {
		/* UGLY but the only way to suppress compat mode warning messages on Linux */
		setenv("AVAHI_COMPAT_NOWARN", "1", 1);
		char hostname[INDIGO_NAME_SIZE], servicename[INDIGO_NAME_SIZE];
		gethostname(hostname, sizeof(hostname));
		snprintf(servicename, INDIGO_NAME_SIZE, "%s (%d)", hostname, indigo_server_tcp_port);
		DNSServiceRegister(&sd_http, 0, 0, servicename, MDNS_HTTP_TYPE, NULL, NULL, htons(indigo_server_tcp_port), 0, NULL, NULL, NULL);
		DNSServiceRegister(&sd_indigo, 0, 0, servicename, MDNS_INDIGO_TYPE, NULL, NULL, htons(indigo_server_tcp_port), 0, NULL, NULL, NULL);
	}
	for (int i = first_driver; static_drivers[i]; i++) {
		indigo_add_driver(static_drivers[i], false);
	}

	indigo_attach_device(&server_device);	
	indigo_server_start(server_callback);

#ifdef INDIGO_MACOS
	DNSServiceRefDeallocate(sd_indigo);
	DNSServiceRefDeallocate(sd_http);
#endif
	
	for (int i = 0; i < INDIGO_MAX_DRIVERS; i++) {
		if (indigo_available_drivers[i].driver) {
			if (indigo_available_drivers[i].dl_handle != NULL)
				indigo_unload_driver(indigo_available_drivers[i].name);
			else
				indigo_remove_driver(indigo_available_drivers[i].driver);
		}
	}
	for (int i = 0; i < INDIGO_MAX_SERVERS; i++) {
		if (indigo_available_servers[i].thread_started)
			indigo_disconnect_server(indigo_available_servers[i].host, indigo_available_servers[i].port);
	}
	for (int i = 0; i < INDIGO_MAX_SERVERS; i++) {
		if (indigo_available_subprocesses[i].thread_started)
			indigo_kill_subprocess(indigo_available_subprocesses[i].executable);
	}
	indigo_detach_device(&server_device);
	indigo_stop();
}

static void signal_handler(int signo) {
	if (server_pid == 0) {
		/* SIGINT is delivered twise with CTRL-C
		   this leads to freeze during shutdown so
		   we ignore the second SIGINT */
		if (signo == SIGINT) {
			signal(SIGINT, SIG_IGN);
		}
		INDIGO_LOG(indigo_log("Shutdown initiated (signal %d)...", signo));
		indigo_server_shutdown();
	} else {
		INDIGO_LOG(indigo_log("Signal %d received...", signo));
		keep_server_running = (signo == SIGHUP);
		if (use_sigkill)
			kill(server_pid, SIGKILL);
		else
			kill(server_pid, SIGINT);

		use_sigkill = true;
	}
}

int main(int argc, const char * argv[]) {
	char const *server_argv[argc];
	int server_argc = 1;
	bool do_fork = true;
	server_argv[0] = argv[0];
	indigo_main_argc = argc;
	indigo_main_argv = argv;
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--") || !strcmp(argv[i], "--do-not-fork")) {
			do_fork = false;
		} else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--use-syslog")) {
			indigo_use_syslog = true;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			printf("%s [-h|--help]\n", argv[0]);
			printf("%s [--|--do-not-fork] [-l|--use-syslog] [-s|--enable-simulators] [-p|--port port] [-b-|--disable-bonjour] [-c-|--disable-control-panel] [-v|--enable-debug] [-vv|--enable-trace] [-r|--remote-server host:port] [-i|--indi-driver driver_executable] indigo_driver_name indigo_driver_name ...\n", argv[0]);
			return 0;
		} else {
			server_argv[server_argc++] = argv[i];
		}
	}
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGHUP, signal_handler);
	if (do_fork) {
		while(keep_server_running) {
			server_pid = fork();
			if (server_pid == -1) {
				INDIGO_LOG(indigo_log("Server start failed!"));
				return EXIT_FAILURE;
			} else if (server_pid == 0) {
#ifdef INDIGO_LINUX
				prctl(PR_SET_PDEATHSIG, SIGINT, 0, 0, 0);
#endif
				server_main(server_argc, server_argv);
				return EXIT_SUCCESS;
			} else {
				if (waitpid(server_pid, NULL, 0) == -1 ) {
					INDIGO_LOG(indigo_log("waitpid() failed."));
					return EXIT_FAILURE;
				}
				use_sigkill = false;
				if (keep_server_running) {
					INDIGO_LOG(indigo_log("Shutdown complete! Starting up..."));
					sleep(2);
				}
			}
		}
		INDIGO_LOG(indigo_log("Shutdown complete! See you!"));
	} else {
		server_main(server_argc, server_argv);
	}
}
