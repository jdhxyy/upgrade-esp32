// Copyright 2021-2021 The jdh99 Authors. All rights reserved.
// ����ģ��
// Authors: jdh99 <jdh821@163.com>

#ifndef UPGRADE_H
#define UPGRADE_H

#include <stdint.h>
#include <stdbool.h>

// UpgradeBegin ��ʼ����
bool UpgradeBegin(void);

// UpgradeWrite д������
bool UpgradeWrite(uint8_t* data, int size);

// UpgradeEnd ��������.�ɹ����Զ����������³���
bool UpgradeEnd(void);

// UpgradeAbort ��ֹ����
void UpgradeAbort(void);

#endif
