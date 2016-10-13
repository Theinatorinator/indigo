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
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR 'AS IS' AND ANY EXPRESS
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
//  2.0 Build 0 - PoC by Peter Polakovic <peter.polakovic@cloudmakers.eu>

/** INDIGO XML wire protocol parser
 \file indigo_xml.c
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <assert.h>
#include <pthread.h>

#include "base64.h"
#include "indigo_xml.h"
#include "indigo_version.h"
#include "indigo_driver_xml.h"

#define BUFFER_SIZE 4096
#define PROPERTY_SIZE sizeof(indigo_property)+INDIGO_MAX_ITEMS*(sizeof(indigo_item))

typedef enum {
  ERROR,
  IDLE,
  BEGIN_TAG1,
  BEGIN_TAG,
  ATTRIBUTE_NAME1,
  ATTRIBUTE_NAME,
  ATTRIBUTE_VALUE1,
  ATTRIBUTE_VALUE,
  TEXT1,
  TEXT,
  BLOB,
  END_TAG1,
  END_TAG2,
  END_TAG
} parser_state;


static char *parser_state_name[] = {
  "ERROR",
  "IDLE",
  "BEGIN_TAG1",
  "BEGIN_TAG",
  "ATTRIBUTE_NAME1",
  "ATTRIBUTE_NAME",
  "ATTRIBUTE_VALUE1",
  "ATTRIBUTE_VALUE",
  "TEXT1",
  "TEXT",
  "BLOB",
  "END_TAG1",
  "END_TAG2",
  "END_TAG"
};

static indigo_property_state parse_state(char *value) {
  if (!strcmp(value, "Ok"))
    return INDIGO_OK_STATE;
  if (!strcmp(value, "Busy"))
    return INDIGO_BUSY_STATE;
  if (!strcmp(value, "Alert"))
    return INDIGO_ALERT_STATE;
  return INDIGO_IDLE_STATE;
}

static indigo_property_perm parse_perm(char *value) {
  if (!strcmp(value, "rw"))
    return INDIGO_RW_PERM;
  if (!strcmp(value, "wo"))
    return INDIGO_WO_PERM;
  return INDIGO_RO_PERM;
}

static indigo_rule parse_rule(char *value) {
  if (!strcmp(value, "OneOfMany"))
    return INDIGO_ONE_OF_MANY_RULE;
  if (!strcmp(value, "AtMostOne"))
    return INDIGO_AT_MOST_ONE_RULE;
  return INDIGO_ANY_OF_MANY_RULE;
}

/* mutex used by writing indigo_xml_printf(), indigo_xml_write() */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

void indigo_xml_printf(int handle, const char *format, ...) {
  static char buffer[1024];
  if (!pthread_mutex_lock(&log_mutex)) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, 1024, format, args);
    va_end(args);
    write(handle, buffer, length);
    INDIGO_DEBUG(indigo_debug("sent: %s", buffer));
    pthread_mutex_unlock(&log_mutex);
  }
}

void indigo_xml_write(int handle, const char *buffer, int length) {
  if (!pthread_mutex_lock(&log_mutex)) {
    INDIGO_DEBUG(int written =) write(handle, buffer, length);
    INDIGO_DEBUG(indigo_debug("%s sent: %d bytes", __FUNCTION__, written));
    pthread_mutex_unlock(&log_mutex);
  }
}

typedef void *(* parser_handler)(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);

void *top_level_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *new_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *new_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *new_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *def_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *def_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *def_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *def_light_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *def_blob_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *set_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *set_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *set_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *set_light_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);
void *set_blob_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message);

void *enable_blob_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  assert(client != NULL);
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: enable_blob_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == END_TAG) {
    return top_level_handler;
  }
  return enable_blob_handler;
}

void *get_properties_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  assert(client != NULL);
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: get_properties_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "version")) {
      indigo_version version = INDIGO_VERSION_LEGACY;
      if (!strncmp(value, "1.", 2))
        version = INDIGO_VERSION_LEGACY;
      else if (!strcmp(value, "2.0"))
        version = INDIGO_VERSION_2_0;
      client->version = version;
    } else if (!strcmp(name, "switch")) {
      indigo_version version = INDIGO_VERSION_LEGACY;
      if (!strncmp(value, "1.", 2))
        version = INDIGO_VERSION_LEGACY;
      else if (!strcmp(value, "2.0"))
        version = INDIGO_VERSION_2_0;
      if (version > client->version) {
        assert(client->client_context != NULL);
        int handle = ((indigo_xml_device_adapter_context *)(client->client_context))->output;
        indigo_xml_printf(handle, "<switchProtocol version='%d.%d'/>\n", (version >> 8) & 0xFF, version & 0xFF);
        client->version = version;
      }
    } else if (!strncmp(name, "device",INDIGO_NAME_SIZE)) {
      strcpy(property->device, value);
    } else if (!strncmp(name, "name",INDIGO_NAME_SIZE)) {
      indigo_copy_property_name(client->version, property, value);;
    }
  } else if (state == END_TAG) {
    indigo_enumerate_properties(client, property);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return get_properties_handler;
}

void *new_one_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_one_text_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(client ? client->version : INDIGO_VERSION_CURRENT, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    strncat(property->items[property->count-1].text.value, value, INDIGO_VALUE_SIZE);
  } else if (state == END_TAG) {
    return new_text_vector_handler;
  }
  return new_one_text_vector_handler;
}

void *new_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_text_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneText")) {
      property->count++;
      return new_one_text_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(client ? client->version : INDIGO_VERSION_CURRENT, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    }
  } else if (state == END_TAG) {
    indigo_change_property(client, property);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return new_text_vector_handler;
}

void *new_one_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_one_number_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(client ? client->version : INDIGO_VERSION_CURRENT, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].number.value = atof(value);
  } else if (state == END_TAG) {
    return new_number_vector_handler;
  }
  return new_one_number_vector_handler;
}

void *new_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_number_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneNumber")) {
      property->count++;
      return new_one_number_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(client ? client->version : INDIGO_VERSION_CURRENT, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    }
  } else if (state == END_TAG) {
    indigo_change_property(client, property);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return new_number_vector_handler;
}

void *new_one_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_one_switch_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(client ? client->version : INDIGO_VERSION_CURRENT, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].sw.value = !strcmp(value, "On");
  } else if (state == END_TAG) {
    return new_switch_vector_handler;
  }
  return new_one_switch_vector_handler;
}

void *new_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: new_switch_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneSwitch")) {
      property->count++;
      return new_one_switch_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(client ? client->version : INDIGO_VERSION_CURRENT, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    }
    return new_switch_vector_handler;
  } else if (state == END_TAG) {
    indigo_change_property(client, property);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return new_switch_vector_handler;
}

void *switch_protocol_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  assert(device != NULL);
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: switch_protocol_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "version")) {
      int major, minor;
      sscanf(value, "%d.%d", &major, &minor);
      device->version = major << 8 | minor;
    }
  } else if (state == END_TAG) {
    return top_level_handler;
  }
  return switch_protocol_handler;
}

void *set_one_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_one_text_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    strncat(property->items[property->count-1].text.value, value, INDIGO_VALUE_SIZE);
  } else if (state == END_TAG) {
    return set_text_vector_handler;
  }
  return set_one_text_vector_handler;
}

void *set_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_text_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneText")) {
      property->count++;
      return set_one_text_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_update_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return set_text_vector_handler;
}

void *set_one_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_one_number_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].number.value = atof(value);
  } else if (state == END_TAG) {
    return set_number_vector_handler;
  }
  return set_one_number_vector_handler;
}

void *set_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_number_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneNumber")) {
      property->count++;
      return set_one_number_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_update_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return set_number_vector_handler;
}

void *set_one_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_one_switch_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
      return set_one_switch_vector_handler;
    }
  } else if (state == TEXT) {
    property->items[property->count-1].sw.value = !strcmp(value, "On");
    return set_one_switch_vector_handler;
  }
  return set_switch_vector_handler;
}

void *set_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_switch_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneSwitch")) {
      property->count++;
      return set_one_switch_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_update_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return set_switch_vector_handler;
}

void *set_one_light_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_one_light_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].light.value = parse_state(value);
  } else if (state == END_TAG) {
    return set_light_vector_handler;
  }
  return set_one_light_vector_handler;
}

void *set_light_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_light_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneLight")) {
      property->count++;
      return set_one_light_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_update_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return set_light_vector_handler;
}

void *set_one_blob_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_one_blob_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "format")) {
      strncpy(property->items[property->count-1].blob.format, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "size")) {
      property->items[property->count-1].blob.size = atol(value);
    }
  } else if (state == BLOB) {
    property->items[property->count-1].blob.value = value;
  } else if (state == END_TAG) {
    return set_blob_vector_handler;
  }
  return set_one_blob_vector_handler;
}

void *set_blob_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: set_blob_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "oneBLOB")) {
      property->count++;
      return set_one_blob_vector_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_update_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return set_blob_vector_handler;
}

void *def_text_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_text_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "label")) {
      strncpy(property->items[property->count-1].label, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == TEXT) {
    strncat(property->items[property->count-1].text.value, value, INDIGO_VALUE_SIZE);
  } else if (state == END_TAG) {
    return def_text_vector_handler;
  }
  return def_text_handler;
}

void *def_text_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_text_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "defText")) {
      property->count++;
      return def_text_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "group")) {
      strncpy(property->group, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "label")) {
      strncpy(property->label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "perm")) {
      property->perm = parse_perm(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_define_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return def_text_vector_handler;
}

void *def_number_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_number_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "label")) {
      strncpy(property->items[property->count-1].label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "min")) {
      property->items[property->count-1].number.min = atof(value);
    } else if (!strcmp(name, "max")) {
      property->items[property->count-1].number.max = atof(value);
    } else if (!strcmp(name, "step")) {
      property->items[property->count-1].number.step = atof(value);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].number.value = atof(value);
  } else if (state == END_TAG) {
    return def_number_vector_handler;
  }
  return def_number_handler;
}

void *def_number_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_number_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "defNumber")) {
      property->count++;
      return def_number_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "group")) {
      strncpy(property->group, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "label")) {
      strncpy(property->label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "perm")) {
      property->perm = parse_perm(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_define_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return def_number_vector_handler;
}

void *def_switch_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_switch_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "label")) {
      strncpy(property->items[property->count-1].label, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].sw.value = !strcmp(value, "On");
  } else if (state == END_TAG) {
    return def_switch_vector_handler;
  }
  return def_switch_handler;
}

void *def_switch_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_switch_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "defSwitch")) {
      property->count++;
      return def_switch_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "group")) {
      strncpy(property->group, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "label")) {
      strncpy(property->label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "perm")) {
      property->perm = parse_perm(value);
    } else if (!strcmp(name, "rule")) {
      property->rule = parse_rule(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_define_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return def_switch_vector_handler;
}

void *def_light_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_light_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "label")) {
      strncpy(property->items[property->count-1].label, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == TEXT) {
    property->items[property->count-1].light.value = parse_state(value);
  } else if (state == END_TAG) {
    return def_light_vector_handler;
  }
  return def_light_handler;
}

void *def_light_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_light_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "defLight")) {
      property->count++;
      return def_light_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "group")) {
      strncpy(property->group, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "label")) {
      strncpy(property->label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_define_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return def_light_vector_handler;
}

void *def_blob_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_blob_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "name")) {
      indigo_copy_item_name(device->version, property, property->items+property->count-1, value);
    } else if (!strcmp(name, "label")) {
      strncpy(property->items[property->count-1].label, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    return def_blob_vector_handler;
  }
  return def_blob_handler;
}

void *def_blob_vector_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: def_blob_vector_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    if (!strcmp(name, "defBLOB")) {
      property->count++;
      return def_blob_handler;
    }
  } else if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "device")) {
      strncpy(property->device, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "name")) {
      indigo_copy_property_name(device->version, property, value);
    } else if (!strcmp(name, "group")) {
      strncpy(property->group, value,INDIGO_NAME_SIZE);
    } else if (!strcmp(name, "label")) {
      strncpy(property->label, value, INDIGO_VALUE_SIZE);
    } else if (!strcmp(name, "state")) {
      property->state = parse_state(value);
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_define_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return def_blob_vector_handler;
}

void *del_property_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: del_property_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strncmp(name, "device",INDIGO_NAME_SIZE)) {
      strcpy(property->device, value);
    } else if (!strncmp(name, "name",INDIGO_NAME_SIZE)) {
      indigo_copy_property_name(device->version, property, value);;
    } else if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_delete_property(device, property, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return del_property_handler;
}

void *message_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: message_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == ATTRIBUTE_VALUE) {
    if (!strcmp(name, "message")) {
      strncpy(message, value, INDIGO_VALUE_SIZE);
    }
  } else if (state == END_TAG) {
    indigo_send_message(device, *message ? message : NULL);
    memset(property, 0, PROPERTY_SIZE);
    return top_level_handler;
  }
  return message_handler;
}

void *top_level_handler(parser_state state, char *name, char *value, indigo_property *property, indigo_device *device, indigo_client *client, char *message) {
  INDIGO_DEBUG_PROTOCOL(indigo_trace("XML Parser: top_level_handler %s '%s' '%s'", parser_state_name[state], name != NULL ? name : "", value != NULL ? value : ""));
  if (state == BEGIN_TAG) {
    *message = 0;
    if (!strcmp(name, "enableBLOB"))
      return enable_blob_handler;
    if (!strcmp(name, "getProperties"))
      return get_properties_handler;
    if (!strcmp(name, "newTextVector")) {
      property->type = INDIGO_TEXT_VECTOR;
      return new_text_vector_handler;
    }
    if (!strcmp(name, "newNumberVector")) {
      property->type = INDIGO_NUMBER_VECTOR;
      return new_number_vector_handler;
    }
    if (!strcmp(name, "newSwitchVector")) {
      property->type = INDIGO_SWITCH_VECTOR;
      return new_switch_vector_handler;
    }
    if (!strcmp(name, "switchProtocol"))
      return switch_protocol_handler;
    if (!strcmp(name, "setTextVector")) {
      property->type = INDIGO_TEXT_VECTOR;
      return set_text_vector_handler;
    }
    if (!strcmp(name, "setNumberVector")) {
      property->type = INDIGO_NUMBER_VECTOR;
      return set_number_vector_handler;
    }
    if (!strcmp(name, "setSwitchVector")) {
      property->type = INDIGO_SWITCH_VECTOR;
      return set_switch_vector_handler;
    }
    if (!strcmp(name, "setLightVector")) {
      property->type = INDIGO_LIGHT_VECTOR;
      return set_light_vector_handler;
    }
    if (!strcmp(name, "setBLOBVector")) {
      property->type = INDIGO_BLOB_VECTOR;
      return set_blob_vector_handler;
    }
    if (!strcmp(name, "defTextVector")) {
      property->type = INDIGO_TEXT_VECTOR;
      return def_text_vector_handler;
    }
    if (!strcmp(name, "defNumberVector")) {
      property->type = INDIGO_NUMBER_VECTOR;
      return def_number_vector_handler;
    }
    if (!strcmp(name, "defSwitchVector")) {
      property->type = INDIGO_SWITCH_VECTOR;
      return def_switch_vector_handler;
    }
    if (!strcmp(name, "defLightVector")) {
      property->type = INDIGO_LIGHT_VECTOR;
      return def_light_vector_handler;
    }
    if (!strcmp(name, "defBLOBVector")) {
      property->type = INDIGO_BLOB_VECTOR;
      return def_blob_vector_handler;
    }
    if (!strcmp(name, "delProperty"))
      return del_property_handler;
    if (!strcmp(name, "message"))
      return message_handler;
  }
  return top_level_handler;
}

void indigo_xml_parse(int handle, indigo_device *device, indigo_client *client) {
  char buffer[BUFFER_SIZE] = { 0 };
  char name_buffer[INDIGO_NAME_SIZE];
  char value_buffer[INDIGO_VALUE_SIZE];
  char property_buffer[PROPERTY_SIZE];
  unsigned char *blob_buffer = NULL;
  char *pointer = buffer;
  char *name_pointer = name_buffer;
  char *value_pointer = value_buffer;
  unsigned char *blob_pointer = NULL;
  char message[INDIGO_VALUE_SIZE];
  parser_state state = IDLE;
  char q = '"';
  int depth = 0;
  char c;  
  (void)parser_state_name;

  parser_handler handler = top_level_handler;
  indigo_property *property = (indigo_property *)property_buffer;
  memset(property_buffer, 0, PROPERTY_SIZE);
  
  if (device != NULL) {
    device->enumerate_properties(device, client, NULL);
  }
  
  while (true) {
    if (state == ERROR) {
      indigo_error("XML Parser: syntax error");
      return;
    }
    while ((c = *pointer++) == 0) {
      ssize_t count = (int)read(handle, (void *)buffer, (ssize_t)BUFFER_SIZE);
      if (count <= 0) {
        if (blob_buffer != NULL)
          free(blob_buffer);
        close(handle);
        return;
      }
      pointer = buffer;
      buffer[count] = 0;
      INDIGO_TRACE_PROTOCOL(indigo_debug("received: %s", buffer));
    }
    switch (state) {
      case IDLE:
        if (c == '<') {
          state = BEGIN_TAG1;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' IDLE -> BEGIN_TAG1", c));
        }
        break;
      case BEGIN_TAG1:
        name_pointer = name_buffer;
        if (isalpha(c)) {
          *name_pointer++ = c;
          state = BEGIN_TAG;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG1 -> BEGIN_TAG", c));
        } else if (c == '/') {
          state = END_TAG;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG1 -> END_TAG", c));
        }
        break;
      case BEGIN_TAG:
        if (name_pointer - name_buffer <INDIGO_NAME_SIZE && isalpha(c)) {
          *name_pointer++ = c;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG", c));
        } else {
          *name_pointer = 0;
          depth++;
          handler = handler(BEGIN_TAG, name_buffer, NULL, property, device, client, message);
          if (isblank(c)) {
            state = ATTRIBUTE_NAME1;
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG -> ATTRIBUTE_NAME1", c));
          } else if (c == '/') {
            state = END_TAG1;
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG -> END_TAG1", c));
          } else if (c == '>') {
            if (property->type == INDIGO_BLOB_VECTOR) {
              state = BLOB;
              if (blob_buffer != NULL)
                blob_buffer = realloc(blob_buffer, property->items[property->count-1].blob.size);
              else
                blob_buffer = malloc(property->items[property->count-1].blob.size);
              blob_pointer = blob_buffer;
            } else
              state = TEXT;
            value_pointer = value_buffer;
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' BEGIN_TAG -> TEXT", c));
          } else {
            state = ERROR;
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' error BEGIN_TAG", c));
          }
        }
        break;
      case END_TAG1:
        if (c == '>') {
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' END_TAG1 -> IDLE", c));
          handler = handler(END_TAG, NULL, NULL, property, device, client, message);
          depth--;
          state = IDLE;
        } else {
          state = ERROR;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' error END_TAG1", c));
        }
        break;
      case END_TAG2:
        if (c == '/') {
          state = END_TAG;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' END_TAG2 -> END_TAG", c));
        } else {
          state = ERROR;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' error END_TAG2", c));
        }
        break;
      case END_TAG:
        if (isalpha(c)) {
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' END_TAG", c));
        } else if (c == '>') {
          handler = handler(END_TAG, NULL, NULL, property, device, client, message);
          depth--;
          state = IDLE;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' END_TAG -> IDLE", c));
        } else {
          state = ERROR;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' error END_TAG", c));
        }
        break;
      case TEXT:
        if (c == '<') {
          if (depth == 2) {
            *value_pointer = 0;
            handler = handler(TEXT, NULL, value_buffer, property, device, client, message);
          }
          state = TEXT1;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' %d TEXT -> TEXT1", c, depth));
          break;
        } else {
          if (depth == 2) {
            if (value_pointer - value_buffer < INDIGO_VALUE_SIZE) {
              *value_pointer++ = c;
            }
          }
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' %d TEXT", c, depth));
        }
        break;
      case TEXT1:
        if (c=='/') {
          state = END_TAG;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' TEXT -> END_TAG", c));
        } else if (isalpha(c)) {
          name_pointer = name_buffer;
          *name_pointer++ = c;
          state = BEGIN_TAG;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' TEXT -> BEGIN_TAG", c));
        }
        break;
      case BLOB:
        if (c == '<') {
          if (depth == 2) {
            *value_pointer = 0;
            blob_pointer += base64_decode_fast(value_buffer, blob_pointer, (int)(value_pointer-value_buffer));
            handler = handler(BLOB, NULL, (char *)blob_buffer, property, device, client, message);
          }
          state = TEXT1;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' %d BLOB -> TEXT1", c, depth));
          break;
        } else if (c != '\n') {
          if (depth == 2) {
            if (value_pointer - value_buffer < 72) {
              *value_pointer++ = c;
            } else {
              *value_pointer = 0;
              blob_pointer += base64_decode_fast(value_buffer, blob_pointer, (int)(value_pointer-value_buffer));
              value_pointer = value_buffer;
              memset(value_buffer, 0, sizeof(value_buffer));
              *value_pointer++ = c;
            }
          }
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' %d BLOB", c, depth));
        }
        break;
      case ATTRIBUTE_NAME1:
        if (isspace(c)) {
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME1", c));
        } else if (isalpha(c)) {
          name_pointer = name_buffer;
          *name_pointer++ = c;
          state = ATTRIBUTE_NAME;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME1 -> ATTRIBUTE_NAME", c));
        } else if (c == '/') {
          state = END_TAG1;
        } else if (c == '>') {
          value_pointer = value_buffer;
          if (property->type == INDIGO_BLOB_VECTOR) {
            state = BLOB;
            if (blob_buffer != NULL)
              blob_buffer = realloc(blob_buffer, property->items[property->count-1].blob.size);
            else
              blob_buffer = malloc(property->items[property->count-1].blob.size);
            blob_pointer = blob_buffer;
          } else
            state = TEXT;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME1 -> TEXT", c));
        }
       break;
      case ATTRIBUTE_NAME:
        if (name_pointer - name_buffer <INDIGO_NAME_SIZE && isalpha(c)) {
          *name_pointer++ = c;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME", c));
        } else {
          *name_pointer = 0;
          if (c == '=') {
            state = ATTRIBUTE_VALUE1;
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME -> ATTRIBUTE_VALUE1", c));
          } else {
            INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_NAME", c));
          }
        }
        break;
      case ATTRIBUTE_VALUE1:
        if (c == '"' || c == '\'') {
          q = c;
          value_pointer = value_buffer;
          state = ATTRIBUTE_VALUE;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_VALUE1 -> ATTRIBUTE_VALUE2", c));
        } else {
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_VALUE1", c));
        }
        break;
      case ATTRIBUTE_VALUE:
        if (c == q) {
          *value_pointer = 0;
          state = ATTRIBUTE_NAME1;
          handler = handler(ATTRIBUTE_VALUE, name_buffer, value_buffer, property, device, client, message);
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_VALUE -> ATTRIBUTE_NAME1", c));
        } else {
          *value_pointer++ = c;
          INDIGO_TRACE_PROTOCOL(indigo_trace("XML Parser: '%c' ATTRIBUTE_VALUE", c));
        }
        break;
      default:
        break;
    }
  }
  INDIGO_DEBUG(indigo_debug("XML Parser: parser finished"));
}
