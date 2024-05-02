//==================================================================================================
/// @file       file_server.h
/// @brief      File server
//==================================================================================================
#pragma once

//==================================================================================================
// Include definition
//==================================================================================================
#include "sdkconfig.h"
#include "esp_vfs.h"
#include "esp_err.h"
#include "esp_http_server.h"


//==================================================================================================
// Constant definition
//==================================================================================================
#define MAX_FILE_PATH_PREFIX_SIZE_BYTE      ESP_VFS_PATH_MAX              // 15 bytes
#define MAX_FILE_NAME_SIZE_BYTE             CONFIG_SPIFFS_OBJ_NAME_LEN    // 64 bytes
/* Max length a file path can have on storage */
#define MAX_FILE_PATH_SIZE_BYTE ( MAX_FILE_PATH_PREFIX_SIZE_BYTE + MAX_FILE_NAME_SIZE_BYTE )