# Cypress Over The Air (OTA) library

## What's Included?
Refer to the [README.md](./README.md) for a complete description of the OTA library

## Known Issues
* None

## Changelog

### v1.2.1
* update cyignore
* Basic HTTP one file OTA Image updating is working


### v1.2.0
* Initial public release of OTA library
* Add MQTT connection context to paramaters for cy_ota_agent_start()
* Fix OTA per-packet timer
* Fix CY_OTA_MQTT_TIMEOUT_MS
* Always check for duplicate packets / missed packets
* Added configUSE_NEWLIB_REENTRANT to FreeRTOSConfig.h for test app


### v1.1
* Add MQTT "clean" flag to paramaters for cy_ota_agent_start()
* Update on-line docs
* Made OTA data flow more efficient.

### v1.0.0
* Initial release of OTA library

## Additional Information
* [ModusToolbox Software Environment, Quick Start Guide, Documentation, and Videos](https://www.cypress.com/products/modustoolbox-software-environment)
