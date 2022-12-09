// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// 升级模块
// Authors: jdh99 <jdh821@163.com>

#ifndef UPGRADE_H
#define UPGRADE_H

#include <stdint.h>
#include <stdbool.h>

// UpgradeBegin 开始升级
bool UpgradeBegin(void);

// UpgradeWrite 写入数据
bool UpgradeWrite(uint8_t* data, int size);

// UpgradeGetOffset 读取偏移地址
int UpgradeGetOffset(void);

// UpgradeEnd 结束升级.成功会自动重启加载新程序
bool UpgradeEnd(void);

// UpgradeAbort 中止升级
void UpgradeAbort(void);

// UpgradeIsBusy 是否升级中
bool UpgradeIsBusy(void);

#endif
