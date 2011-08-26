/*
 *  Copyright 2010 Thomas Bonfort
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include <geocache.h>
#include <apr_strings.h>
#include <apr_tables.h>
#include <curl/curl.h>

int geocache_util_extract_int_list(geocache_context *ctx, char* args, const char sep, int **numbers,
      int *numbers_count) {
   char *last, *key, *endptr;
   int tmpcount=1;
   char delim[2];
   delim[0] = sep;
   delim[1] = 0;

   *numbers_count = 0;
   for(key=args;*key;key++) {
      if(*key == sep)
         tmpcount++;
   }
   *numbers = (int*)apr_pcalloc(ctx->pool,tmpcount*sizeof(int));
   for (key = apr_strtok(args, delim, &last); key != NULL;
         key = apr_strtok(NULL, delim, &last)) {
      (*numbers)[(*numbers_count)++] = (int)strtol(key,&endptr,10);
      if(*endptr != 0)
         return GEOCACHE_FAILURE;
   }
   return GEOCACHE_SUCCESS;
}

int geocache_util_extract_double_list(geocache_context *ctx, char* args, const char sep, double **numbers,
      int *numbers_count) {
   char *last, *key, *endptr;
   int tmpcount=1;
   char delim[2];
   delim[0] = sep;
   delim[1] = 0;

   *numbers_count = 0;
   for(key=args;*key;key++) {
      if(*key == sep)
         tmpcount++;
   }
   *numbers = (double*)apr_pcalloc(ctx->pool,tmpcount*sizeof(double));
   for (key = apr_strtok(args, delim, &last); key != NULL;
         key = apr_strtok(NULL, delim, &last)) {
      (*numbers)[(*numbers_count)++] = strtod(key,&endptr);
      if(*endptr != 0)
         return GEOCACHE_FAILURE;
   }
   return GEOCACHE_SUCCESS;
}

geocache_error_code _geocache_context_get_error_default(geocache_context *ctx) {
    return ctx->_errcode;
}

char* _geocache_context_get_error_msg_default(geocache_context *ctx) {
    return ctx->_errmsg;
}

void _geocache_context_set_error_default(geocache_context *ctx, geocache_error_code code, char *msg, ...) {
    char *fmt;
    va_list args;
    va_start(args,msg);

    if(ctx->_errmsg) {
        fmt=apr_psprintf(ctx->pool,"%s\n%s",ctx->_errmsg,msg);
    } else {
        fmt=msg;
        ctx->_errcode = code;
    }
    ctx->_errmsg = apr_pvsprintf(ctx->pool,fmt,args);
    va_end(args);
}


void geocache_context_init(geocache_context *ctx) {
    ctx->_errcode = GEOCACHE_NO_ERROR;
    ctx->_errmsg = NULL;
    ctx->get_error = _geocache_context_get_error_default;
    ctx->get_error_message = _geocache_context_get_error_msg_default;
    ctx->set_error = _geocache_context_set_error_default;
}


