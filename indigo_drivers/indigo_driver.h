//  Copyright (c) 2016 CloudMakers, s. r. o.
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//  1. Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution.
//
//  3. The name of the author may not be used to endorse or promote
//  products derived from this software without specific prior
//  written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
//  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
//  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
//  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
//  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

//  version history
//  0.0 PoC by Peter Polakovic <peter.polakovic@cloudmakers.eu>

#ifndef indigo_device_h
#define indigo_device_h

//#define INDIGO_LINUX
//#undef INDIGO_DARWIN

#if defined(INDIGO_LINUX)
#include <pthread.h>
#elif defined(INDIGO_DARWIN)
#include <dispatch/dispatch.h>
#endif

#include "indigo_bus.h"

#define DEVICE_CONTEXT                ((indigo_device_context *)device->device_context)

#define MAIN_GROUP                    "Main"

#define PRIVATE_DATA                  (DEVICE_CONTEXT->private_data)

#define CONNECTION_PROPERTY           (DEVICE_CONTEXT->connection_property)
#define CONNECTION_CONNECTED_ITEM     (CONNECTION_PROPERTY->items+0)
#define CONNECTION_DISCONNECTED_ITEM  (CONNECTION_PROPERTY->items+1)

#define INFO_PROPERTY                 (DEVICE_CONTEXT->info_property)
#define INFO_DEVICE_NAME_ITEM         (INFO_PROPERTY->items+0)
#define INFO_DEVICE_VERSION_ITEM      (INFO_PROPERTY->items+1)
#define INFO_DEVICE_INTERFACE_ITEM    (INFO_PROPERTY->items+2)
#define INFO_FRAMEWORK_NAME_ITEM      (INFO_PROPERTY->items+3)
#define INFO_FRAMEWORK_VERSION_ITEM   (INFO_PROPERTY->items+4)

#define DEBUG_PROPERTY                (DEVICE_CONTEXT->debug_property)
#define DEBUG_ENABLED_ITEM            (DEBUG_PROPERTY->items+0)
#define DEBUG_DISABLED_ITEM           (DEBUG_PROPERTY->items+1)

#define SIMULATION_PROPERTY           (DEVICE_CONTEXT->simulation_property)
#define SIMULATION_ENABLED_ITEM       (SIMULATION_PROPERTY->items+0)
#define SIMULATION_DISABLED_ITEM      (SIMULATION_PROPERTY->items+1)

#define CONFIG_PROPERTY               (DEVICE_CONTEXT->congfiguration_property)
#define CONFIG_LOAD_ITEM              (CONFIG_PROPERTY->items+0)
#define CONFIG_SAVE_ITEM              (CONFIG_PROPERTY->items+1)
#define CONFIG_DEFAULT_ITEM           (CONFIG_PROPERTY->items+2)

typedef void (*indigo_timer_callback)(indigo_device *device);

typedef struct indigo_timer {
  indigo_device *device;
  indigo_timer_callback callback;
#if defined(INDIGO_LINUX)
  struct timespec time;
  struct indigo_timer *next;
#elif defined(INDIGO_DARWIN)
  bool canceled;
#endif
} indigo_timer;

typedef struct {
  void *private_data;
  int property_save_file_handle;
#if defined(INDIGO_LINUX)
  pthread_t timer_thread;
  pthread_mutex_t timer_mutex;
  int timer_pipe[2];
  indigo_timer *timer_queue;
#endif
  indigo_property *connection_property;
  indigo_property *info_property;
  indigo_property *debug_property;
  indigo_property *simulation_property;
  indigo_property *congfiguration_property;
} indigo_device_context;

extern indigo_result indigo_device_attach(indigo_device *device, char *name, indigo_version version, int interface);
extern indigo_result indigo_device_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property);
extern indigo_result indigo_device_change_property(indigo_device *device, indigo_client *client, indigo_property *property);
extern indigo_result indigo_device_detach(indigo_device *device);

extern indigo_result indigo_load_properties(indigo_device *device, bool default_properties);
extern indigo_result indigo_save_property(indigo_device*device, indigo_property *property);

extern indigo_timer *indigo_set_timer(indigo_device *device, double delay, indigo_timer_callback callback);
extern void indigo_cancel_timer(indigo_device *device, indigo_timer *timer);

#endif /* indigo_device_h */
