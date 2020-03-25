// Copyright (c) 2020 Rumen G.Bogdanovski
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
// 2.0 by Rumen G.Bogdanvski <rumen@skyarchive.org>

/** INDIGO token handling
 \file indigo_token.h
 */

#ifndef indigo_token_h
#define indigo_token_h

#define MAX_TOKENS 256

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t indigo_token;

/** Add device token to the list
 */
extern bool indigo_add_device_token(const char *device, indigo_token token);

/** Remove device access token from the list
 */
extern bool indigo_remove_device_token(const char *device);

/** Get device access token from the list or 0 if not set
 */
extern indigo_token indigo_get_device_token(const char *device);

/** Get master token if set else get device access token or 0 if not set
 */
extern indigo_token indigo_get_token(const char *device);

/** Get master token
 */
extern indigo_token indigo_get_master_token();

/** Set master token
 */
extern void indigo_set_master_token(indigo_token token);

/** Clear token list
 */
extern void indigo_clear_device_tokens();

#ifdef __cplusplus
}
#endif

#endif /* indigo_token_h */
