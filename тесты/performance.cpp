// performance.cpp - Тестирование производительности

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>
#include <vector>
#include <chrono>

#pragma comment(lib, "psapi.lib")

using namespace std;
using namespace chrono;

// ==================== ФУНКЦИИ ====================

double GetProcessCPUUsage(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) return 0.0;

    FILETIME ftSysIdle, ftSysKernel, ftSysUser;
    FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

    GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser);
    GetProcessTimes(hProcess, &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser);

    CloseHandle(hProcess);

    ULARGE_INTEGER sysKernel, sysUser, procKernel, procUser;
    sysKernel.LowPart = ftSysKernel.dwLowDateTime;
    sysKernel.HighPart = ftSysKernel.dwHighDateTime;
    sysUser.LowPart = ftSysUser.dwLowDateTime;
    sysUser.HighPart = ftSysUser.dwHighDateTime;
    procKernel.LowPart = ftProcKernel.dwLowDateTime;
    procKernel.HighPart = ftProcKernel.dwHighDateTime;
    procUser.LowPart = ftProcUser.dwLowDateTime;
    procUser.HighPart = ftProcUser.dwHighDateTime;

    ULONGLONG sysTotal = sysKernel.QuadPart + sysUser.QuadPart;
    ULONGLONG procTotal = procKernel.QuadPart + procUser.QuadPart;

    if (sysTotal == 0) return 0.0;
    return (double)procTotal / (double)sysTotal * 100.0;
}

SIZE_T GetProcessMemoryMB(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) return 0;

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
        CloseHandle(hProcess);
        return pmc.WorkingSetSize / (1024 * 1024);
    }

    CloseHandle(hProcess);
    return 0;
}

DWORD FindProcessId(const string& processName) {
    DWORD processes[1024], cbNeeded, pid = 0;

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) return 0;

    for (unsigned int i = 0; i < cbNeeded / sizeof(DWORD); i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) {
                char name[MAX_PATH];
                if (GetModuleBaseNameA(hProcess, NULL, name, sizeof(name))) {
                    if (processName == name) {
                        pid = processes[i];
                        CloseHandle(hProcess);
                        break;
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    return pid;
}

double MeasureTime(void (*func)()) {
    auto start = high_resolution_clock::now();
    func();
    auto end = high_resolution_clock::now();
    return duration<double, milli>(end - start).count();
}

// ==================== ТЕСТЫ ====================

void TestStartupTime() {
    cout << "\n[ТЕСТ 1] Время запуска программы" << endl;

    vector<double> times;

    for (int i = 0; i < 3; i++) {
        auto start = high_resolution_clock::now();

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;

        if (CreateProcessA("x64\\Debug\\Ayanami.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            for (int j = 0; j < 50; j++) {
                HWND hwnd = FindWindowA(NULL, "Аянами Ассистент");
                if (hwnd) break;
                Sleep(100);
            }

            auto end = high_resolution_clock::now();
            double ms = duration<double, milli>(end - start).count();
            times.push_back(ms);
            cout << "  Попытка " << (i + 1) << ": " << ms << " мс" << endl;

            TerminateProcess(pi.hProcess, 0);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
            Sleep(500);
        }
        else {
            cout << "  [FAIL] Не удалось запустить программу" << endl;
        }
    }

    if (!times.empty()) {
        double avg = 0;
        for (double t : times) avg += t;
        avg /= times.size();
        cout << "  Среднее время: " << avg << " мс" << endl;
    }
}

void TestMemoryUsage() {
    cout << "\n[ТЕСТ 2] Потребление памяти" << endl;

    DWORD pid = FindProcessId("Ayanami.exe");

    if (pid) {
        SIZE_T memoryMB = GetProcessMemoryMB(pid);
        cout << "  Потребление памяти: " << memoryMB << " МБ" << endl;

        if (memoryMB < 200) {
            cout << "  [OK] Норма (менее 200 МБ)" << endl;
        }
        else {
            cout << "  [WARN] Высокое потребление памяти" << endl;
        }
    }
    else {
        cout << "  [FAIL] Процесс не найден" << endl;
    }
}

void TestCPUUsage() {
    cout << "\n[ТЕСТ 3] Загрузка CPU" << endl;

    DWORD pid = FindProcessId("Ayanami.exe");

    if (pid) {
        double cpu = GetProcessCPUUsage(pid);
        cout << "  Загрузка CPU в режиме ожидания: " << cpu << "%" << endl;

        if (cpu < 5.0) {
            cout << "  [OK] Норма (менее 5%)" << endl;
        }
        else {
            cout << "  [WARN] Высокая загрузка CPU" << endl;
        }
    }
    else {
        cout << "  [FAIL] Процесс не найден" << endl;
    }
}

void TestCommandSpeed() {
    cout << "\n[ТЕСТ 4] Скорость выполнения команд" << endl;

    cout << "  [INFO] Для точного замера запустите ассистента вручную" << endl;
    cout << "  [INFO] И произнесите команды с секундомером" << endl;
    cout << "" << endl;
    cout << "  Ожидаемые нормы:" << endl;
    cout << "  - Команда 'привет': менее 500 мс" << endl;
    cout << "  - Команда 'пауза': менее 300 мс" << endl;
    cout << "  - Команда 'громче': менее 300 мс" << endl;
}

void TestModelLoadTime() {
    cout << "\n[ТЕСТ 5] Время загрузки модели Vosk" << endl;

    cout << "  [INFO] Модель загружается при запуске программы" << endl;
    cout << "  [INFO] Обычное время загрузки: 2-5 секунд" << endl;
}

void TestResponseTime() {
    cout << "\n[ТЕСТ 6] Время ответа ассистента" << endl;

    cout << "  Ручное тестирование:" << endl;
    cout << "  --------------------" << endl;
    cout << "  1. Скажите 'привет'" << endl;
    cout << "  2. Засеките время от окончания фразы до ответа" << endl;
    cout << "  3. Запишите результат" << endl;
    cout << "" << endl;
    cout << "  Результат: ______ мс" << endl;
    cout << "  Норма: менее 1000 мс" << endl;
}

void TestCPUUnderLoad() {
    cout << "\n[ТЕСТ 7] Загрузка CPU при нагрузке" << endl;

    cout << "  Ручное тестирование:" << endl;
    cout << "  --------------------" << endl;
    cout << "  1. Откройте Диспетчер задач (Ctrl+Shift+Esc)" << endl;
    cout << "  2. Скажите длинную фразу" << endl;
    cout << "  3. Запишите загрузку CPU во время распознавания" << endl;
    cout << "" << endl;
    cout << "  Результат: ______ %" << endl;
    cout << "  Норма: менее 30%" << endl;
}

// ==================== MAIN ====================

int main() {
    cout << "========================================" << endl;
    cout << "   ТЕСТИРОВАНИЕ ПРОИЗВОДИТЕЛЬНОСТИ" << endl;
    cout << "        АЯНАМИ АССИСТЕНТ" << endl;
    cout << "========================================" << endl;

    TestStartupTime();
    TestMemoryUsage();
    TestCPUUsage();
    TestModelLoadTime();
    TestCommandSpeed();

    cout << "\n========================================" << endl;
    cout << "   ТЕСТЫ ТРЕБУЮЩИЕ РУЧНОГО ЗАМЕРА" << endl;
    cout << "========================================" << endl;

    TestResponseTime();
    TestCPUUnderLoad();

    cout << "\n========================================" << endl;
    cout << "   ТЕСТИРОВАНИЕ ЗАВЕРШЕНО" << endl;
    cout << "========================================" << endl;

    return 0;
}