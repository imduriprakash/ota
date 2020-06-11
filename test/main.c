/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for OTA Update Example in
* ModusToolbox.
*
* Related Document: See Readme.md
*
*******************************************************************************
* (c) 2019, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

/* Comment  /uncomment these defines to set the MQTT or HTTP
 * transport and TLS usage. The Broker/Server URL and certs/keys
 * are set in test_http_config.h or test_mqtt_config.h.
 */
#define USE_MQTT_FOR_TRANSPORT  1
//#define USE_HTTP_FOR_TRANSPORT  1

/* Only enable one of the MQTT Brokers
 * NOTES:
 * - Amazon only works with TLS
 * - Eclipse only works without TLS
 * - Mosquitto works with/without TLS
 */
//#define AMAZON   1
//#define ECLIPSE   1
#define MOSQUITTO   1

/* Uncomment to use TLS over MQTT for OTA_SUPPORT */
#define OTA_USE_TLS    1


/* Header file includes */
#include <string.h>
#include "cyhal.h"
#include "cybsp.h"
#include "cybsp_wifi.h"
#include "cy_retarget_io.h"
#include "cyabs_rtos.h"

/* FreeRTOS header file */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

/* lwIP header files */
#include <lwip/tcpip.h>
#include <lwip/api.h>
#include <cy_lwip.h>
#include "ip4_addr.h"

/* for Iot SDK, Secure Sockets, and MQTT initialization */
#include "iot_init.h"
#include "cy_iot_network_secured_socket.h"
#include "iot_mqtt.h"
#include "iot_mqtt_types.h"
#include "iot_network.h"
#include "cy_wcm.h"

/* OTA API */
#include "cy_ota_api.h"

/* test keys, etc */
#include "test_mqtt_config.h"
#include "test_http_config.h"

/* Enable this define to have the application connect to the MQTT server and pass the
 * MQTT connection info to the OTA Agent.
 */
//#define APPLICATION_CONNECTS_TO_MQTT    1


/*******************************************************************************
* Macros
********************************************************************************/

#define WIFI_SSID                           "MY-WIFI-SSID"
#define WIFI_PASSWORD                       "MY-PASSWORD"
#define WIFI_SECURITY                       WHD_SECURITY_WPA2_AES_PSK

/* MAX connection retries to join WI-FI AP */
#define MAX_CONNECTION_RETRIES              (10u)

/* Wait between connection retries */
#define WIFI_CONN_RETRY_DELAY_MS            (500)

#if (APP_VERSION_MAJOR == 1)
#define BLINKY_DELAY                        (1000)          // 1 second on/off flash
#else
#define BLINKY_DELAY                        (250)          // 1/4 second on/off flash
#endif

#define ota_example_task_STACK_SIZE         (1024 * 6)
#define ota_example_task_PRIORITY           (CY_RTOS_PRIORITY_BELOWNORMAL)

/* wait between checking for an IP address */
#define RTOS_TASK_TICKS_TO_WAIT             (100)

/* Name of the update JSON file for HTTP - NOT IMPLEMENTED YET */
#define OTA_JOB_FILE                        "update_job.json"

/* name of the OTA Image */
#define OTA_DATA_FILE                       "/anycloud-ota.bin"

/* Server Port for OTA
 * HTTP:        80
 * from https://test.mosquitto.org/
 * HTTP:        8080 WebSockets, unencrypted
 * Secure HTTP: 8081 WebSockets, encrypted
 * MQTT:        1883 unencrypted
 * secure MQTT: 8883 encrypted
 * secure MQTT: 8884 encrypted, client certificate required
 */
#define OTA_HTTP_SERVER_PORT                (80)    /* or 8080 */
#define OTA_HTTP_SECURE_SERVER_PORT         (8081)
#define OTA_MQTT_SERVER_PORT                (1883)
#define OTA_MQTT_SECURE_SERVER_PORT         (8883)
#define OTA_MQTT_SECURE_CERT_SERVER_PORT    (8884)

/* MQTT identifier */
#define OTA_MQTT_ID                         "CY8CP_062_4343W"       /* less than 17 */

/* MQTT topics */
const char * my_topics[ CY_OTA_MQTT_MAX_TOPICS ] =
{
        "anycloud/ota/image"
};

/**
 * @brief MQTT Credentials for OTA
 */
struct IotNetworkCredentials    credentials = { 0 };


/**
 * @brief network parameters for OTA
 */
cy_ota_network_params_t     ota_test_network_params = { 0 };

/**
 * @brief aAgent parameters for OTA
 */
cy_ota_agent_params_t     ota_test_agent_params = { 0 };

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void ota_example_task(void *arg);
void init_ota(void);
static void ota_callback(cy_ota_cb_reason_t reason, uint32_t value, void *cb_arg );

/*******************************************************************************
* Global Data and Variables
********************************************************************************/

/**
 * @brief blinky LED state
 */
uint32_t blinky_led_state;

/**
 * @brief OTA example main task handle.
 */
TaskHandle_t ota_example_task_handle;

/**
 * @brief The primary WIFI driver.
 */
whd_interface_t whd_iface;

/**
 * @brief  forward declaration for OTA callback function
 */
static void ota_callback(cy_ota_cb_reason_t reason, uint32_t value, void *cb_arg );

/**
 * @brief OTA context
 * */
cy_ota_context_ptr ota_context;

/* This enables RTOS aware debugging. */
volatile int uxTopUsedPriority;

#ifdef APPLICATION_CONNECTS_TO_MQTT

#define CLIENT_IDENTIFIER_MAX_LENGTH                ( 24 )
#define UINT16_DECIMAL_LENGTH                       ( 5 )
#define WILL_TOPIC_NAME                             IOT_MQTT_TOPIC_PREFIX "/will"
#define WILL_TOPIC_NAME_LENGTH                      ( ( uint16_t ) ( sizeof( WILL_TOPIC_NAME ) - 1 ) )
#define WILL_MESSAGE                                "MQTT demo unexpectedly disconnected."
#define WILL_MESSAGE_LENGTH                         ( ( size_t ) ( sizeof( WILL_MESSAGE ) - 1 ) )


static IotMqttConnection_t   ota_test_mqtt_connection;



static void ota_test_disconnect_callback( void * pCallbackContext,
                                          IotMqttCallbackParam_t * pCallbackParam )
{
    if ((pCallbackContext == NULL) || (pCallbackParam == NULL))
    {
        /* bad callback - no info! */
        return;
    }
    printf( "MQTT Network disconnect callback.\n" );

    if( pCallbackParam->u.disconnectReason == IOT_MQTT_KEEP_ALIVE_TIMEOUT )
    {
        printf( "Network disconnected.\n" );
        if (pCallbackParam->mqttConnection != NULL)
        {
            printf( "Shutting down OTA.\n" );
            cy_ota_agent_stop(&ota_context);
        }
    }
}

/**
 * @brief Establish a new connection to the MQTT server.
 *
 * @param[in] awsIotMqttMode Specify if this demo is running with the AWS IoT
 * MQTT server. Set this to `false` if using another MQTT server.
 * @param[in] pIdentifier NULL-terminated MQTT client identifier.
 * @param[in] pNetworkServerInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkCredentialInfo Passed to the MQTT connect function when
 * establishing the MQTT connection.
 * @param[in] pNetworkInterface The network interface to use for this demo.
 * @param[out] pMqttConnection Set to the handle to the new MQTT connection.
 *
 * @return `EXIT_SUCCESS` if the connection is successfully established; `EXIT_FAILURE`
 * otherwise.
 */
static cy_rslt_t ota_test_MQTT_connect(     bool awsIotMqttMode,
                                     char * pIdentifier,
                                     void * pNetworkServerInfo,
                                     void * pNetworkCredentialInfo,
                                     const IotNetworkInterface_t * pNetworkInterface,
                                     IotMqttConnection_t * pMqttConnection,
                                     void *app_context,
                                     bool clean)
{
    cy_time_t tval;
    int status = EXIT_SUCCESS;
    IotMqttError_t connectStatus = IOT_MQTT_STATUS_PENDING;
    IotMqttNetworkInfo_t networkInfo = IOT_MQTT_NETWORK_INFO_INITIALIZER;
    IotMqttConnectInfo_t connectInfo = IOT_MQTT_CONNECT_INFO_INITIALIZER;
    IotMqttPublishInfo_t willInfo = IOT_MQTT_PUBLISH_INFO_INITIALIZER;
    char temp_identifier[CLIENT_IDENTIFIER_MAX_LENGTH ] = { 0 };
    char pClientIdentifierBuffer[ CLIENT_IDENTIFIER_MAX_LENGTH ] = { 0 };

    /* Set the members of the network info not set by the initializer. This
     * struct provides information on the transport layer to the MQTT connection. */
    networkInfo.createNetworkConnection = true;
    networkInfo.u.setup.pNetworkServerInfo = pNetworkServerInfo;
    networkInfo.u.setup.pNetworkCredentialInfo = pNetworkCredentialInfo;
    networkInfo.pNetworkInterface = pNetworkInterface;
    networkInfo.disconnectCallback.function = ota_test_disconnect_callback;
    networkInfo.disconnectCallback.pCallbackContext = app_context;

    #if ( IOT_MQTT_ENABLE_SERIALIZER_OVERRIDES == 1 ) && defined( IOT_DEMO_MQTT_SERIALIZER )
        networkInfo.pMqttSerializer = IOT_DEMO_MQTT_SERIALIZER;
    #endif

    /* Set the members of the connection info not set by the initializer. */
    connectInfo.awsIotMqttMode = awsIotMqttMode;
    connectInfo.cleanSession = clean;
    connectInfo.keepAliveSeconds = MQTT_KEEP_ALIVE_SECONDS;
    connectInfo.pWillInfo = &willInfo;

    /* Set the members of the Last Will and Testament (LWT) message info. The
     * MQTT server will publish the LWT message if this client disconnects
     * unexpectedly. */
    willInfo.pTopicName = WILL_TOPIC_NAME;
    willInfo.topicNameLength = WILL_TOPIC_NAME_LENGTH;
    willInfo.pPayload = WILL_MESSAGE;
    willInfo.payloadLength = WILL_MESSAGE_LENGTH;

    /* Use the parameter client identifier if provided. Otherwise, generate a
     * unique client identifier. */
    cy_rtos_get_time(&tval);
    memset(temp_identifier, 0x00, sizeof(temp_identifier));
    if( ( pIdentifier == NULL ) || ( pIdentifier[ 0 ] == '\0' ) )
    {
        strncpy(temp_identifier, CLIENT_IDENTIFIER_PREFIX, CLIENT_IDENTIFIER_MAX_LENGTH - UINT16_DECIMAL_LENGTH);
    }
    else
    {
        strncpy(temp_identifier, pIdentifier, CLIENT_IDENTIFIER_MAX_LENGTH - UINT16_DECIMAL_LENGTH);
    }

    /* Every active MQTT connection must have a unique client identifier. The demos
     * generate this unique client identifier by appending a timestamp to a common
     * prefix. */
    status = snprintf( pClientIdentifierBuffer,
                       CLIENT_IDENTIFIER_MAX_LENGTH,
                       "%s%d", temp_identifier, (uint16_t)(tval & 0xFFFF) );

    /* Check for errors from snprintf. */
    if( status >=  CLIENT_IDENTIFIER_MAX_LENGTH)
    {
        printf( "Failed to generate unique MQTT client identifier. Using partial");
        status = snprintf( pClientIdentifierBuffer,
                           CLIENT_IDENTIFIER_MAX_LENGTH,
                           "Unique%d", (uint16_t)(tval & 0xFFFF) );
        if( status >=  CLIENT_IDENTIFIER_MAX_LENGTH)
        {
            printf( "Failed to generate unique MQTT client identifier. Fail");
            status = EXIT_FAILURE;
        }
    }
    else
    {
        status = EXIT_SUCCESS;
    }

    /* Set the client identifier buffer and length. */
    connectInfo.pClientIdentifier = pClientIdentifierBuffer;
    connectInfo.clientIdentifierLength = ( uint16_t ) strlen(pClientIdentifierBuffer);
    if ( networkInfo.u.setup.pNetworkCredentialInfo != NULL)
    {
        connectInfo.pUserName = networkInfo.u.setup.pNetworkCredentialInfo->pUserName;
        connectInfo.userNameLength = networkInfo.u.setup.pNetworkCredentialInfo->userNameSize;
        connectInfo.pPassword = networkInfo.u.setup.pNetworkCredentialInfo->pPassword;
        connectInfo.passwordLength = networkInfo.u.setup.pNetworkCredentialInfo->passwordSize;
    }
#ifdef PRINT_CONECT_INFO
    _print_connect_info(&connectInfo);
#endif
#ifdef PRINT_NETWORK_INFO
    _print_network_info(&networkInfo);
#endif

    /* Establish the MQTT connection. */
    if( status == EXIT_SUCCESS )
    {
        printf( "Application MQTT unique client identifier is %.*s (length %hu).\n",
                    connectInfo.clientIdentifierLength,
                    connectInfo.pClientIdentifier,
                    connectInfo.clientIdentifierLength);

        connectStatus = IotMqtt_Connect( &networkInfo,
                                         &connectInfo,
                                         MQTT_TIMEOUT_MS,
                                         pMqttConnection);

        if( connectStatus != IOT_MQTT_SUCCESS )
        {
            printf( "MQTT CONNECT returned error %s.\n",
                         IotMqtt_strerror( connectStatus ) );

            status = 1;
        }
    }

    return (status == EXIT_SUCCESS) ? CY_RSLT_SUCCESS : CY_RSLT_MODULE_OTA_ERROR;
}
#endif  /* APPLICATION_CONNECTS_TO_MQTT */

/*******************************************************************************
 * Function Name: main
 ********************************************************************************
 * Summary:
 *  System entrance point. This function sets up user tasks and then starts
 *  the RTOS scheduler.
 *
 * Parameters:
 *  void
 *
 * Return:
 *  int
 *
 *******************************************************************************/
int main()
{
    cy_rslt_t result ;

    /* This enables RTOS aware debugging in OpenOCD. */
    uxTopUsedPriority = configMAX_PRIORITIES - 1 ;

    /* Initialize the board support package. */
    result = cybsp_init() ;
    CY_ASSERT(result == CY_RSLT_SUCCESS) ;

     /* Initialize the User LED. */
    cyhal_gpio_init((cyhal_gpio_t) CYBSP_USER_LED, CYHAL_GPIO_DIR_OUTPUT,
                    CYHAL_GPIO_DRIVE_PULLUP, CYBSP_LED_STATE_OFF);

    /* Enable global interrupts. */
    __enable_irq();

    /* Initialize retarget-io to use the debug UART port. */
    cy_retarget_io_init(CYBSP_DEBUG_UART_TX, CYBSP_DEBUG_UART_RX,
                        CY_RETARGET_IO_BAUDRATE);

    /* Initialize the User LED */
    cyhal_gpio_init((cyhal_gpio_t) CYBSP_USER_LED1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, CYBSP_LED_STATE_OFF);

    printf("===============================================================\n");
    printf("TEST Application: OTA Update version: %d.%d.%d\n", APP_VERSION_MAJOR,APP_VERSION_MINOR, APP_VERSION_BUILD);
    printf("configTOTAL_HEAP_SIZE : 0x%x\n", configTOTAL_HEAP_SIZE);
    printf("===============================================================\n\n");

    /* Create the tasks. */
    xTaskCreate(ota_example_task, "OTA Test", ota_example_task_STACK_SIZE, NULL,
            ota_example_task_PRIORITY, &ota_example_task_handle);

    /* Start the FreeRTOS scheduler. */
    vTaskStartScheduler();

    /* Should never get here. */
    CY_ASSERT(0);
}

/*******************************************************************************
 * Function Name: ota_example_task
 *******************************************************************************
 * Summary:
 *  Task used to establish a connection to a remote TCP client.
 *
 * Parameters:
 *  void *args : Task parameter defined during task creation (unused)
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void ota_example_task(void *arg)
{
    cy_wcm_config_t wifi_config = { .interface = CY_WCM_INTERFACE_TYPE_STA};
    cy_wcm_connect_params_t wifi_conn_param;
    cy_wcm_ip_address_t ip_address;
    cy_rslt_t result;

    /* Variable to track the number of connection retries to the Wi-Fi AP specified
     * by WIFI_SSID macro. */
    uint32_t conn_retries = 0;

    /* Initialize Wi-Fi connection manager. */
    cy_wcm_init(&wifi_config);

     /* Set the Wi-Fi SSID, password and security type. */
    memset(&wifi_conn_param, 0, sizeof(cy_wcm_connect_params_t));
    memcpy(wifi_conn_param.ap_credentials.SSID, WIFI_SSID, sizeof(WIFI_SSID));
    memcpy(wifi_conn_param.ap_credentials.password, WIFI_PASSWORD, sizeof(WIFI_PASSWORD));
    wifi_conn_param.ap_credentials.security = WIFI_SECURITY;

    /* Connect to the Wi-Fi AP */
    for(conn_retries = 0; conn_retries < MAX_CONNECTION_RETRIES; conn_retries++)
    {
        result = cy_wcm_connect_ap( &wifi_conn_param, &ip_address );

        if (result == CY_RSLT_SUCCESS)
        {
            printf( "Successfully connected to Wi-Fi network '%s'.\n",
                    wifi_conn_param.ap_credentials.SSID);
            break;
        }

        printf( "Connection to Wi-Fi network failed with error code %d."
                "Retrying in %d ms...\n", (int) result, WIFI_CONN_RETRY_DELAY_MS );
        vTaskDelay(pdMS_TO_TICKS(WIFI_CONN_RETRY_DELAY_MS));
    }

    if (result != CY_RSLT_SUCCESS)
    {
        printf( "Exceeded maximum Wi-Fi connection attempts\n" );
        CY_ASSERT(0);
    }


    /* Initialize underlying support code that is needed for OTA and MQTT */
    if (IotSdk_Init() != 1)
    {
        printf("\nIotSdk_Init Failed.\n");
        while(true)
        {
            cy_rtos_delay_milliseconds(10);
        }
    }

    /* Call the Network Secured Sockets initialization function. */
    IotNetworkError_t networkInitStatus = IOT_NETWORK_SUCCESS;
    networkInitStatus = IotNetworkSecureSockets_Init();
    if( networkInitStatus != IOT_NETWORK_SUCCESS )
    {
        printf("\nIotNetworkSecureSockets_Init Failed.\n");
        while(true)
        {
            cy_rtos_delay_milliseconds(10);
        }
    }

    /* And initialize the MQTT subsystem */
    if( IotMqtt_Init() != IOT_MQTT_SUCCESS )
    {
        printf("\nIotMqtt_Init Failed.\n");
        while(true)
        {
            cy_rtos_delay_milliseconds(10);
        }
    }

    /* start the OTA agent */
    init_ota();

    blinky_led_state = 0;
    while(true)
    {
        cy_rtos_delay_milliseconds(BLINKY_DELAY);

        blinky_led_state = !blinky_led_state;
        if (blinky_led_state)
        {
            cyhal_gpio_write((cyhal_gpio_t) CYBSP_USER_LED, CYBSP_LED_STATE_OFF);
        }
        else
        {
            cyhal_gpio_write((cyhal_gpio_t) CYBSP_USER_LED, CYBSP_LED_STATE_ON);
        }
    }
 }

/*******************************************************************************
 * Function Name: ota_callback()
 *******************************************************************************
 * Summary:
 *  Got a callback from OTA
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
static void ota_callback(cy_ota_cb_reason_t reason, uint32_t value, void *cb_arg )
{
    cy_ota_agent_state_t ota_state;
    cy_ota_context_ptr ctx = *((cy_ota_context_ptr *)cb_arg);
    cy_ota_get_state(ctx, &ota_state);
    printf("Application OTA callback ctx:%p reason:%d %s value:%ld state:%d %s  %s\n", ctx,
            reason, cy_ota_get_callback_reason_string(reason), value,
            ota_state, cy_ota_get_state_string(ota_state),
            cy_ota_get_error_string(cy_ota_last_error()));

}

/*******************************************************************************
 * Function Name: init_ota()
 *******************************************************************************
 * Summary:
 *  Initialize and start the OTA update
 *
 * Parameters:
 *  void
 *
 * Return:
 *  void
 *
 *******************************************************************************/
void init_ota(void)
{
    cy_rslt_t                       result;

    /* commoon */
    /* Server parameters */
    ota_test_network_params.network_interface = (void *)IOT_NETWORK_INTERFACE_CY_SECURE_SOCKETS;
    ota_test_network_params.server.pHostName = OTA_SERVER_URL;      /* must not be a local variable */
    ota_test_network_params.server.port = OTA_SERVER_PORT;
    ota_test_network_params.credentials = NULL;
    ota_test_network_params.u.mqtt.awsIotMqttMode = 0;      /* Special Amazon mode.
                                                             * Not necessary for normal MQTT operation
                                                             */

    /* credentials settings */
    credentials.pAlpnProtos = NULL;
    credentials.maxFragmentLength = 0;
    credentials.disableSni = 0;

#if USE_MQTT_FOR_TRANSPORT
    ota_test_network_params.transport = CY_OTA_TRANSPORT_MQTT;
    ota_test_network_params.u.mqtt.numTopicFilters = 1;
    ota_test_network_params.u.mqtt.pTopicFilters = my_topics;
    ota_test_network_params.u.mqtt.pIdentifier = OTA_MQTT_ID;
    ota_test_network_params.u.mqtt.session_type = CY_OTA_MQTT_SESSION_CLEAN;
#ifdef OTA_USE_TLS
    /* Set up the credentials information */
#ifdef AMAZON
    /* Only set this for Amazon testing */
    credentials.pUserName = "Test";
    credentials.userNameSize = strlen("Test");
    credentials.pPassword = "";
    credentials.passwordSize = 0;
#endif
    credentials.pAlpnProtos = NULL;
    credentials.maxFragmentLength = 0;
    credentials.disableSni = 0;
    credentials.pRootCa = (const char *) &root_ca_certificate;
    credentials.rootCaSize = sizeof(root_ca_certificate);
    credentials.pClientCert = (const char *) &client_cert;
    credentials.clientCertSize = sizeof(client_cert);
    credentials.pPrivateKey = (const char *) &client_key;
    credentials.privateKeySize = sizeof(client_key);

    ota_test_network_params.credentials = &credentials;
#endif  /* Use TLS */
#endif  /* USE_MQTT_FOR_TRANSPORT   */

#ifdef USE_HTTP_FOR_TRANSPORT  /* Set to 1 for HTTP transport */
    ota_test_network_params.transport = CY_OTA_TRANSPORT_HTTP;
    /* For HTTP server to get "job" file - NOT IMPLEMENTED YET */
    ota_test_network_params.u.http.job_file = OTA_JOB_FILE;
    ota_test_network_params.u.http.data_file = OTA_DATA_FILE;
    ota_test_network_params.network_interface = (void *)IOT_NETWORK_INTERFACE_CY_SECURE_SOCKETS;

#ifdef OTA_USE_TLS
    credentials.pRootCa = (const char *) &root_ca_certificate;
    credentials.rootCaSize = sizeof(root_ca_certificate);
    credentials.pClientCert = (const char *) &client_cert;
    credentials.clientCertSize = sizeof(client_cert);
    credentials.pPrivateKey = (const char *) &client_key;
    credentials.privateKeySize = sizeof(client_key);
    ota_test_network_params.credentials = &credentials;
#endif  /* USE TLS */
#endif  /* USE HTTP */

    /* OTA Agent parameters */
    ota_test_agent_params.validate_after_reboot = 0;        /* Validate after download so we don't have to call
                                                                cy_ota_validated() on reboot */
    ota_test_agent_params.reboot_upon_completion = 1;       /* 1 = reboot when download is finished                  */
    ota_test_agent_params.cb_func = ota_callback;
    ota_test_agent_params.cb_arg = &ota_context;

#ifdef APPLICATION_CONNECTS_TO_MQTT
    ota_test_mqtt_connection = NULL;
    if (CY_RSLT_SUCCESS == ota_test_MQTT_connect(    ota_test_network_params.u.mqtt.awsIotMqttMode,
                                       (char *)ota_test_network_params.u.mqtt.pIdentifier,
                                       &ota_test_network_params.server,
                                       ota_test_network_params.credentials,
                                       ota_test_network_params.network_interface,
                                       &ota_test_mqtt_connection,
                                       NULL,
                                       true) )
    {
        ota_test_network_params.u.mqtt.app_mqtt_connection = ota_test_mqtt_connection;
    }

#endif

    result = cy_ota_agent_start(&ota_test_network_params, &ota_test_agent_params, &ota_context);
    if (result != CY_RSLT_SUCCESS)
    {
        printf("cy_ota_agent_start() Failed - result: 0x%lx\n", result);
        while(true)
        {
            cy_rtos_delay_milliseconds(10);
        }
    }
}
/* [] END OF FILE */
