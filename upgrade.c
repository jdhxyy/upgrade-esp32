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
static esp_ota_handle_t updateHandle = 0;
const esp_partition_t* updatePartition = NULL;
static int offset = 0;

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
    offset = 0;
    isBegin = true;
    return true;
}

// UpgradeWrite 写入数据
bool UpgradeWrite(uint8_t* data, int size) {
    if (isBegin == false) {
        LW(TAG, "write failed!is not upgrade");
        return false;
    }

    LI(TAG, "write data len:%d offset:%d", size, offset);
    LaganPrintHex(TAG, LAGAN_LEVEL_DEBUG, data, size);

    if (esp_ota_write(updateHandle, data, size) != ESP_OK) {
        esp_ota_abort(updateHandle);
        isBegin = false;

        LE(TAG, "esp_ota_write error!");
        return false;
    }
    offset += size;
    return true;
}

// UpgradeFlush 刷写尾部不足16字节的数据
// 说明:开启flash加密后,esp_ota_write会把不足16字节的尾部数据缓存在内存中,
//      直到下一次写入或esp_ota_end才真正落盘.若要在结束升级前回读分区校验,必须先调用本接口
bool UpgradeFlush(void) {
    uint8_t padding[16] = {0};
    int remainder = 0;
    int paddingLen = 0;

    if (isBegin == false) {
        LW(TAG, "flush failed!is not upgrade");
        return false;
    }

    remainder = offset % 16;
    if (remainder == 0) {
        return true;
    }

    // 补齐到16字节边界.补齐数据位于文件长度之外,不参与md5校验
    paddingLen = 16 - remainder;
    if (esp_ota_write(updateHandle, padding, paddingLen) != ESP_OK) {
        esp_ota_abort(updateHandle);
        isBegin = false;

        LE(TAG, "esp_ota_write error when flush!");
        return false;
    }

    LI(TAG, "flush padding len:%d data offset:%d", paddingLen, offset);
    return true;
}

// UpgradeGetOffset 读取偏移地址
int UpgradeGetOffset(void) {
    if (isBegin == false) {
        return 0;
    }
    return offset;
}

// UpgradeEnd 结束升级.成功会自动重启加载新程序
bool UpgradeEnd(void) {
    if (isBegin == false) {
        return true;
    }

    LI(TAG, "end upgrade");

    isBegin = false;

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

// UpgradeIsBusy 是否升级中
bool UpgradeIsBusy(void) {
    return isBegin;
}
