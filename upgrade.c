// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// 升级模块
// Authors: jdh99 <jdh821@163.com>

#include "upgrade.h"
#include "lagan.h"

#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#define TAG "upgrade"

static bool isBegin = false;
static esp_ota_handle_t updateHandle = 0 ;
const esp_partition_t* updatePartition = NULL;

// UpgradeBegin 开始升级
bool UpgradeBegin(void) {
    if (isBegin) {
        return true;
    }

    updatePartition = esp_ota_get_next_update_partition(NULL);
    if (updatePartition == NULL) {
        LE(TAG, "begin failed!get partition failed");
        return false;
    }
    LI(TAG, "partition subtype %d at offset 0x%x", updatePartition->subtype, 
        updatePartition->address);
    
    esp_err_t err = esp_ota_begin(updatePartition, OTA_WITH_SEQUENTIAL_WRITES, 
        &updateHandle);
    if (err != ESP_OK) {
        LE(TAG, "begin failed!esp ota begin failed");
        esp_ota_abort(updateHandle);
        return false;
    }

    LI(TAG, "begin upgrade");
    isBegin = true;
    return true;
}

// UpgradeWrite 写入数据
bool UpgradeWrite(uint8_t* data, int size) {
    if (isBegin == false) {
        LW(TAG, "write failed!is not upgrade");
        return false;
    }

    LI(TAG, "write data len:%d", size);
    LaganPrintHex(TAG, LAGAN_LEVEL_DEBUG, data, size);

    if (esp_ota_write(updateHandle, data, size) != ESP_OK) {
        esp_ota_abort(updateHandle);
        isBegin = false;
        
        LE(TAG, "esp_ota_write error!");
        return false;
    }
    return true;
}

// UpgradeEnd 结束升级.成功会自动重启加载新程序
bool UpgradeEnd(void) {
    if (isBegin == false) {
        return true;
    }

    LI(TAG, "end upgrade");

    esp_err_t err = esp_ota_end(updateHandle);
    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            LE(TAG, "Image validation failed, image is corrupted");
        }
        LE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        esp_ota_abort(updateHandle);
        return false;
    }

    err = esp_ota_set_boot_partition(updatePartition);
    if (err != ESP_OK) {
        LE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
        esp_ota_abort(updateHandle);
        return false;
    }

    LI(TAG, "Prepare to restart system!");
    esp_restart();
    isBegin = false;
    return true;
}

// UpgradeAbort 中止升级
void UpgradeAbort(void) {
    if (isBegin == false) {
        return;
    }

    esp_ota_abort(updateHandle);
    isBegin = false;
}
