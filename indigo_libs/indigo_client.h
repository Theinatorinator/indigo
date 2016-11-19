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

/** INDIGO client
 \file indigo_client.h
 */

#ifndef indigo_client_h
#define indigo_client_h

#include <pthread.h>

#include "indigo_bus.h"
#include "indigo_driver.h"

#define INDIGO_MAX_DRIVERS	100
#define INDIGO_MAX_SERVERS	10

/** Driver entry type.
 */
typedef struct {
	char description[INDIGO_NAME_SIZE];	///< driver description
	char name[INDIGO_NAME_SIZE];				///< driver name (entry point name)
	driver_entry_point driver;					///< driver entry point
	void *dl_handle;										///< dynamic library handle (NULL for statically linked driver)
} indigo_driver_entry;

/** Remote server entry type.
 */
typedef struct {
	char host[INDIGO_NAME_SIZE];				///< server host name
	int port;														///< server port
	pthread_t thread;										///< client thread
	int socket;													///< stream socket
	indigo_device *protocol_adapter;		///< server protocol adapter
} indigo_server_entry;

/** Array of all available drivers (statically & dynamically linked).
 */
extern indigo_driver_entry indigo_available_drivers[INDIGO_MAX_DRIVERS];

/** Array of all available servers.
 */
extern indigo_server_entry indigo_available_servers[INDIGO_MAX_SERVERS];

/** Add statically linked driver.
 */
extern indigo_result indigo_add_driver(driver_entry_point driver);

/** Remove statically linked driver.
 */
extern indigo_result indigo_remove_driver(driver_entry_point driver);

/** Load & add dynamically linked driver.
 */
extern indigo_result indigo_load_driver(const char *name);

/** Remove & unload dynamically linked driver.
 */
extern indigo_result indigo_unload_driver(const char *name);

/** Connect and start thread for remote server.
 */
extern indigo_result indigo_connect_server(const char *host, int port);

/** Disconnect and stop thread for remote server.
 */
extern indigo_result indigo_disconnect_server(const char *host, int port);

#endif /* indigo_client_h */
