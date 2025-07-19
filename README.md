```markdown
# 屏幕截图工具说明文档

## 功能概述

本工具是一个轻量级的屏幕截图实用程序，具有以下功能：

- 使用热键快速截取全屏
- 支持 BMP 和 JPEG 两种图片格式
- 自动保存截图到桌面
- 开机自启动功能
- 详细的日志记录

## 系统要求

- Windows 7 及以上操作系统
- 支持 GDI+的显示适配器

## 安装与使用

### 首次运行

1. 直接运行程序，会自动：
   - 创建日志文件在桌面（`log_YYYYMMDD_HHMMSS.log`）
   - 将自身添加到启动项（`C:\Users\[用户名]\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\`）

### 热键说明

| 热键组合  | 功能                 |
| --------- | -------------------- |
| Shift + 1 | 截取全屏并保存到桌面 |
| Shift + 2 | 切换 BMP/JPEG 格式   |
| ESC       | 退出程序             |

### 截图保存

截图会自动保存到桌面，文件名格式为：
```

screenshot_YYYYMMDD_HHMMSS.[bmp|jpg]

````

## 配置选项
程序通过修改源代码中的全局配置进行定制：

```c
// 文件配置
const char* LOG_PREFIX = "log_";
const char* SCREENSHOT_PREFIX = "screenshot_";
const char* BMP_EXTENSION = ".bmp";
const char* JPEG_EXTENSION = ".jpg";

// 路径配置
const char* DESKTOP_PATH = "C:\\Users\\%s\\Desktop\\";
const char* STARTUP_PATH = "C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\";

// 热键配置
const int HOTKEY_MODIFIER = VK_SHIFT;  // 修饰键
const int HOTKEY_MAIN = 0x31;         // 主键 (1键)
const int EXIT_KEY = VK_ESCAPE;       // 退出键

// 程序行为
const int KEY_CHECK_INTERVAL = 100;   // 热键检测间隔(ms)
const int SCREENSHOT_DELAY = 500;     // 截图防误触延迟(ms)
````

## 技术细节

- 使用 GDI 进行屏幕捕获
- 支持多显示器环境
- 实现 DPI 感知
- 使用互斥体确保单实例运行
- 详细的日志记录系统

## 编译说明

需要链接以下库：

- Shcore.lib (DPI 感知)
- Gdiplus.lib (JPEG 支持)

编译命令示例：

```
cl screenshot_utility.cpp /link Shcore.lib Gdiplus.lib
```

[build.sh](build.sh) 是在 mac 下进行交叉编译 Win11 平台的命令

## 常见问题

**Q: 截图保存失败怎么办？**
A: 检查：

1. 桌面目录是否可写
2. 磁盘空间是否充足
3. 查看日志文件中的错误信息

**Q: 如何禁用开机启动？**
A: 删除启动目录中的程序快捷方式：

```
C:\Users\[用户名]\AppData\Roaming\Microsoft\Windows\Start Menu\Programs\Startup\screenshot_utility.exe
```

## 版本历史

- v1.0 初始版本
  - 基本截图功能
  - BMP 格式支持
- v1.1 新增功能
  - 添加 JPEG 格式支持
  - 改进日志系统
  - 添加 DPI 感知

## 许可证

MIT License

```

```
