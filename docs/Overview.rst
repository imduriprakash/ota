
************
Getting Started
************

Overview
==================
 This library provides support for OTA updating using WiFi of the application code running on a PSoC 6 MCU with CYW4343W or CYW43012 connectivity device.

 In this example, device establishes connection with designated MQTT Broker. Once the connection completes successfully, the device subscribes to a topic.
 The topic is defined by the user and passed as a parameter when starting the OTA Agent.

 When an update is available, the customer will publish the update to the MQTT broker. The device receives and saves the update to the Secondary slot (slot1) in FLASH.
 On the next reboot, MCUBoot bootloader will copy the new version of the application over to the Primary slot (slot 0) and run the application.

 The ModusToolbox OTA code examples import this library automatically.

Features and functionality
==================
This library utilizes MQTT and TLS to securely connect to an MQTT Broker and download an update for the users application.

Other features:

Configuration to adjust multiple timing values to customize how often to check for updates, and other parameters for the MQTT Broker connection.
Runs in a separate background thread, only connecting to MQTT broker based on the application configuration.
 Run-time parameters for MQTT broker, credentials, and other parameters.
Provides a callback mechanism to report stages of connect, download %, and errors.

Once the application starts the OTA agent, the OTA agent will contact the MQTT broker at the defined intervals to see if there is an update available.

Integration notes
==================
A pre-defined configuration file has been bundled with this library.\n

The user is expected to:\n

 Copy the cy_ota_config.h file from the libs/anycloud-ota/configs\n
 directory to the top-level code example directory.\n

 Add the following to COMPONENTS in the code example project's Makefile\n
 FREERTOS, PSOC6HAL, LWIP, MBEDTLS and either 4343W or 43012 depending on the platform.\n
      For instance, if [CY8CKIT-062](https://jira.cypress.com/browse/CY8CKIT-062)S2-43012 is\n      chosen, then the Makefile entry would look like:\n
code COMPONENTS=FREERTOS PSOC6HAL LWIP MBEDTLS 43012 \endcode

General Concept
==================

 The OTA Agent stores the update to the Secondary Slot and uses MCUBoot to\n
   validate and copy the update from the Secondary Slot to the Primary Slot.\n

  MCUBoot runs the current application in Primary Slot.\n
  The OTA Agent downloads the update and stores it in the Secondary Slot.\n
  The Secondary Slot is marked so that MCUBoot will copy it over to the Primary Slot.\n
  If validate_after_reboot == 0 then Secondary Slot will be tagged as "perm"\n
  If validate_after_reboot == 1 then Secondary Slot will be tagged as "test"\n
  If reboot_upon_completion == 1, the system will reboot automatically.\n
  On the next system reboot, MCUBoot sees that there is an update in the Secondary Slot.\n
     MCUBoot then copies the update from the Secondary Slot to the Primary Slot.
  MCUBoot runs update in Primary Slot.
  If validate_after_reboot == 1 then the update must validate itself and call cy_ota_validated()\n
     to inform MCUBoot that it is now "perm" to complete the process.

API Overview
==================

Start The OTA Agent
-------------------
 `cy_ota_start_agent()` is a non-blocking call, which returns a context pointer that is used in subsequent calls.\n

 The OTA Agent uses callbacks to signal application when events occur.\n
 \n
 `cy_rslt_t cy_ota_agent_start(cy_ota_network_params_t *network_params, cy_ota_agent_params_t agent_params, cy_ota_context_ptr *ota_ptr);`

*
* These defines determine when the checks happen. Between checks, the OTA Agent is not checking for updates.\n
*   * CY_OTA_INITIAL_CHECK_SECS
*   * CY_OTA_NEXT_CHECK_INTERVAL_SECS
*   * CY_OTA_RETRY_INTERVAL_SECS
*   * CY_OTA_CHECK_TIME_SECS
*
*
* Stop OTA agent
* --------------
* When you want to stop the OTA agent from checking for updates.\n
* \n
* `cy_rslt_t cy_ota_agent_stop(cy_ota_context_ptr *ota_ptr);`
*
* Trigger a check right now.
* ----------------------------------------------
* Use this when you want to trigger a check for an OTA update earlier than\n
* is currently scheduled. The OTA agent must have been started already.\n
* \n
* `cy_rslt_t cy_ota_get_update_now(cy_ota_context_ptr ota_ptr);`
*
*
Override default settings
==================
*  Copy the cy_ota_config.h file from the libs/anycloud-ota/include\n
*   directory to the top-level code example directory in the project.\n
*  Change these values to your preferred settings.
*
.. code:: cpp


	// Initial time for checking for OTA updates
	// This is used to start the timer for the initial OTA update check after calling cy_ota_agent_start().
	#define CY_OTA_INITIAL_CHECK_SECS             (10)              // 10 seconds

	// Next time for checking for OTA updates
	// This is used to re-start the timer after an OTA update check in the OTA Agent.
	#define CY_OTA_NEXT_CHECK_INTERVAL_SECS       (24 * 60 * 60)    // one day between checks
	
	// Retry time which checking for OTA updates
	// This is used to re-start the timer after failing to contact the server during an OTA update check.
	#define CY_OTA_RETRY_INTERVAL_SECS            (5)               // seconds between retries after an error

	// Length of time to check for downloads
	// OTA Agent wakes up, connects to server, and waits this much time before stopping checks.
	// This allows the OTA Agent to be inactive for long periods of time, only checking at the interval.
	// Use 0x00 to continue checking once started.
	#define CY_OTA_CHECK_TIME_SECS                (60)              // 1 minute

	// Expected maximum download time between each OTA packet arrival.
	// This is used check that the download occurs in a reasonable time frame.
	#define CY_OTA_PACKET_INTERVAL_SECS           (60)              // 1 minute

	// Number of retries when attempting OTA update
	// This is used to determine # retries when attempting an OTA update.
	#define CY_OTA_RETRIES                        (5)               // retry entire process 5 times*
	// Number of retries when attempting to contact the server
	// This is used to determine # retries when connecting to the server during an OTA update check.
	#define CY_OTA_CONNECT_RETRIES                (3)               // 3 server connect retries
	// The number of MQTT Topics to subscribe to
	#define CY_OTA_MQTT_MAX_TOPICS                (2)               // 2 topics
	// The keep-alive interval for MQTT
	// An MQTT ping request will be sent periodically at this interval.
	#define CY_OTA_MQTT_KEEP_ALIVE_SECONDS        (60)              // 60 second keep-alive	
	// The timeout for MQTT operations.
	#define CY_OTA_MQTT_TIMEOUT_MS                (5000)            // 5 second timeout waiting for MQTT response


Code Snippets
==================
* \n
* The following code snip demonstrates an example for initializing the cy_ota_network_params_t and ota_agent_params structures required to start the OTA agent.\n
* \snippet ota_snippets.c snippet_cy_ota_structure_initialisation
* \n
* The example function demonstrates the usage of OTA callback function to print the status of every OTA event. The callback feature is optional but be aware that the OTA middleware will not print the status of the OTA agent on its own.\n
* \n
* Note: User needs to initialize retarget-io to use the debug UART port.\n
* \snippet ota_snippets.c snippet_cy_ota_setup_callback
* \n
* The following code snip demonstrates the initialization of OTA agent.\n
* \snippet ota_snippets.c snippet_cy_ota_setup
* \n
*

Code Example
==================

For code example, please refer to this example:\n
https://github.com/cypresssemiconductorco/mtb-example-anycloud-ota-mqtt
