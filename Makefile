################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Top-level application make file.
#
################################################################################
# \copyright
# Copyright 2018-2019 Cypress Semiconductor Corporation
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################


################################################################################
# Basic Configuration
################################################################################

# Target board/hardware
TARGET=CY8CPROTO-062-4343W
# Underscore needed for $(TARGET) directory
TARGET_UNDERSCORE=$(subst -,_,$(TARGET))

# Core processor
CORE?=CM4

# Name of application (used to derive name of final linked file).
APPNAME=anycloud-ota


# Name of toolchain to use. Options include:
#
# GCC_ARM -- GCC 7.2.1, provided with ModusToolbox IDE
# ARM     -- ARM Compiler (must be installed separately)
# IAR     -- IAR Compiler (must be installed separately)
#
# See also: CY_COMPILER_PATH below
TOOLCHAIN?=GCC_ARM

# Default build configuration. Options include:
#
# Debug   -- build with minimal optimizations, focus on debugging.
# Release -- build with full optimizations
CONFIG?=Debug

# If set to "true" or "1", display full command-lines when building.
VERBOSE=

# Set to 1 to add OTA defines, sources, and libraries (must be used with MCUBoot)
# NOTE: Extra code must be called from your app to initialize AnyCloud OTA middleware.
OTA_SUPPORT=1

################################################################################
# Advanced Configuration
################################################################################

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
COMPONENTS=FREERTOS PSOC6HAL LWIP MBEDTLS

# Add connectivity device based on the TARGET board
ifeq ($(TARGET), CY8CPROTO-062-4343W)
COMPONENTS+=4343W
else ifeq ($(TARGET), CY8CKIT-062S2-43012)
COMPONENTS+=43012
endif

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=

# Custom configuration of mbedtls library.
MBEDTLSFLAGS = MBEDTLS_USER_CONFIG_FILE='"configs/mbedtls_user_config.h"'

# Add additional defines to the build process (without a leading -D).
DEFINES=$(MBEDTLSFLAGS) CYBSP_WIFI_CAPABLE CY_RETARGET_IO_CONVERT_LF_TO_CRLF
DEFINES+=CY_SD_HOST_CLK_RAMP_UP_TIME_MS_WAKEUP=0 CY_MQTT_ENABLE_SECURE_TEST_MOSQUITTO_SUPPORT CY_RTOS_AWARE

# CY8CPROTO-062-4343W board shares the same GPIO for the user button (SW2)
# and the CYW4343W host wake up pin. Since this example uses the GPIO for
# interfacing with the user button, the SDIO interrupt to wake up the host is
# disabled by setting CY_WIFI_HOST_WAKE_SW_FORCE to '0'.
ifeq ($(TARGET), CY8CPROTO-062-4343W)
DEFINES+=CY_WIFI_HOST_WAKE_SW_FORCE=0
endif

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=hardfp

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
ifeq ($(TOOLCHAIN),GCC_ARM)
LDFLAGS=-Wl,--undefined=uxTopUsedPriority
else
ifeq ($(TOOLCHAIN),IAR)
LDFLAGS=--keep uxTopUsedPriority
else
ifeq ($(TOOLCHAIN),ARM)
LDFLAGS=--undefined=uxTopUsedPriority
else
LDFLAGS=
endif
endif
endif

# Additional / custom libraries to link in to the application.
LDLIBS=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=

# Check for default Version values
# Change the version here or over-ride by setting an environment variable
# before building the application.
#
# export APP_VERSION_MAJOR=2
#
APP_VERSION_MAJOR?=1
APP_VERSION_MINOR?=0
APP_VERSION_BUILD?=0

###########################################################################
#
# OTA Support
#
ifeq ($(OTA_SUPPORT),1)
    # OTA / MCUBoot defines
    #
    # IMPORTANT NOTE: These defines are also used in the building of MCUBOOT
    #                 they must EXACTLY match the values added to
    #                 mcuboot/boot/cypress/MCUBootApp/MCUBootApp.mk
    #
    # Must be a multiple of 1024 (must leave __vectors on a 1k boundary)
    MCUBOOT_HEADER_SIZE=0x400
    MCUBOOT_MAX_IMG_SECTORS=2000
    CY_BOOT_SCRATCH_SIZE=0x00010000
    # Boot loader size defines for mcuboot & app are different, but value is the same
    MCUBOOT_BOOTLOADER_SIZE=0x00012000
    CY_BOOT_BOOTLOADER_SIZE=$(MCUBOOT_BOOTLOADER_SIZE)
    # Primary Slot Currently follows Bootloader sequentially
    CY_BOOT_PRIMARY_1_START=0x00012000
    CY_BOOT_PRIMARY_1_SIZE=0x000EE000
    CY_BOOT_SECONDARY_1_SIZE=0x000EE000

    # Change to non-zero if stored in external FLASH
    CY_FLASH_ERASE_VALUE=0

    # Additional / custom linker flags.
    # This needs to be before finding LINKER_SCRIPT_WILDCARD as we need the extension defined
    ifeq ($(TOOLCHAIN),GCC_ARM)
    CY_ELF_TO_HEX=$(CY_CROSSPATH)/bin/arm-none-eabi-objcopy
    CY_ELF_TO_HEX_OPTIONS="-O ihex"
    CY_ELF_TO_HEX_FILE_ORDER="elf_first"
    CY_TOOLCHAIN=GCC
    CY_TOOLCHAIN_LS_EXT=ld
    LDFLAGS+="-Wl,--defsym,MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE),--defsym,MCUBOOT_BOOTLOADER_SIZE=$(MCUBOOT_BOOTLOADER_SIZE),--defsym,CY_BOOT_PRIMARY_1_SIZE=$(CY_BOOT_PRIMARY_1_SIZE)"
    else
    ifeq ($(TOOLCHAIN),IAR)
    CY_ELF_TO_HEX=$(CY_CROSSPATH)/bin/ielftool
    CY_ELF_TO_HEX_OPTIONS="--ihex"
    CY_ELF_TO_HEX_FILE_ORDER="elf_first"
    CY_TOOLCHAIN=$(TOOLCHAIN)
    CY_TOOLCHAIN_LS_EXT=icf
    LDFLAGS+=--config_def MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE) --config_def MCUBOOT_BOOTLOADER_SIZE=$(MCUBOOT_BOOTLOADER_SIZE) --config_def CY_BOOT_PRIMARY_1_SIZE=$(CY_BOOT_PRIMARY_1_SIZE)
    else
    ifeq ($(TOOLCHAIN),ARM)
    CY_ELF_TO_HEX=$(CY_CROSSPATH)/bin/fromelf.exe
    CY_ELF_TO_HEX_OPTIONS="--i32 --output"
    CY_ELF_TO_HEX_FILE_ORDER="hex_first"
    CY_TOOLCHAIN=GCC
    CY_TOOLCHAIN_LS_EXT=sct
    LDFLAGS+=--pd=-DMCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE) --pd=-DMCUBOOT_BOOTLOADER_SIZE=$(MCUBOOT_BOOTLOADER_SIZE) --pd=-DCY_BOOT_PRIMARY_1_SIZE=$(CY_BOOT_PRIMARY_1_SIZE)
    else
    LDFLAGS+=
    endif #ARM
    endif #IAR
    endif #GCC_ARM

    # Linker Script
    LINKER_SCRIPT_WILDCARD:=./$(TARGET_UNDERSCORE)/COMPONENT_$(CORE)/TOOLCHAIN_$(TOOLCHAIN)/ota/*_ota_int.$(CY_TOOLCHAIN_LS_EXT)
    LINKER_SCRIPT:=$(wildcard $(LINKER_SCRIPT_WILDCARD))

    # MCUBoot location
    MCUBOOT_DIR=./libs/mcuboot

    # build location
    ifeq ($(CY_BUILD_LOCATION),)
        # NOTE: This needs to be different from "build" as GitLab scripts use that directory
        CY_BUILD_LOCATION=./bld
    endif

    # MCU sign script location
    ifeq ($(SIGN_SCRIPT_FILE_PATH),)
        SIGN_SCRIPT_FILE_PATH=./scripts/sign_script.bash
    endif

    # output directory for use in the sign_script.bash
    OUTPUT_FILE_PATH=$(CY_BUILD_LOCATION)/$(APPNAME)/$(TARGET)/$(CONFIG)

    # signing scripts and keys from MCUBoot
    IMGTOOL_SCRIPT_NAME=imgtool.py
    MCUBOOT_SCRIPT_FILE_DIR=$(MCUBOOT_DIR)/scripts
    MCUBOOT_KEY_DIR=$(MCUBOOT_DIR)/boot/cypress/keys
    # Additional include directories from MQTT
    # Our test/configs directory has different mcuboot_config.h and mcuboot_logging.h files
    #  to fix errors encountered compiling with IAR, and must come before mcuboot includes.
    INCLUDES+=./test/configs\
              ./test\
              ./libs/mqtt/src\
              ./libs/mqtt/cyport/include\
              ./libs/wifi-mw-core/configs

    DEFINES+=OTA_SUPPORT=1 \
        OTA_MQTT_USE_TLS=$(OTA_MQTT_USE_TLS) \
        MCUBOOT_HEADER_SIZE=$(MCUBOOT_HEADER_SIZE) \
        MCUBOOT_MAX_IMG_SECTORS=$(MCUBOOT_MAX_IMG_SECTORS) \
        CY_BOOT_SCRATCH_SIZE=$(CY_BOOT_SCRATCH_SIZE) \
        MCUBOOT_IMAGE_NUMBER=1\
        MCUBOOT_BOOTLOADER_SIZE=$(MCUBOOT_BOOTLOADER_SIZE) \
        CY_BOOT_BOOTLOADER_SIZE=$(CY_BOOT_BOOTLOADER_SIZE) \
        CY_BOOT_PRIMARY_1_START=$(CY_BOOT_PRIMARY_1_START) \
        CY_BOOT_PRIMARY_1_SIZE=$(CY_BOOT_PRIMARY_1_SIZE) \
        CY_BOOT_SECONDARY_1_SIZE=$(CY_BOOT_SECONDARY_1_SIZE) \
        CY_FLASH_ERASE_VALUE=$(CY_FLASH_ERASE_VALUE)\
        APP_VERSION_MAJOR=$(APP_VERSION_MAJOR)\
        APP_VERSION_MINOR=$(APP_VERSION_MINOR)\
        APP_VERSION_BUILD=$(APP_VERSION_BUILD)

    # Custom post-build commands to run.
    MCUBOOT_KEY_FILE=$(MCUBOOT_KEY_DIR)/cypress-test-ec-p256.pem

    # For signing, use "sign", and key path
    IMGTOOL_COMMAND_ARG=sign
    CY_SIGNING_KEY_ARG="-k $(MCUBOOT_KEY_FILE)"
    # Use "Create" for PSoC 062 instead of "sign", and no key path (use a space " " for keypath to keep batch happy)
    # MCUBoot must also be modified to skip checking the signature
    #   Comment out and re-build MCUBootApp
    #   <mcuboot>/boot/cypress/MCUBootApp/config/mcuboot_config/mcuboot_config.h
    #   line 37, 38, 77
    # 37: //#define MCUBOOT_SIGN_EC256
    # 38: //#define NUM_ECC_BYTES (256 / 8)   // P-256 curve size in bytes, rnok: to make compilable
    # 77: //#define MCUBOOT_VALIDATE_PRIMARY_SLOT
    SECURE_TEST=$(shell echo $(TARGET) | grep "062")
    ifneq ($(SECURE_TEST),)
        IMGTOOL_COMMAND_ARG=create
        CY_SIGNING_KEY_ARG=" "
    endif
    CY_HEX_TO_BIN="$(CY_COMPILER_GCC_ARM_DIR)/bin/arm-none-eabi-objcopy"
    CY_BUILD_VERSION=$(APP_VERSION_MAJOR).$(APP_VERSION_MINOR).$(APP_VERSION_BUILD)

    POSTBUILD=$(SIGN_SCRIPT_FILE_PATH) $(OUTPUT_FILE_PATH) $(APPNAME)\
              $(CY_ELF_TO_HEX) $(CY_ELF_TO_HEX_OPTIONS) $(CY_ELF_TO_HEX_FILE_ORDER)\
              $(MCUBOOT_SCRIPT_FILE_DIR) $(IMGTOOL_SCRIPT_NAME) $(IMGTOOL_COMMAND_ARG) $(CY_FLASH_ERASE_VALUE) $(MCUBOOT_HEADER_SIZE)\
              $(MCUBOOT_MAX_IMG_SECTORS) $(CY_BUILD_VERSION) $(CY_BOOT_PRIMARY_1_START) $(CY_BOOT_PRIMARY_1_SIZE)\
              $(CY_HEX_TO_BIN) $(CY_SIGNING_KEY_ARG)

endif # OTA Support

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the "base" library. It provides the core makefile build
# infrastructure.
CY_BASELIB_PATH=libs/psoc6make

# Absolute path to the compiler's "bin" directory.
#
# The default depends on the selected TOOLCHAIN (GCC_ARM uses the ModusToolbox
# IDE provided compiler by default).
CY_COMPILER_PATH?=


# Locate ModusToolbox IDE helper tools folders in default installation
# locations for Windows, Linux, and macOS.
CY_WIN_HOME=$(subst \,/,$(USERPROFILE))
CY_TOOLS_PATHS ?= $(wildcard \
    $(CY_WIN_HOME)/ModusToolbox/tools_* \
    $(HOME)/ModusToolbox/tools_* \
    /Applications/ModusToolbox/tools_*)

# If you install ModusToolbox IDE in a custom location, add the path to its
# "tools_X.Y" folder (where X and Y are the version number of the tools
# folder).
CY_TOOLS_PATHS+=

# Default to the newest installed tools folder, or the users override (if it's
# found).
CY_TOOLS_DIR=$(lastword $(sort $(wildcard $(CY_TOOLS_PATHS))))

ifeq ($(CY_TOOLS_DIR),)
$(error Unable to find any of the available CY_TOOLS_PATHS -- $(CY_TOOLS_PATHS))
endif

$(info Tools Directory: $(CY_TOOLS_DIR))
include $(CY_TOOLS_DIR)/make/start.mk
