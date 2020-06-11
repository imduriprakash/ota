/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/*
 *  Cypress OTA Agent network abstraction for HTTP
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <FreeRTOS.h>

/* lwIP header files */
#include <lwip/tcpip.h>
#include <lwip/api.h>

#include "cy_ota_api.h"
#include "cy_ota_internal.h"
#include "ip4_addr.h"

#include "iot_platform_types.h"
#include "cyabs_rtos.h"
#include "cy_iot_network_secured_socket.h"


/***********************************************************************
 *
 * HTTP network Functions
 *
 **********************************************************************/


/***********************************************************************
 *
 * defines & enums
 *
 **********************************************************************/

typedef enum
{
    HTTP_CONTINUE                        = 100,
    HTTP_SWITCHING_PROTOCOLS             = 101,
    HTTP_RESPONSE_OK                     = 200,
    HTTP_CREATED                         = 201,
    HTTP_ACCEPTED                        = 202,
    HTTP_NONAUTHORITATIVE                = 203,
    HTTP_NO_CONTENT                      = 204,
    HTTP_RESET_CONTENT                   = 205,
    HTTP_PARTIAL_CONTENT                 = 206,
    HTTP_MULTIPLE_CHOICES                = 300,
    HTTP_MOVED_PERMANENTLY               = 301,
    HTTP_FOUND                           = 302,
    HTTP_SEE_OTHER                       = 303,
    HTTP_NOT_MODIFIED                    = 304,
    HTTP_USEPROXY                        = 305,
    HTTP_TEMPORARY_REDIRECT              = 307,
    HTTP_BAD_REQUEST                     = 400,
    HTTP_UNAUTHORIZED                    = 401,
    HTTP_PAYMENT_REQUIRED                = 402,
    HTTP_FORBIDDEN                       = 403,
    HTTP_NOT_FOUND                       = 404,
    HTTP_METHOD_NOT_ALLOWED              = 405,
    HTTP_NOT_ACCEPTABLE                  = 406,
    HTTP_PROXY_AUTHENTICATION_REQUIRED   = 407,
    HTTP_REQUEST_TIMEOUT                 = 408,
    HTTP_CONFLICT                        = 409,
    HTTP_GONE                            = 410,
    HTTP_LENGTH_REQUIRED                 = 411,
    HTTP_PRECONDITION_FAILED             = 412,
    HTTP_REQUESTENTITYTOOLARGE           = 413,
    HTTP_REQUESTURITOOLONG               = 414,
    HTTP_UNSUPPORTEDMEDIATYPE            = 415,
    HTTP_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
    HTTP_EXPECTATION_FAILED              = 417,
    HTTP_INTERNAL_SERVER_ERROR           = 500,
    HTTP_NOT_IMPLEMENTED                 = 501,
    HTTP_BAD_GATEWAY                     = 502,
    HTTP_SERVICE_UNAVAILABLE             = 503,
    HTTP_GATEWAY_TIMEOUT                 = 504,
    HTTP_VERSION_NOT_SUPPORTED           = 505,
} http_status_code_t;

/***********************************************************************
 *
 * Macros
 *
 **********************************************************************/

/***********************************************************************
 *
 * Structures
 *
 **********************************************************************/

/***********************************************************************
 *
 * Data & Variables
 *
 **********************************************************************/

/* template for HTTP GET */
static char get_request_template[] =
{
    "GET %s HTTP/1.1\r\n"
    "Host: %s:%d \r\n"
    "\r\n"
};

#define HTTP_HEADER_STR                 "HTTP/"
#define CONTENT_STRING                  "Content-Length:"
#define HTTP_HEADERS_BODY_SEPARATOR     "\r\n\r\n"

/***********************************************************************
 *
 * Forward declarations
 *
 **********************************************************************/

/***********************************************************************
 *
 * Functions
 *
 **********************************************************************/

#ifdef PRINT_DATA
void cy_ota_http_print_data( const char *buffer, uint32_t length)
{
    uint32_t i, j;

    IotLogInfo("%s() %p %d\n", __func__, buffer, length);

    cy_rtos_delay_milliseconds(1000);

    for (i = 0 ; i < length; i+=16)
    {
        printf("0x%04lx ", i);
        for (j = 0 ; j < 16; j++)
        {
            if ((i + j) < length)
            {
                printf("0x%02x ", buffer[ i + j]);
            }
            else
            {
                printf("     ");
            }
        }
        printf("    ");
        for (j = 0 ; j < 16; j++)
        {
            if ((i + j) < length)
            {
                printf("%c", (isprint(buffer[ i + j]) ? buffer[ i + j] : '.'));
            }
            else
            {
                break;
            }
        }
        printf("\n");
    }
    IotLogInfo("\n");
}
#endif

/**
 * Length limited version of strstr. Ported from wiced_lib.c
 *
 * @param s[in]             : The string to be searched.
 * @param s_len[in]         : The length of the string to be searched.
 * @param substr[in]        : The string to be found.
 * @param substr_len[in]    : The length of the string to be found.
 *
 * @return    pointer to the found string if search successful, otherwise NULL
 */
char* strnstrn(const char *s, uint16_t s_len, const char *substr, uint16_t substr_len)
{
    for (; s_len >= substr_len; s++, s_len--)
    {
        if (strncmp(s, substr, substr_len) == 0)
        {
            return (char*)s;
        }
    }

    return NULL;
}

void cy_ota_http_timer_callback(cy_timer_callback_arg_t arg)
{
    cy_ota_context_t *ctx = (cy_ota_context_t *)arg;
    CY_OTA_CONTEXT_ASSERT(ctx);

    IotLogDebug("%s() new event:%d\n", __func__, ctx->u.http.http_timer_event);
    /* yes, we set the ota_event as the http get() function uses the same event var */
    cy_rtos_setbits_event(&ctx->ota_event, ctx->u.http.http_timer_event, 0);
}

cy_rslt_t cy_ota_stop_http_timer(cy_ota_context_t *ctx)
{
    CY_OTA_CONTEXT_ASSERT(ctx);
    return cy_rtos_stop_timer(&ctx->u.http.http_timer);
}

cy_rslt_t cy_ota_start_http_timer(cy_ota_context_t *ctx, uint32_t secs, ota_events_t event)
{
    cy_rslt_t result;
    uint32_t    num_ms = SECS_TO_MILLISECS(secs);

    CY_OTA_CONTEXT_ASSERT(ctx);

    cy_ota_stop_http_timer(ctx);
    ctx->u.http.http_timer_event = event;
    result = cy_rtos_start_timer(&ctx->u.http.http_timer, num_ms);
    return result;
}


cy_rslt_t cy_ota_http_parse_header( uint8_t **ptr, uint16_t *data_len, uint32_t *file_len, http_status_code_t *response_code)
{
    char    *response_status;
    uint8_t *header_end;

    if ( (ptr == NULL) || (*ptr == NULL) ||
         (data_len == NULL) || (*data_len == 0) ||
         (file_len == NULL) ||
         (response_code == NULL) )
    {
        return CY_RSLT_MODULE_OTA_BADARG;
    }
    *response_code = HTTP_FORBIDDEN;

    /* example:
        "HTTP/1.1 200 Ok\r\n"
        "Server: mini_httpd/1.23 28Dec2015\r\n"
        "Date: Tue, 03 Mar 2020 18:49:23 GMT\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 830544\r\n"
        "\r\n\r\n"
     */

    /* sanity check */
    if (*data_len < 12)
    {
        return CY_RSLT_MODULE_OTA_NOT_A_HEADER;
    }

    /* Find the HTTP/x.x part*/
    response_status = strnstrn( (char *)*ptr, *data_len, HTTP_HEADER_STR, sizeof(HTTP_HEADER_STR) - 1 );
    if (response_status == NULL)
    {
        return CY_RSLT_MODULE_OTA_NOT_A_HEADER;
    }
    /* skip to next ' ' space character */
    response_status = strchr(response_status, ' ');
    if (response_status == NULL)
    {
        return CY_RSLT_MODULE_OTA_NOT_A_HEADER;
    }
    *response_code = (http_status_code_t)atoi(response_status + 1);

    /* Find Content-Length part*/
    response_status = strnstrn( (char *)*ptr, *data_len, CONTENT_STRING, sizeof(CONTENT_STRING) - 1);
    if (response_status == NULL)
    {
        return CY_RSLT_MODULE_OTA_NOT_A_HEADER;
    }
    response_status += sizeof(CONTENT_STRING);
    *file_len = atoi(response_status);

    /* find end of header */
    header_end = (uint8_t *)strnstrn( (char *)*ptr, *data_len, HTTP_HEADERS_BODY_SEPARATOR, sizeof(HTTP_HEADERS_BODY_SEPARATOR) - 1);
    if (header_end == NULL)
    {
        return CY_RSLT_MODULE_OTA_NOT_A_HEADER;
    }
    header_end += sizeof(HTTP_HEADERS_BODY_SEPARATOR) - 1;
    *data_len -= (header_end - *ptr);
    IotLogDebug("Move ptr from %p to %p skipping %d new_len:%d first_byte:0x%x\n", *ptr, header_end, (header_end - *ptr), *data_len, *header_end);

    *ptr = header_end;
    return CY_RSLT_SUCCESS;
}

/**
 * @brief Validate network parameters
 *
 * NOTE: Individual Network Transport type will test appropriate fields
 *
 * @param[in]  network_params   pointer to Network parameter structure
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_MODULE_OTA_ERROR
 */
cy_rslt_t cy_ota_http_validate_network_params(cy_ota_network_params_t *network_params)
{
    IotLogInfo("%s()\n", __func__);
    if(network_params == NULL)
    {
        return CY_RSLT_MODULE_OTA_BADARG;
    }

    if ( (network_params->server.pHostName == NULL) || (network_params->u.http.data_file == NULL) )
    {
        return CY_RSLT_MODULE_OTA_BADARG;
    }
    return CY_RSLT_SUCCESS;
}


/**
 * @brief Write a chunk of OTA data to FLASH
 *
 * @param[in]   ctx         - ptr to OTA context
 * @param[in]   chunk_info  - ptr to a chunk_info structure
 *
 * @return      CY_RSLT_SUCCESS
 *              CY_RSLT_MODULE_OTA_BADARG
 *              CY_RSLT_MODULE_OTA_WRITE_STORAGE_ERROR
 */
cy_rslt_t cy_ota_http_write_chunk_to_flash(cy_ota_context_t *ctx, cy_ota_storage_write_info_t *chunk_info)
{
    IotLogDebug("%s()\n", __func__);

    if ( (ctx == NULL) || (chunk_info == NULL) )
    {
        IotLogError("%s() Bad args\n", __func__);
        return CY_RSLT_MODULE_OTA_BADARG;
    }

    ctx->num_packets_received++;    /* this is so we don't have a false failure with the per packet timer */

    /* store the chunk */
    if (cy_ota_storage_write(ctx, chunk_info) != CY_RSLT_SUCCESS)
    {
        IotLogError("%s() Write failed\n", __func__);
        cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DATA_FAIL, 0);
        return CY_RSLT_MODULE_OTA_WRITE_STORAGE_ERROR;
    }

    /* update the stats */
    ctx->total_bytes_written   += chunk_info->size;
    ctx->last_offset            = chunk_info->offset;
    ctx->last_size              = chunk_info->size;
    ctx->last_packet_received   = chunk_info->packet_number;
    ctx->total_packets          = chunk_info->total_packets;

    IotLogInfo("Written to offset:%ld  %ld of %ld (%ld remaining)\n",
                ctx->last_offset, ctx->total_bytes_written, ctx->total_image_size,
                (ctx->total_image_size - ctx->total_bytes_written) );

    cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_GOT_DATA, 0);

    return CY_RSLT_SUCCESS;
}

/**
 * Provide an asynchronous notification of incoming network data.
 *
 * A function with this signature may be set with platform_network_function_setreceivecallback
 * to be invoked when data is available on the network.
 *
 * param[in] pConnection The connection on which data is available, defined by
 * the network stack.
 * param[in] pContext The third argument passed to @ref platform_network_function_setreceivecallback.
 */
void cy_ota_http_receive_callback( IotNetworkConnection_t pConnection, void * pContext )
{
    cy_rslt_t           result = CY_RSLT_SUCCESS;
    uint32_t            bytes_received;
    uint16_t            data_len;
    uint32_t            file_len;
    uint8_t             *ptr;
    cy_ota_context_t    *ctx = (cy_ota_context_t *)pContext;
    CY_OTA_CONTEXT_ASSERT(ctx);

    cy_rtos_get_mutex(&ctx->sub_callback_mutex, CY_OTA_WAIT_HTTP_MUTEX_MS);

    IotLogInfo("%s()", __func__);

    if (pConnection != NULL)
    {
        uint32_t data_to_receive = OTA_HTTP_SIZE_OF_RECV_BUFFER;

        if ( (ctx->total_image_size > 0) && ( (ctx->total_image_size - ctx->total_bytes_written) < data_to_receive) )
        {
            data_to_receive = (ctx->total_image_size - ctx->total_bytes_written);
        }

        ptr = (uint8_t *)ctx->u.http.data_buffer;
        bytes_received = IotNetworkSecureSockets_Receive(ctx->u.http.connection, ptr, data_to_receive);
        if (bytes_received == 0)
        {
            IotLogError("%s() IotNetworkSecureSockets_Receive() received %ld\n", __func__, bytes_received);
        }
        else
        {
            cy_ota_storage_write_info_t chunk_info = { 0 };
            data_len = bytes_received;

            IotLogInfo("offset %d data_len %d\n", ctx->total_bytes_written, data_len);
#ifdef PRINT_DATA
            cy_ota_http_print_data( (const char *)ptr, data_len);
#endif
            if (ctx->total_bytes_written == 0)
            {
                http_status_code_t response_code;
                /* first block here - check the HTTP header */
                IotLogInfo("call cy_ota_http_parse_header( %p %d)\n", ptr, data_len);
                result = cy_ota_http_parse_header(&ptr, &data_len, &file_len, &response_code);
                IotLogInfo("     cy_ota_http_parse_header() %p %d\n", ptr, data_len);
                if (result != CY_RSLT_SUCCESS)
                {
                    /* couldn't parse the header */
                    IotLogError( "HTTP parse header fail: 0x%lx !\r\n ", result);
                    result = CY_RSLT_MODULE_OTA_GET_ERROR;
                }
                else if (response_code < 100)
                {
                    /* do nothing here */
                }
                else if (response_code < 200 )
                {
                    /* 1xx (Informational): The request was received, continuing process */
                }
                else if (response_code < 300 )
                {
                    /* 2xx (Successful): The request was successfully received, understood, and accepted */
                    chunk_info.total_size = file_len;
                    ctx->total_image_size = file_len;
                    IotLogError("%s() HTTP File Length: 0x%lx (%ld)\n", __func__, chunk_info.total_size, chunk_info.total_size);
                }
                else if (response_code < 400 )
                {
                    /* 3xx (Redirection): Further action needs to be taken in order to complete the request */
                    IotLogError( "HTTP response code: %d, redirection - code needed to handle this!\r\n ", response_code);
                    result = CY_RSLT_MODULE_OTA_GET_ERROR;
                }
                else
                {
                    /* 4xx (Client Error): The request contains bad syntax or cannot be fulfilled */
                    IotLogError( "HTTP response code: %d, ERROR!\r\n ", response_code);
                    result = CY_RSLT_MODULE_OTA_SERVER_DROPPED_CNCT;
                }
            }

            if (result == CY_RSLT_SUCCESS)
            {
                /* set parameters for writing */
                chunk_info.offset = ctx->total_bytes_written;
                chunk_info.buffer = ptr;
                chunk_info.size   = data_len;
                IotLogDebug("call cy_ota_http_write_chunk_to_flash( %p %d)\n", ptr, data_len);
                cy_ota_http_write_chunk_to_flash(ctx, &chunk_info);
                if (result != CY_RSLT_SUCCESS)
                {
                    IotLogError("%s() cy_ota_storage_write() failed 0x%lx\n", __func__, result);
                    result = CY_RSLT_MODULE_OTA_WRITE_STORAGE_ERROR;
                }
            }
        }
    }

    if (result == CY_RSLT_MODULE_OTA_GET_ERROR)
    {
        cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DATA_FAIL, 0);
    }
    else if (result == CY_RSLT_MODULE_OTA_SERVER_DROPPED_CNCT)
    {
        cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DROPPED_US, 0);
    }
    else if (result == CY_RSLT_MODULE_OTA_WRITE_STORAGE_ERROR)
    {
        cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_AGENT_STORAGE_ERROR, 0);
    }

    cy_rtos_set_mutex(&ctx->sub_callback_mutex);
}

/**
 * brief Provide an asynchronous notification of network closing
 *
 * A function with this signature may be set with platform_network_function_setclosecallback
 * to be invoked when the network connection is closed.
 *
 * param[in] pConnection The connection that was closed, defined by
 * the network stack.
 * param[in] reason The reason the connection was closed
 * param[in] pContext The third argument passed to @ref platform_network_function_setclosecallback.
 */
void cy_ota_http_close_callback( IotNetworkConnection_t pConnection,
                                              IotNetworkCloseReason_t reason,
                                              void * pContext )
{
    cy_ota_context_t    *ctx = (cy_ota_context_t *)pContext;
    CY_OTA_CONTEXT_ASSERT(ctx);

    IotLogInfo("%s()", __func__);

    if (pConnection != NULL)
    {
        switch (reason)
        {
        case IOT_NETWORK_NOT_CLOSED:
            break;
        case IOT_NETWORK_SERVER_CLOSED:
        case IOT_NETWORK_TRANSPORT_FAILURE:
        case IOT_NETWORK_CLIENT_CLOSED:
        case IOT_NETWORK_UNKNOWN_CLOSED:
            cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DROPPED_US, 0);
        }
    }
}

/**
 * @brief Connect to OTA Update server
 *
 * NOTE: Individual Network Transport type will do whatever is necessary
 *      ex: MQTT
 *          - connect
 *          HTTP
 *          - connect
 *
 * @param[in]   ctx - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_MODULE_OTA_ERROR
 */
cy_rslt_t cy_ota_http_connect(cy_ota_context_t *ctx)
{
    IotNetworkError_t       err;

    IotLogInfo("%s()\n", __func__);
    CY_OTA_CONTEXT_ASSERT(ctx);

    if (ctx->u.http.connection != NULL)
    {
        IotLogError("%s() Already connected.\n", __func__);
        return CY_RSLT_MODULE_OTA_ERROR;
    }

    /* create the secure socket and connect to the server */
    err = IotNetworkSecureSockets_Create((IotNetworkServerInfo_t)&ctx->network_params.server,
                                         ctx->network_params.credentials,
                                         &ctx->u.http.connection);
    if (err != IOT_NETWORK_SUCCESS)
    {
        IotLogError("%s() socket create failed %d.\n", __func__, err);
        return CY_RSLT_MODULE_OTA_ERROR;
    }

    IotLogInfo("Connected to %s:%d\n", ctx->network_params.server.pHostName, ctx->network_params.server.port);

    /* set up receive data callback */
    err =  IotNetworkSecureSockets_SetReceiveCallback(ctx->u.http.connection, cy_ota_http_receive_callback, ctx);
    if (err != IOT_NETWORK_SUCCESS)
    {
        IotLogError("%s() SetReceiveCallback() failed %d.\n", __func__, err);
        return CY_RSLT_MODULE_OTA_ERROR;
    }

    /* set up socket close callback */
    err = IotNetworkSecureSockets_SetCloseCallback(ctx->u.http.connection, cy_ota_http_close_callback, ctx);
    if (err != IOT_NETWORK_SUCCESS)
    {
        IotLogError("%s() SetCloseCallback() failed %d.\n", __func__, err);
        return CY_RSLT_MODULE_OTA_ERROR;
    }

    return CY_RSLT_SUCCESS;
}

/**
 * @brief get the OTA download
 *
 * NOTE: Individual Network Transport type will do whatever is necessary
 *      ex: MQTT
 *          - subscribe to start data transfer
 *          HTTP
 *          - pull the data from the server
 *
 * @param[in]   ctx - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_MODULE_OTA_ERROR
 */
cy_rslt_t cy_ota_http_get(cy_ota_context_t *ctx)
{
    cy_rslt_t       result;
    char            *req_buffer;
    uint32_t        req_buff_len;
    uint32_t        bytes_sent;

    IotLogInfo("%s()\n", __func__);
    CY_OTA_CONTEXT_ASSERT(ctx);

    if (cy_rtos_init_mutex(&ctx->sub_callback_mutex) != CY_RSLT_SUCCESS)
    {
        IotLogWarn("%s() sub_callback_mutex init failed\n", __func__);
        return CY_RSLT_MODULE_OTA_GET_ERROR;
    }
    ctx->sub_callback_mutex_inited = 1;

    result = cy_rtos_init_timer(&ctx->u.http.http_timer, CY_TIMER_TYPE_ONCE,
                        cy_ota_http_timer_callback, (cy_timer_callback_arg_t)ctx);
    if (result != CY_RSLT_SUCCESS)
    {
        /* Event create failed */
        IotLogWarn( "%s() Timer Create Failed!\n", __func__);
        return CY_RSLT_MODULE_OTA_GET_ERROR;
    }

    /* Form GET request - re-use data buffer to save some RAM */
    req_buffer = (char *)ctx->u.http.data_buffer;
    memset(req_buffer, 0x00, OTA_HTTP_SIZE_OF_RECV_BUFFER);
    snprintf(req_buffer, OTA_HTTP_SIZE_OF_RECV_BUFFER, get_request_template,
            ctx->network_params.u.http.data_file, ctx->curr_server.pHostName, ctx->curr_server.port);
    req_buff_len = strlen(req_buffer);

    IotLogInfo("%s() IotNetworkSecureSockets_Send() 0x%x >%s< \n", __func__, req_buff_len, req_buffer);
    bytes_sent = IotNetworkSecureSockets_Send( ctx->u.http.connection, (uint8 *)req_buffer, req_buff_len);
    if (bytes_sent != req_buff_len)
    {
        IotLogError("%s() IotNetworkSecureSockets_Send( len:0x%x) sent 0x%lx\n", __func__, req_buff_len, bytes_sent);
        return CY_RSLT_MODULE_OTA_GET_ERROR;
    }

    while (1)
    {
        uint32_t    waitfor;

        /* get event */
        waitfor = OTA_EVENT_HTTP_EVENTS;
        result = cy_rtos_waitbits_event(&ctx->ota_event, &waitfor, 1, 0, CY_OTA_WAIT_HTTP_EVENTS_MS);
        IotLogInfo("%s() HTTP cy_rtos_waitbits_event: 0x%lx type:%d mod:0x%lx code:%d\n", __func__, waitfor, CY_RSLT_GET_TYPE(result), CY_RSLT_GET_MODULE(result), CY_RSLT_GET_CODE(result) );

        /* We only want to act on events we are waiting on.
         * For timeouts, just loop around.
         */
        if (waitfor == 0)
        {
            continue;
        }

        if (waitfor & OTA_EVENT_AGENT_SHUTDOWN_NOW)
        {
            /* Pass along to Agent thread */
            cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_AGENT_SHUTDOWN_NOW, 0);
            result = CY_RSLT_SUCCESS;
            break;
        }

        if (waitfor & OTA_EVENT_AGENT_DOWNLOAD_TIMEOUT)
        {
            /* This was generated by a timer in cy_ota_agent.c
             * Pass along to Agent thread.
             */
            result = CY_RSLT_MODULE_NO_UPDATE_AVAILABLE;
            break;
        }

        if (waitfor & OTA_EVENT_AGENT_STORAGE_ERROR)
        {
            cy_ota_internal_call_cb(ctx, CY_OTA_REASON_OTA_FLASH_WRITE_ERROR, 0);
            result = CY_RSLT_MODULE_OTA_WRITE_STORAGE_ERROR;
            break;
        }

        if (waitfor & OTA_EVENT_REDIRECT)
        {
            result = CY_RSLT_SUCCESS;
            break;
        }

        if (waitfor & OTA_EVENT_GOT_DATA)
        {
            if (ctx->packet_timeout_sec > 0 )
            {
                /* got some data - restart the download interval timer */
                IotLogDebug("%s() RESTART PACKET TIMER %ld secs\n", __func__, ctx->packet_timeout_sec);
                cy_ota_start_http_timer(ctx, ctx->packet_timeout_sec, OTA_EVENT_AGENT_PACKET_TIMEOUT);
            }

            if (ctx->total_image_size > 0)
            {
                uint32_t percent = (ctx->total_bytes_written * 100) / ctx->total_image_size;
                IotLogInfo("Percent written:%d", percent);
                cy_ota_internal_call_cb(ctx, CY_OTA_REASON_DOWNLOAD_PERCENT, percent);
            }

            if (ctx->total_bytes_written >= ctx->total_image_size)
            {
                IotLogDebug("Done writing all data! %ld of %ld\n", ctx->total_bytes_written, ctx->total_image_size);
                cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DATA_DONE, 0);
                /* stop timer asap */
                cy_ota_stop_http_timer(ctx);
            }
        }

        if (waitfor & OTA_EVENT_AGENT_PACKET_TIMEOUT)
        {
            /* We set a timer and if packets take too long, we will assume the broker forgot about us.
             * Set with CY_OTA_PACKET_INTERVAL_SECS.
             */
            if (ctx->num_packets_received > ctx->last_num_packets_received)
            {
                /* If we received packets since the last time we were here, just continue.
                 * This thread may be held off for a while, and we don't want a false failure.
                 */
                IotLogDebug("%s() RESTART PACKET TIMER %ld secs\n", __func__, ctx->packet_timeout_sec);
                cy_ota_start_http_timer(ctx, ctx->packet_timeout_sec, OTA_EVENT_AGENT_PACKET_TIMEOUT);

                /* update our variable */
                ctx->last_num_packets_received = ctx->num_packets_received;

                continue;
            }
            IotLogWarn("OTA Timeout waiting for a packet (%d seconds), fail\n", ctx->packet_timeout_sec);
            cy_ota_internal_call_cb(ctx, CY_OTA_REASON_DOWNLOAD_FAILED, 0);
            cy_rtos_setbits_event(&ctx->ota_event, OTA_EVENT_DATA_FAIL, 0);
        }

        if (waitfor & OTA_EVENT_DATA_DONE)
        {
            result = CY_RSLT_SUCCESS;
            break;
        }

        if (waitfor & OTA_EVENT_INVALID_VERSION)
        {
            result = CY_RSLT_MODULE_OTA_INVALID_VERSION;
            break;
        }

        if (waitfor & OTA_EVENT_DATA_FAIL)
        {
            result = CY_RSLT_MODULE_OTA_GET_ERROR;
            break;
        }

        if (waitfor & OTA_EVENT_DROPPED_US)
        {
            result = CY_RSLT_MODULE_OTA_SERVER_DROPPED_CNCT;
            break;
        }
    }   /* While 1 */

    ctx->sub_callback_mutex_inited = 0;
    cy_rtos_deinit_mutex(&ctx->sub_callback_mutex);

    /* we completed the download, stop the timer */
    cy_ota_stop_http_timer(ctx);
    cy_rtos_deinit_timer(&ctx->u.http.http_timer);

    IotLogDebug("%s() HTTP DONE result: 0x%lx\n", __func__, result);

    return result;
}

/**
 * @brief Open Storage area for download
 *
 * NOTE: Typically, this erases Secondary Slot
 *
 * @param[in]   ctx - pointer to OTA agent context @ref cy_ota_context_t
 *
 * @return  CY_RSLT_SUCCESS
 *          CY_RSLT_MODULE_OTA_ERROR
 */
cy_rslt_t cy_ota_http_disconnect(cy_ota_context_t *ctx)
{
    cy_rslt_t           result = CY_RSLT_SUCCESS;
    IotNetworkError_t   err = IOT_NETWORK_SUCCESS;

    IotLogInfo("%s()\n", __func__);
    CY_OTA_CONTEXT_ASSERT(ctx);

    IotNetworkConnection_t  old_conn;
    old_conn = ctx->u.http.connection;
    ctx->u.http.connection = NULL;

    /* Only disconnect if we had connected before */
    if (old_conn != NULL)
    {
        err = IotNetworkSecureSockets_Close(old_conn);
        if (err != IOT_NETWORK_SUCCESS)
        {
            IotLogError("%s() IotNetworkSecureSockets_Close() returned Error %d", __func__, result);
            result = CY_RSLT_MODULE_OTA_DISCONNECT_ERROR;
        }

        err = IotNetworkSecureSockets_Destroy(old_conn);
        if (err != IOT_NETWORK_SUCCESS)
        {
            IotLogError("%s() IotNetworkSecureSockets_Destroy() returned Error %d", __func__, result);
            result = CY_RSLT_MODULE_OTA_DISCONNECT_ERROR;
        }
    }

    return result;
}
