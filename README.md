# upgrade-esp32

## 1. 介绍
esp32的升级组件。

## 2. API
```c
// UpgradeBegin 开始升级
bool UpgradeBegin(void);

// UpgradeWrite 写入数据
bool UpgradeWrite(uint8_t* data, int size);

// UpgradeEnd 结束升级.成功会自动重启加载新程序
bool UpgradeEnd(void);

// UpgradeAbort 中止升级
void UpgradeAbort(void);
```
