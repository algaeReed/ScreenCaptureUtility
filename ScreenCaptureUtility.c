#include <windows.h>
#include <shellscalingapi.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#pragma comment(lib, "Shcore.lib")  // 用于 SetProcessDpiAwareness

FILE* logFile = NULL;

void Log(const char* message) {
    time_t now;
    time(&now);
    char timeStr[64];
    strftime(timeStr, sizeof(timeStr), "[%Y-%m-%d %H:%M:%S] ", localtime(&now));

    if (logFile) {
        fprintf(logFile, "%s%s\n", timeStr, message);
        fflush(logFile);
    }
}

#define IS_PRESSED(x) (GetAsyncKeyState(x) & 0x8000)

void GenerateFilename(char* filename, const char* extension) {
    time_t now;
    struct tm* timeinfo;
    time(&now);
    timeinfo = localtime(&now);

    sprintf(filename, "C:\\Users\\%s\\Desktop\\screenshot_%04d%02d%02d_%02d%02d%02d%s",
            getenv("USERNAME"),
            timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1,
            timeinfo->tm_mday,
            timeinfo->tm_hour,
            timeinfo->tm_min,
            timeinfo->tm_sec,
            extension);
}

BOOL InitLogFile() {
    char logPath[MAX_PATH];
    GenerateFilename(logPath, ".log");

    char* pos = strstr(logPath, "screenshot_");
    if (pos) memcpy(pos, "log_", 4);

    logFile = fopen(logPath, "a");
    if (!logFile) return FALSE;

    Log("日志文件初始化完成");
    return TRUE;
}

void SetDpiAwareness() {
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);
}

BOOL TakeScreenshot() {
    Log("开始截图...");

    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    Log("获取虚拟屏幕尺寸完成");

    HDC hScreen = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    SelectObject(hMemDC, hBitmap);

    if (!BitBlt(hMemDC, 0, 0, width, height, hScreen, x, y, SRCCOPY)) {
        Log("屏幕捕获失败");
        ReleaseDC(NULL, hScreen);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        return FALSE;
    }

    Log("屏幕捕获成功");

    char filename[MAX_PATH];
    GenerateFilename(filename, ".bmp");

    BITMAPFILEHEADER bmfHeader = {0};
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    DWORD dwBmpSize = (((width * 24 + 31) & ~31) / 8) * height;

    HANDLE hFile = CreateFileA(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        Log("创建输出文件失败");
        ReleaseDC(NULL, hScreen);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        return FALSE;
    }

    Log("输出文件创建成功");

    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmpSize;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    DWORD dwWritten;
    WriteFile(hFile, &bmfHeader, sizeof(bmfHeader), &dwWritten, NULL);
    WriteFile(hFile, &bi, sizeof(bi), &dwWritten, NULL);

    char* pixels = (char*)malloc(dwBmpSize);
    if (!pixels) {
        Log("内存分配失败");
        CloseHandle(hFile);
        ReleaseDC(NULL, hScreen);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        return FALSE;
    }

    if (!GetDIBits(hScreen, hBitmap, 0, height, pixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        Log("获取位图数据失败");
        free(pixels);
        CloseHandle(hFile);
        ReleaseDC(NULL, hScreen);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
        return FALSE;
    }

    WriteFile(hFile, pixels, dwBmpSize, &dwWritten, NULL);
    free(pixels);
    CloseHandle(hFile);

    char logMsg[256];
    sprintf(logMsg, "截图已保存: %s", filename);
    Log(logMsg);

    ReleaseDC(NULL, hScreen);
    DeleteDC(hMemDC);
    DeleteObject(hBitmap);
    return TRUE;
}

void SetAutoStart() {
    char exePath[MAX_PATH];
    char startupPath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    sprintf(startupPath, "C:\\Users\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\screenshot_helper.exe",
            getenv("USERNAME"));

    if (strcmp(exePath, startupPath) != 0) {
        if (CopyFileA(exePath, startupPath, FALSE)) {
            Log("已添加到开机启动");
        } else {
            Log("添加到开机启动失败");
        }
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    SetDpiAwareness();  // ✅ 关键：设置 DPI 感知，获取真实屏幕尺寸

    if (!InitLogFile()) return 1;

    Log("程序启动");
    SetAutoStart();

    HANDLE hMutex = CreateMutexA(NULL, FALSE, "Global\\ScreenshotApp");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Log("程序已在运行中");
        if (logFile) fclose(logFile);
        return 0;
    }

    Log("等待热键 Shift+1...");
    while (1) {
        if (IS_PRESSED(VK_SHIFT) && IS_PRESSED(0x31)) {
            Log("检测到热键 Shift+1");
            TakeScreenshot();
            Sleep(500);
        }

        if (IS_PRESSED(VK_ESCAPE)) {
            Log("检测到 ESC 键，退出程序");
            break;
        }

        Sleep(100);
    }

    ReleaseMutex(hMutex);
    CloseHandle(hMutex);
    if (logFile) fclose(logFile);
    return 0;
}