
#ifdef CY_DOXYGEN

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

#include "iot_init.h"
#include "cy_iot_network_secured_socket.h"
#include "iot_mqtt.h"
#include "iot_mqtt_types.h"
#include "iot_network.h"
#include "cy_wcm.h"

/* OTA API */
#include "cy_ota_api.h"



/* Initialze the cy_ota_network_params_t and ota_agent_params structures */

/* The following code snip demonstrates an example for initializing the cy_ota_network_params_t and ota_agent_params structures required to start the OTA agent. */

/*! [snippet_cy_ota_structure_initialisation] */
/* Macro to enable/disable TLS */
#define ENABLE_TLS          (false)

/* Number of MQTT topic filters */
#define MQTT_TOPIC_FILTER_NUM   (1)

/* MQTT topics */
const char * mqtt_topics[ MQTT_TOPIC_FILTER_NUM ] =
{
        "anycloud/test/ota/image"
};

/* Root CA Certificate -
   Must include the PEM header and footer:

        "-----BEGIN CERTIFICATE-----\n" \
        ".........base64 data.......\n" \
        "-----END CERTIFICATE-------\n"
*/
#define ROOT_CA_CERTIFICATE     ""

/* Client Certificate
   Must include the PEM header and footer:

        "-----BEGIN CERTIFICATE-----\n" \
        ".........base64 data.......\n" \
        "-----END CERTIFICATE-------\n"
*/
#define CLIENT_CERTIFICATE      ""


/* Private Key
   Must include the PEM header and footer:

        "-----BEGIN RSA PRIVATE KEY-----\n" \
        "...........base64 data.........\n" \
        "-----END RSA PRIVATE KEY-------\n"
*/
#define CLIENT_KEY              ""

/* OTA context */
static cy_ota_context_ptr ota_context;

void ota_callback(cy_ota_cb_reason_t reason, uint32_t value, void *cb_arg );

/* MQTT Credentials for OTA */
struct IotNetworkCredentials tls_credentials =
{
    .pRootCa = ROOT_CA_CERTIFICATE,
    .rootCaSize = sizeof(ROOT_CA_CERTIFICATE),
    .pClientCert = CLIENT_CERTIFICATE,
    .clientCertSize = sizeof(CLIENT_CERTIFICATE),
    .pPrivateKey = CLIENT_KEY,
    .privateKeySize = sizeof(CLIENT_KEY),
};

/* Network parameters for OTA */
cy_ota_network_params_t ota_network_params =
{
    .server =
    {
        .pHostName = "test.mosquitto.org",  /* MQTT Broker endpoint */
#if (ENABLE_TLS == true)
        .port = 8884                        /* MQTT Server Port */
#else
        .port = 1883
#endif
    },
    .transport = CY_OTA_TRANSPORT_MQTT,
    .u.mqtt =
    {
        .pTopicFilters = mqtt_topics,
        .numTopicFilters = MQTT_TOPIC_FILTER_NUM,
        .pIdentifier = "CY_IOT_DEVICE",     /* MQTT identifier - less than 17 characters */
        .awsIotMqttMode = 0                 /* This parameter must be 1 when using the AWS IoT MQTT server, 0 otherwise */
    },
#if (ENABLE_TLS == true)
    .credentials = &tls_credentials
#else
    .credentials = NULL
#endif
};

/* Parameters for OTA agent */
cy_ota_agent_params_t ota_agent_params =
{
    .cb_func = ota_callback,                /* Set value to NULL if callback function is not used */
    .cb_arg = &ota_context,                 /* Set value to NULL if callback function is not used */
    .reboot_upon_completion = 1
};

/*! [snippet_cy_ota_structure_initialisation] */


/* The example function demonstrates the usage of OTA callback function to print the status of every OTA event. The callback feature is optional but be aware that the OTA middleware will not print the status of the OTA agent on its own. */
/* Note: User needs to initialize retarget-io to use the debug UART port. */

/*! [snippet_cy_ota_setup_callback] */
/* Setup callback function to print OTA status */
void ota_callback(cy_ota_cb_reason_t reason, uint32_t value, void *cb_arg )
{
    cy_ota_agent_state_t ota_state;
    cy_ota_context_ptr ctx = *((cy_ota_context_ptr *)cb_arg);
    cy_ota_get_state(ctx, &ota_state);
    printf("Application OTA callback ctx:%p reason:%d %s value:%ld state:%d %s %s\n",
            ctx,
            reason,
            cy_ota_get_callback_reason_string(reason),
            value,
            ota_state,
            cy_ota_get_state_string(ota_state),
            cy_ota_get_error_string(cy_ota_last_error()));
}

/*! [snippet_cy_ota_setup_callback] */




/* The following code snip demonstrates the initialization of OTA agent */

/*! [snippet_cy_ota_setup] */
/* Initialze and start OTA agent */

void ota_setup(void)
{
    /*****************************************************************************
    < Add code snippet here to initialze and connect to a Wi-Fi AP >
    ******************************************************************************/

    /* Initialize the underlying support code that is needed for OTA and MQTT */
    IotSdk_Init();

    /* Call the Network Secured Sockets initialization function. */
    IotNetworkSecureSockets_Init();

    /* Initialize the MQTT subsystem */
    IotMqtt_Init();

    /* Add the network interface to the OTA network parameters */
    ota_network_params.network_interface = (void *)IOT_NETWORK_INTERFACE_CY_SECURE_SOCKETS;

    /* Initialize and start the OTA agent */
    cy_ota_agent_start(&ota_network_params, &ota_agent_params, &ota_context);
}
/*! [snippet_cy_ota_setup] */

#endif
