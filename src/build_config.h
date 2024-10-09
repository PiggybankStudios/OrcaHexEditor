/*
File:   build_config.h
Author: Taylor Robbins
Date:   09\21\2024
Description:
	** This file is scraped at build time and also #included by main.c
	** so that both can be informed about various options
	** (like the application name, debug mode, etc.)
*/

#ifndef _BUILD_CONFIG_H
#define _BUILD_CONFIG_H

#define DEBUG_BUILD           1

#define STRINGIFY_DEFINE(define) STRINGIFY(define)
#define STRINGIFY(text)          #text

#define PROJECT_NAME          Orca Hex Editor
#define PROJECT_NAME_SAFE     OcHexEd
#define PROJECT_NAME_STR      STRINGIFY_DEFINE(PROJECT_NAME)
#define PROJECT_NAME_SAFE_STR STRINGIFY_DEFINE(PROJECT_NAME_SAFE)

#define MAIN_HEAP_PAGE_SIZE        Megabytes(16)
#define SCRATCH_ARENAS_PAGE_SIZE   Megabytes(64)
#define SCRATCH_ARENAS_MAX_MARKS   32

#define DEBUG_OUTPUT_ENABLED     1
#define REGULAR_OUTPUT_ENABLED   1
#define INFO_OUTPUT_ENABLED      1
#define NOTIFY_OUTPUT_ENABLED    1
#define OTHER_OUTPUT_ENABLED     1
#define WARNING_OUTPUT_ENABLED   1
#define ERROR_OUTPUT_ENABLED     1

#endif //  _BUILD_CONFIG_H
