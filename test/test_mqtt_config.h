/*
 * $ Copyright Cypress Semiconductor Apache2 $
 */

/* This file contains configuration settings for the OTA MQTT demo.
 *
 * 1 - choose a server to test with by uncommenting the define
 * 2 - if using AMAZON, turn off TLS in the Makefile
 *
 * */

#ifndef TEST_MQTT_CONFIG_H_
#define TEST_MQTT_CONFIG_H_

#if MOSQUITTO
/* Mosquitto Broker connection Info */
#define OTA_SERVER_URL                "test.mosquitto.org"

#ifdef OTA_USE_TLS
#define OTA_SERVER_PORT                 (8884)
#else
#define OTA_SERVER_PORT                 (1883)
#endif

/* Mosquitto - Root CA */
static const char root_ca_certificate[] =
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIC8DCCAlmgAwIBAgIJAOD63PlXjJi8MA0GCSqGSIb3DQEBBQUAMIGQMQswCQYD\n" \
        "VQQGEwJHQjEXMBUGA1UECAwOVW5pdGVkIEtpbmdkb20xDjAMBgNVBAcMBURlcmJ5\n" \
        "MRIwEAYDVQQKDAlNb3NxdWl0dG8xCzAJBgNVBAsMAkNBMRYwFAYDVQQDDA1tb3Nx\n" \
        "dWl0dG8ub3JnMR8wHQYJKoZIhvcNAQkBFhByb2dlckBhdGNob28ub3JnMB4XDTEy\n" \
        "MDYyOTIyMTE1OVoXDTIyMDYyNzIyMTE1OVowgZAxCzAJBgNVBAYTAkdCMRcwFQYD\n" \
        "VQQIDA5Vbml0ZWQgS2luZ2RvbTEOMAwGA1UEBwwFRGVyYnkxEjAQBgNVBAoMCU1v\n" \
        "c3F1aXR0bzELMAkGA1UECwwCQ0ExFjAUBgNVBAMMDW1vc3F1aXR0by5vcmcxHzAd\n" \
        "BgkqhkiG9w0BCQEWEHJvZ2VyQGF0Y2hvby5vcmcwgZ8wDQYJKoZIhvcNAQEBBQAD\n" \
        "gY0AMIGJAoGBAMYkLmX7SqOT/jJCZoQ1NWdCrr/pq47m3xxyXcI+FLEmwbE3R9vM\n" \
        "rE6sRbP2S89pfrCt7iuITXPKycpUcIU0mtcT1OqxGBV2lb6RaOT2gC5pxyGaFJ+h\n" \
        "A+GIbdYKO3JprPxSBoRponZJvDGEZuM3N7p3S/lRoi7G5wG5mvUmaE5RAgMBAAGj\n" \
        "UDBOMB0GA1UdDgQWBBTad2QneVztIPQzRRGj6ZHKqJTv5jAfBgNVHSMEGDAWgBTa\n" \
        "d2QneVztIPQzRRGj6ZHKqJTv5jAMBgNVHRMEBTADAQH/MA0GCSqGSIb3DQEBBQUA\n" \
        "A4GBAAqw1rK4NlRUCUBLhEFUQasjP7xfFqlVbE2cRy0Rs4o3KS0JwzQVBwG85xge\n" \
        "REyPOFdGdhBY2P1FNRy0MDr6xr+D2ZOwxs63dG1nnAnWZg7qwoLgpZ4fESPD3PkA\n" \
        "1ZgKJc2zbSQ9fCPxt2W3mdVav66c6fsb7els2W2Iz7gERJSX\n" \
        "-----END CERTIFICATE-----\n";

/* Mosquitto - Client Certificate */
static const char client_cert[] =
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIDHDCCAoWgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\n" \
        "FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\n" \
        "CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\n" \
        "ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMDAzMTgwNTIy\n" \
        "MzBaFw0yMDA2MTYwNTIyMzBaMHcxCzAJBgNVBAYTAklOMRIwEAYDVQQIDAlLQVJO\n" \
        "QVRBS0ExEjAQBgNVBAcMCUJFTkdBTFVSVTENMAsGA1UECgwEVVNFUjENMAsGA1UE\n" \
        "CwwEVVNFUjENMAsGA1UEAwwEVVNFUjETMBEGCSqGSIb3DQEJARYEVVNFUjCCASIw\n" \
        "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANZvzIHU7iru9xz1B7O5YdsujVGN\n" \
        "fEvAxR0qtdzqQv4VdGeglP0xbNPjSNdx4yZ/YbVq2uO5nz8SgysJnQjGEP6AVC9Y\n" \
        "9ysEsJCHYB6cQFGP+fzBtSIIWF8oFUltCr6q97znZKM9lsLRJ2JnRVc6sQ3YTVHv\n" \
        "o3xketv4b5jAB3mRWxG+Dcl1nRKNEiGBHIAJTE3yQFMahZe8Q9Rep42EhRoMbuTu\n" \
        "MMuEGjdeQXgdhD7Nzz2X2hKx4IkmEt5R3sZSx1Ez0xa1DxOKrQpdUezSjwdVdGU3\n" \
        "8mVR9BLAnGm9WV1lFdnFiY1zJNLaHrBPt9Tt+lmBqPMvzX/3PKiRzuUDs0sCAwEA\n" \
        "AaMaMBgwCQYDVR0TBAIwADALBgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADgYEA\n" \
        "JnoXgUcbApaW3aUIbCPtxnwud95Hd+NgWC4TK2inEhl9XS52JSuPPHXRndh2AUdo\n" \
        "VanNjwyjPR/HJBX2GRCB8+l8q7yXZP5bWeV5hldxwFxzOegyX0VXNbkiREFTnOi6\n" \
        "2lqJkrKKCdZqKCMchZBedk3FUVVz8WuPymaLrdf2n0g=\n" \
        "-----END CERTIFICATE-----\n" ;


/* Mosquitto - Client Private Key */
static const char client_key[] =
        "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEpQIBAAKCAQEA1m/MgdTuKu73HPUHs7lh2y6NUY18S8DFHSq13OpC/hV0Z6CU\n" \
        "/TFs0+NI13HjJn9htWra47mfPxKDKwmdCMYQ/oBUL1j3KwSwkIdgHpxAUY/5/MG1\n" \
        "IghYXygVSW0Kvqr3vOdkoz2WwtEnYmdFVzqxDdhNUe+jfGR62/hvmMAHeZFbEb4N\n" \
        "yXWdEo0SIYEcgAlMTfJAUxqFl7xD1F6njYSFGgxu5O4wy4QaN15BeB2EPs3PPZfa\n" \
        "ErHgiSYS3lHexlLHUTPTFrUPE4qtCl1R7NKPB1V0ZTfyZVH0EsCcab1ZXWUV2cWJ\n" \
        "jXMk0toesE+31O36WYGo8y/Nf/c8qJHO5QOzSwIDAQABAoIBAQCtmwGUsxDHDYRe\n" \
        "mznLFXDWBFBBECzrH3v16xglbDskfbYqF2XHvnX+7WmRkxB1oHc4avQfna4BYDl1\n" \
        "ZGkQK1qi0L5XPs2MtFVGDXOSFKo13jxjl2fPz7XMBCWHtpIWX1AGTce6KSifPLmH\n" \
        "/1eyEUA6lz2x8jMHiEm3QOEIZ58lmT8DojKYvnWobmeTGnNxZRkHYGPkCpXAw9JG\n" \
        "aVph/SY1vRARP5O+I8Tox0FfBEcodMM0ap6/H66tmrpQ98FMbvpcUYOHxg1nmnkd\n" \
        "MhULA3wrKugbIWjl3MCaI92ZKBGA2Vc2eoO+RFYn7gqFCUU85nznNNVd1iHSLyay\n" \
        "eE6cHVxhAoGBAPBYy0dbzGk8hWouwnERAFTFk5Yt0yoYGZFrZA/eIaaOEJMwVPdq\n" \
        "TwSyW8GCLDqM6tTZ+3QIHLqPWnN4C9gUucNyop5BYKFihwSwQ86LAUs02wDC2k00\n" \
        "YKZR+hvXT6dnR09n/9bXcH2ybEYsjstUPDEfaVJ6/u/5Q5NZb0VsfQwZAoGBAORn\n" \
        "A/rIomNFL5PKyOfNwUk5y0vRCw6D9fDc0SvytKGam1d49uFnWNTh6GMAE5eVq8Ny\n" \
        "bxPTfV9FKdCdc2rn+5WL42AT3NUmGJqaQWn/b1yg80VagYXt5bjBa3AsCTBq9j1U\n" \
        "vzZcEtzMNFwQv4N+GZmHhE6LNBSTrRXo9TJq8ucDAoGBALp5OeY4a+TuRti2RLfy\n" \
        "p94dmL84+I+9OQEhX/YLsvIFYQjBw/pd6cbMFvKQ633V3VCde0FwFdIM7PGPnyjc\n" \
        "E7B3XYIJWU59IiJaG5HBOZS/AneZtkOWMrJ4AePd7ngEYkhSeSWX6zgq2WSsp3FL\n" \
        "/pNiOpX6xZcsPuiwx+sGHBLpAoGAdN9dwvLtXTym1spaOXson46qP1ZtgPT3YVOs\n" \
        "2P8tfM//Eyh7s+VNKEmcvyPn4pYcuTsSKaP7sEdU2UHoMFkafHCDXcNsKcl/p4St\n" \
        "Wk7xrgVR1jg0tNKBzSN0FYcDALqK2+FEos1EfaUkWcPcK63X7Z7RjdtfP/Q9CkwV\n" \
        "Q8JEHv8CgYEAjGNAgEC2tVwFdBIDof63JiZw95zpYwNm9Yn3L2MObK/EbaikJSm4\n" \
        "KTtKU+M2sWbw0LvTmvdxnddX5Qo8HjU/Y/KopOgrsPMQIYBzypds9EwjxTaUT3Yf\n" \
        "vTtM+VKM8L3N+Wo1Upcrp667lTMjF2bO8cbR+N+zP5DY8JtBd7FJpXk=\n" \
        "-----END RSA PRIVATE KEY-----\n" ;

#endif

#if AMAZON
/* Amazon Broker connection Info */
#define OTA_SERVER_URL                  "a2pn4v3xtwglbn-ats.iot.us-east-1.amazonaws.com"

#ifdef OTA_USE_TLS
#define OTA_SERVER_PORT                 (8883)
#else
#error "Amazon Broker can only be used with TLS!"
#endif

/* Amazon - Root CA */
static const char root_ca_certificate[] =
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF \n" \
        "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6 \n" \
        "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL \n" \
        "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv \n" \
        "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj \n" \
        "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM \n" \
        "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw \n" \
        "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6 \n" \
        "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L \n" \
        "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm \n" \
        "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC \n" \
        "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA \n" \
        "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI \n" \
        "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs \n" \
        "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv \n" \
        "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU \n" \
        "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy \n" \
        "rqXRfboQnoZsG4q5WTP468SQvvG5\n" \
        "-----END CERTIFICATE-----\n";

/* Amazon Client Certificate */
static const char client_cert[] =
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIDWTCCAkGgAwIBAgIUdVVnwp5b8gAw8NipiseSYNaKTtkwDQYJKoZIhvcNAQEL\n" \
        "BQAwTTFLMEkGA1UECwxCQW1hem9uIFdlYiBTZXJ2aWNlcyBPPUFtYXpvbi5jb20g\n" \
        "SW5jLiBMPVNlYXR0bGUgU1Q9V2FzaGluZ3RvbiBDPVVTMB4XDTE5MTAxMTA2NDA0\n" \
        "OVoXDTQ5MTIzMTIzNTk1OVowHjEcMBoGA1UEAwwTQVdTIElvVCBDZXJ0aWZpY2F0\n" \
        "ZTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANgVb+TvsuluyNcseovo\n" \
        "yJV3mYhULF2tawjslZ1cewOqVTWwp0J9E3dKjwVYjbOraM7raHwdzAnoE6wANXRK\n" \
        "b9ejJ+uiOffqXHOPEWaGeFf91eZ1aWZ1W7tY4ptvKbRFIT9lG736qLZyJZDFCahD\n" \
        "jVaA/S5QwMQh4wlPMxM8LJntcHBkbLnO+hgZxDSNr3lKrEtxN4Eku1JBksapkK9s\n" \
        "CMlP33BuhLsBnSzRx9az7cX77zZQs1Fc/xb4Th9brQ63+JVedYVmw5mylgiOB5xJ\n" \
        "bmmwTOhaVLOwRFx7RnR5ww5dGlHGaa6kz/QcMudbMr3ROqSHhz2YXSBzoPzeFn+L\n" \
        "PCkCAwEAAaNgMF4wHwYDVR0jBBgwFoAU193aI791C7ZRSJtt5neVX3d2mIEwHQYD\n" \
        "VR0OBBYEFJrcV2NZk8cNb57Y501qEjq7ZYmQMAwGA1UdEwEB/wQCMAAwDgYDVR0P\n" \
        "AQH/BAQDAgeAMA0GCSqGSIb3DQEBCwUAA4IBAQBUoM4AhC/3gRx+88l1XfIInzIS\n" \
        "kYy6ithX/3fRre/lHt38hTfmELsIMElBIbFK8zqakUOpwwVxzUWgfoV5iAGRwRr9\n" \
        "dl2MgUPrwOnFLpf9bmU090oW/WDg2W3io8MC9EeMNUne7uUxy1Q3+lIV5dA6Q+kh\n" \
        "kKLhYVUPryckaHnypcJYPBc6RcIKFES071UG4vBMOaUSy0vhYXffhFmJhaVpuM0J\n" \
        "Lg/plW7dNJzktQyuwhJobwWIENjIk+YqSkXKcxADXlsRfzPgefTIcglsgjpoEC9D\n" \
        "zxeS0RulQiltQf645L1jtnRQT90RzX8eppKq4fmEh4CgDesVLTCEHns0oCjs\n" \
        "-----END CERTIFICATE-----\n";

/* Amazon Client Private Key */
static const char client_key[] =
        "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEpgIBAAKCAQEA2BVv5O+y6W7I1yx6i+jIlXeZiFQsXa1rCOyVnVx7A6pVNbCn  \n" \
        "Qn0Td0qPBViNs6tozutofB3MCegTrAA1dEpv16Mn66I59+pcc48RZoZ4V/3V5nVp  \n" \
        "ZnVbu1jim28ptEUhP2UbvfqotnIlkMUJqEONVoD9LlDAxCHjCU8zEzwsme1wcGRs  \n" \
        "uc76GBnENI2veUqsS3E3gSS7UkGSxqmQr2wIyU/fcG6EuwGdLNHH1rPtxfvvNlCz  \n" \
        "UVz/FvhOH1utDrf4lV51hWbDmbKWCI4HnEluabBM6FpUs7BEXHtGdHnDDl0aUcZp  \n" \
        "rqTP9Bwy51syvdE6pIeHPZhdIHOg/N4Wf4s8KQIDAQABAoIBAQDSMRglNF1u4MsR  \n" \
        "ND0It6nWav64NFjrzvNdPPXUms82o4w7q9knvg8fbTFuW9OLQGO1VBzRDxMZV906  \n" \
        "5QEaXAYrVpRQi/+jfx/rcPHWL1wFC+/0O6YJeoN+3FmiJSIGL3DGAA5aYOeYgbS0  \n" \
        "/FYGjZ+A0/xz/y1kHCqDNb1TKqegGiRAg1vP9GCfqbO1jsd8sCu7+Hzpu++G/6ao  \n" \
        "5m+QUGwnBrwr+UtA5b4uWxuUl4OCLdRnvb5XOwE83GtnvTLsXhBn6Qm/iNLQ2klI  \n" \
        "fr88SXXloWino5O1cY/7BM34hjJlStapstTSSjtHLQNEz+X2jrenye7osNZbNZ4T  \n" \
        "4BPk2Dz5AoGBAOvKNegEkxtX70yzP7Zt7M3gyW26eUGKKFjOkuLt7Hd+yjpT1pTR  \n" \
        "DdHnHLu615IY1KViJn36+WeqaTc+EArRzXyJCrnoS/PGJPqHDAo0/zmOFrWPzsGt  \n" \
        "kkRgqkS1rz3tKaNQO15Q+3O0deJ36nNdt0K9561WjmYLdgSXEE8zQJDvAoGBAOqa  \n" \
        "07C2742+/fORlQvZFWh3ohtA4TuPECLkizpoATlhxPGE8oCEmbW3N6t38bBkfP8x  \n" \
        "6A25Cn9i/rVUk0l4DAdL00na40YCYz/bVJ0aaSidFigZYKY07UzNKLaliCEHWeMb  \n" \
        "LFxHBmGl6XZ033sjlyDf+eFMAJisSzaBdIjqJdRnAoGBAOptIudJFREMfmjdlizE  \n" \
        "YbnXEa08xq24tKCG0Vc5ge7Uai9Ph3px1rGoe9LR0t34PgMn8Gk0i57buLwpwj1f  \n" \
        "llOI1Nc887ase7txZTEIu+/3KvDcx42cCl5dVSv3DwIct1cdv8jn7vh8pO/ClaCZ  \n" \
        "vkhnEt4tvmoKXw31O0vjAwCVAoGBAI75dHGJy0xTTWkXHAJwdf+IiR6a16j4cxkQ  \n" \
        "t1kM8VILJ38ZSsDQDu0Tj7XMD1s0oojT3VwfnSotJpVTyb9IjewUZdvorgGIivvr  \n" \
        "qxFjexrpQQuHC8O/Fa9siJdnIiMe28TwVoSWNGlmN3WdnGTrN7stRJftPZ/NwLij  \n" \
        "BGY8bHmZAoGBANR4TgZ0PuL2t2uMrghYWdbdmXunPWEZP6cMw4au41aBpRGp5cas  \n" \
        "NW6dmahAJ98m87EhMfi/ZkRcdmJXlRD/Jy9vBQX7jhSEwm2NFvMqWOjP2+qMxsjx  \n" \
        "2Dpa1Dk5/p9GDOjiHApRy7I2QqmqDCzc+KiU2rzrkNEPYYXfQHkHHDl3\n" \
        "-----END RSA PRIVATE KEY-----\n";
#endif

#if ECLIPSE
/* Eclipse Mosquitto Broker connection Info */
#define OTA_SERVER_URL      "mqtt.eclipse.org" /* This server is running the open source Mosquitto broker in its most recently released version*/

#ifdef OTA_USE_TLS
#error "Eclipse Broker Host Publish needs new certificate!"
#else
#define OTA_SERVER_PORT                 (1883)
#endif

/* Eclipse mosquitto - Root CA */
static const char root_ca_certificate[] =
        "-----BEGIN CERTIFICATE----- \n" \
        "MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/ \n" \
        "MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT \n" \
        "DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow \n" \
        "PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD \n" \
        "Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB \n" \
        "AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O \n" \
        "rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq \n" \
        "OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b \n" \
        "xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw \n" \
        "7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD \n" \
        "aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV \n" \
        "HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG \n" \
        "SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69 \n" \
        "ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr \n" \
        "AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz \n" \
        "R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5 \n" \
        "JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo \n" \
        "Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\n" \
        "-----END CERTIFICATE-----\n";

/* Eclipse mosquitto - Client Certificate */
static const char client_cert[] =
        "-----BEGIN CERTIFICATE-----\n" \
        "MIIDHDCCAoWgAwIBAgIBADANBgkqhkiG9w0BAQsFADCBkDELMAkGA1UEBhMCR0Ix\n" \
        "FzAVBgNVBAgMDlVuaXRlZCBLaW5nZG9tMQ4wDAYDVQQHDAVEZXJieTESMBAGA1UE\n" \
        "CgwJTW9zcXVpdHRvMQswCQYDVQQLDAJDQTEWMBQGA1UEAwwNbW9zcXVpdHRvLm9y\n" \
        "ZzEfMB0GCSqGSIb3DQEJARYQcm9nZXJAYXRjaG9vLm9yZzAeFw0yMDAzMTcwOTUx\n" \
        "MTVaFw0yMDA2MTUwOTUxMTVaMHcxCzAJBgNVBAYTAklOMRIwEAYDVQQIDAlLQVJO\n" \
        "QVRBS0ExEjAQBgNVBAcMCUJFTkdBTFVSVTENMAsGA1UECgwEVEVTVDENMAsGA1UE\n" \
        "CwwEVEVTVDEQMA4GA1UEAwwHQ1lfVEVTVDEQMA4GCSqGSIb3DQEJARYBLTCCASIw\n" \
        "DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL91PrnDOF4evHsvj7lXIgOgcw/h\n" \
        "hWvHmh5sxoTNHRz9nhkCFDLh+YyGf74k3aQy2cZTM58N51M1lHNo39fsyQC2amas\n" \
        "40DjJiu2Ypi+KaIWn0vufGqisj8M4guzrsbnVqwpRKBkokwnBthc0YMlqmJZUIk5\n" \
        "LGp5mwI8WtfTd46Bk6mWKlZ+MzF8lQ1/rkCN+hOji7ZJmcrgXTq6xLea6j38BN53\n" \
        "L6JVlAXt26H0r1tCme0eTtQzsAf4IG4cK0KZJqYQqrIZUrmKDAF2s113TRFZJzy0\n" \
        "4rtow3ff+Gh4aVghQy+M8knaRsQO9eWe/Rj1/mpZZQlUiwAsWcXrzZEptw0CAwEA\n" \
        "AaMaMBgwCQYDVR0TBAIwADALBgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQELBQADgYEA\n" \
        "N84Hfd9qdeK9RCBQRQ4B2jizCVvy50a1y1FI79Jo9NnYxP/qg0vdvWEyHuY3rmdS\n" \
        "5w4k9dau+FZ+7zUCLea/DArEYU/coo9UuJao98K0pFdGDtdrbmV8+cJqm4Vhsv9r\n" \
        "NIRr4mKcHcqOzdfd0tGycfmTeyEtXi0+mKzmZjfTRtw=\n" \
        "-----END CERTIFICATE-----\n";

/* Eclipse mosquitto - Client Private Key */
static const char client_key[] =
        "-----BEGIN RSA PRIVATE KEY-----\n" \
        "MIIEowIBAAKCAQEAv3U+ucM4Xh68ey+PuVciA6BzD+GFa8eaHmzGhM0dHP2eGQIU\n" \
        "MuH5jIZ/viTdpDLZxlMznw3nUzWUc2jf1+zJALZqZqzjQOMmK7ZimL4pohafS+58\n" \
        "aqKyPwziC7OuxudWrClEoGSiTCcG2FzRgyWqYllQiTksanmbAjxa19N3joGTqZYq\n" \
        "Vn4zMXyVDX+uQI36E6OLtkmZyuBdOrrEt5rqPfwE3ncvolWUBe3bofSvW0KZ7R5O\n" \
        "1DOwB/ggbhwrQpkmphCqshlSuYoMAXazXXdNEVknPLTiu2jDd9/4aHhpWCFDL4zy\n" \
        "SdpGxA715Z79GPX+alllCVSLACxZxevNkSm3DQIDAQABAoIBAAyb0BGzxZaNN6R1\n" \
        "MWXnUxoiCvHUzFCYyJGQYFKGn5A1M6A5/tKLB0A161u+Wcpifu0Mta2X4NAeCzS+\n" \
        "h19U/LfLS56PbDftvanLTRr6CwvdqYf6oL71EAAO/SQAJVZ8Db0wypA1kGqrd0vw\n" \
        "cOvOIzZu41rmJQKuF4NCLklgZ9M+oCpYsDSUDNKWJ8gzcKBKfafLkjsZ7aOPDBbD\n" \
        "9JdIFrUM2KOtGDZ4G1z0ebmYsbvLJH6Y028/Iiu8Lhkvpp7DEO1sud9LzV/qhwky\n" \
        "+UFvQe9G2SKTGkHF3qBmHHCbGGCRWtc5IbVyCWX6YqEHzwOM9ev4qY+lCNIWHVOl\n" \
        "cC2DOJECgYEA5FNL5245p1eLirYhf0lUoQm90ZqOC/tLOqSqp0hnGVLJiI5Eio+y\n" \
        "jRu3dz6YCrv+3GNLaLByCiB9ddRmHK5cIxzpAqIzTRpM1D+39qmsZwnZhzW9uM93\n" \
        "L3dB/fgQ+UG2RAkVeRyXrIIwKk6ubTl+nSMt2jCYmgm6BGN4dhhYJXcCgYEA1qn9\n" \
        "tp0Kz1An8YbdipAR2QWN0mMjMPSNzRD1lJUfiANsg2ZlqV6KPl63EEa/Ya/SE8vw\n" \
        "rD6mwQTCckogXATV01fx10ZeRXX2VxdGiiR1hgb6Qyzplz69HuZ5oNUHSmI8xEa8\n" \
        "OPbxh4NdsQ0vq27FWaFpYB9IXqCfcnZ0EFtvOJsCgYAkkvs7rTprBekCZT5kZ1i5\n" \
        "Gxhe9QUAAlcAo9P6S+zH76z8xN1R3MbbN69h2JL+VTL0FqvDr0/1M1Mlhsp8mamE\n" \
        "rmLc2yKgnS/lZ/eN+Yv2TWuc7wzfB8ytu8rEZfJCGzaZG2NnLR9WsNwKSuRB9s3D\n" \
        "WhlENDidpCU6Ar0PHcMw7QKBgQCiUMmSkq7A0bhSTgsoogWM10FEy0ep1dF/NQqs\n" \
        "/hFolEYyGXRXnC/mN1IvEX/XCs5Lv9LNO1Z17vZk2x4a1mEkvIQWppPT6/2IHwZs\n" \
        "oGLpJE+KCeTbQ0pXmihv74TivuN5+o6KeFdrg26N2Q2pkEnTm1SFpUHgo43NTDo4\n" \
        "wBqabwKBgBwvtNr4V7uHZY7eSDCe67TibEzBjttpTAo+y6j2P4FpJR6MS0ww0MdZ\n" \
        "nqtJ/2AG54+O+LGRH8T3Uj9L0HChe/HRRhKQmvZqMvPMBhmpCuW02Lz2doPyoZSD\n" \
        "zU1FdfIcRCX4MKBewcWV1nY4S/G2H6kZHNphc8bZ2ttyMM+ui21K\n" \
        "-----END RSA PRIVATE KEY-----\n" ;
#endif /* ECLIPSE */

#endif /* TEST_MQTT_CONFIG_H_ */
