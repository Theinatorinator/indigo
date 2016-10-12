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

#ifndef indigo_ccd_device_h
#define indigo_ccd_device_h

#include "indigo_bus.h"
#include "indigo_driver.h"

#define CCD_DEVICE_CONTEXT                ((indigo_ccd_device_context *)device->device_context)

#define CCD_MAIN_GROUP                         "CCD main"
#define CCD_IMAGE_GROUP                        "CCD image"
#define CCD_COOLER_GROUP                       "CCD cooler"
#define CCD_GUIDER_GROUP                       "CCD guider"

#define CCD_INFO_PROPERTY                 (CCD_DEVICE_CONTEXT->ccd_info_property)
#define CCD_INFO_WIDTH_ITEM               (CCD_INFO_PROPERTY->items+0)
#define CCD_INFO_HEIGHT_ITEM              (CCD_INFO_PROPERTY->items+1)
#define CCD_INFO_MAX_HORIZONAL_BIN_ITEM   (CCD_INFO_PROPERTY->items+2)
#define CCD_INFO_MAX_VERTICAL_BIN_ITEM    (CCD_INFO_PROPERTY->items+3)
#define CCD_INFO_PIXEL_SIZE_ITEM          (CCD_INFO_PROPERTY->items+4)
#define CCD_INFO_PIXEL_WIDTH_ITEM         (CCD_INFO_PROPERTY->items+5)
#define CCD_INFO_PIXEL_HEIGHT_ITEM        (CCD_INFO_PROPERTY->items+6)
#define CCD_INFO_BITS_PER_PIXEL_ITEM      (CCD_INFO_PROPERTY->items+7)

#define CCD_UPLOAD_MODE_PROPERTY          (CCD_DEVICE_CONTEXT->ccd_upload_mode_property)
#define CCD_UPLOAD_MODE_CLIENT_ITEM       (CCD_UPLOAD_MODE_PROPERTY->items+0)
#define CCD_UPLOAD_MODE_LOCAL_ITEM        (CCD_UPLOAD_MODE_PROPERTY->items+1)
#define CCD_UPLOAD_MODE_BOTH_ITEM         (CCD_UPLOAD_MODE_PROPERTY->items+2)

#define CCD_LOCAL_MODE_PROPERTY           (CCD_DEVICE_CONTEXT->ccd_local_mode_property)
#define CCD_LOCAL_MODE_DIR_ITEM           (CCD_LOCAL_MODE_PROPERTY->items+0)
#define CCD_LOCAL_MODE_PREFIX_ITEM        (CCD_LOCAL_MODE_PROPERTY->items+1)

#define CCD_EXPOSURE_PROPERTY             (CCD_DEVICE_CONTEXT->ccd_exposure_property)
#define CCD_EXPOSURE_ITEM                 (CCD_EXPOSURE_PROPERTY->items+0)

#define CCD_ABORT_EXPOSURE_PROPERTY       (CCD_DEVICE_CONTEXT->ccd_abort_exposure_property)
#define CCD_ABORT_EXPOSURE_ITEM           (CCD_ABORT_EXPOSURE_PROPERTY->items+0)

#define CCD_FRAME_PROPERTY                (CCD_DEVICE_CONTEXT->ccd_frame_property)
#define CCD_FRAME_LEFT_ITEM               (CCD_FRAME_PROPERTY->items+0)
#define CCD_FRAME_TOP_ITEM                (CCD_FRAME_PROPERTY->items+1)
#define CCD_FRAME_WIDTH_ITEM              (CCD_FRAME_PROPERTY->items+2)
#define CCD_FRAME_HEIGHT_ITEM             (CCD_FRAME_PROPERTY->items+3)

#define CCD_BIN_PROPERTY                  (CCD_DEVICE_CONTEXT->ccd_bin_property)
#define CCD_BIN_HORIZONTAL_ITEM           (CCD_BIN_PROPERTY->items+0)
#define CCD_BIN_VERTICAL_ITEM             (CCD_BIN_PROPERTY->items+1)

#define CCD_FRAME_TYPE_PROPERTY           (CCD_DEVICE_CONTEXT->ccd_frame_type_property)
#define CCD_FRAME_TYPE_LIGHT_ITEM         (CCD_FRAME_TYPE_PROPERTY->items+0)
#define CCD_FRAME_TYPE_BIAS_ITEM          (CCD_FRAME_TYPE_PROPERTY->items+1)
#define CCD_FRAME_TYPE_DARK_ITEM          (CCD_FRAME_TYPE_PROPERTY->items+2)
#define CCD_FRAME_TYPE_FLAT_ITEM          (CCD_FRAME_TYPE_PROPERTY->items+3)

#define CCD_IMAGE_FORMAT_PROPERTY         (CCD_DEVICE_CONTEXT->ccd_image_format_property)
#define CCD_IMAGE_FORMAT_RAW_ITEM         (CCD_IMAGE_FORMAT_PROPERTY->items+0)
#define CCD_IMAGE_FORMAT_FITS_ITEM        (CCD_IMAGE_FORMAT_PROPERTY->items+1)
#define CCD_IMAGE_FORMAT_JPEG_ITEM        (CCD_IMAGE_FORMAT_PROPERTY->items+2)

#define CCD_IMAGE_FILE_PROPERTY           (CCD_DEVICE_CONTEXT->ccd_image_file_property)
#define CCD_IMAGE_FILE_ITEM               (CCD_IMAGE_FILE_PROPERTY->items+0)

#define CCD_IMAGE_PROPERTY                (CCD_DEVICE_CONTEXT->ccd_image_property)
#define CCD_IMAGE_ITEM                    (CCD_IMAGE_PROPERTY->items+0)

#define CCD_TEMPERATURE_PROPERTY          (CCD_DEVICE_CONTEXT->ccd_temperature_property)
#define CCD_TEMPERATURE_ITEM              (CCD_TEMPERATURE_PROPERTY->items+0)

#define CCD_COOLER_PROPERTY               (CCD_DEVICE_CONTEXT->ccd_cooler_property)
#define CCD_COOLER_ON_ITEM                (CCD_COOLER_PROPERTY->items+0)
#define CCD_COOLER_OFF_ITEM               (CCD_COOLER_PROPERTY->items+1)

#define CCD_COOLER_POWER_PROPERTY         (CCD_DEVICE_CONTEXT->ccd_cooler_power_property)
#define CCD_COOLER_POWER_ITEM             (CCD_COOLER_POWER_PROPERTY->items+0)

#define CCD_GUIDE_DEC_PROPERTY            (CCD_DEVICE_CONTEXT->ccd_guide_dec_property)
#define CCD_GUIDE_NORTH_ITEM              (CCD_GUIDE_DEC_PROPERTY->items+0)
#define CCD_GUIDE_SOUTH_ITEM              (CCD_GUIDE_DEC_PROPERTY->items+1)

#define CCD_GUIDE_RA_PROPERTY             (CCD_DEVICE_CONTEXT->ccd_guide_ra_property)
#define CCD_GUIDE_WEST_ITEM               (CCD_GUIDE_RA_PROPERTY->items+0)
#define CCD_GUIDE_EAST_ITEM               (CCD_GUIDE_RA_PROPERTY->items+1)


#define FITS_HEADER_SIZE  2880

typedef struct {
  indigo_device_context device_context;
  indigo_property *ccd_info_property;
  indigo_property *ccd_upload_mode_property;
  indigo_property *ccd_local_mode_property;
  indigo_property *ccd_exposure_property;
  indigo_property *ccd_abort_exposure_property;
  indigo_property *ccd_frame_property;
  indigo_property *ccd_bin_property;
  indigo_property *ccd_frame_type_property;
  indigo_property *ccd_image_format_property;
  indigo_property *ccd_image_property;
  indigo_property *ccd_image_file_property;
  indigo_property *ccd_temperature_property;
  indigo_property *ccd_cooler_property;
  indigo_property *ccd_cooler_power_property;
  indigo_property *ccd_guide_dec_property;
  indigo_property *ccd_guide_ra_property;
} indigo_ccd_device_context;

extern indigo_result indigo_ccd_device_attach(indigo_device *device, char *name, indigo_version version);
extern indigo_result indigo_ccd_device_enumerate_properties(indigo_device *device, indigo_client *client, indigo_property *property);
extern indigo_result indigo_ccd_device_change_property(indigo_device *device, indigo_client *client, indigo_property *property);
extern indigo_result indigo_ccd_device_detach(indigo_device *device);

typedef struct {
  char name[8];
  enum { INDIGO_FITS_STRING, INDIGO_FITS_NUMBER } type;
  union {
  char string[70];
    double number;
  } value;
} indigo_fits_header;

extern void indigo_process_image(indigo_device *device, void *data, double exposure_time);

#endif /* indigo_ccd_device_h */
