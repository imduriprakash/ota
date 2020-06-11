/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/* This file contains configuration settings for the OTA HTTP demo.
 *
 * */

#ifndef TEST_HTTP_CONFIG_H_
#define TEST_HTTP_CONFIG_H_

#if USE_HTTP_FOR_TRANSPORT

#define OTA_SERVER_URL                  "10.0.0.88"
#define OTA_SERVER_PORT                 (80)

static const char root_ca_certificate[] = "";
static const char client_cert[] = "";
static const char client_key[] = "";

#endif


#endif /* TEST_HTTP_CONFIG_H_ */
