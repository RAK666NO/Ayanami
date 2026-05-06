// memory.cpp - Тестирование потребления памяти

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <psapi.h>
#include <iostream>
#include <string>

#pragma comment(lib, "psapi.lib")

using namespace std;

// ==================== ФУНКЦИИ ====================

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

// ==================== MAIN ====================

int main() {
    cout << "========================================" << endl;
    cout << "   ТЕСТИРОВАНИЕ ПОТРЕБЛЕНИЯ ПАМЯТИ" << endl;
    cout << "        АЯНАМИ АССИСТЕНТ" << endl;
    cout << "========================================" << endl;

    cout << "\n[ИНСТРУКЦИЯ]" << endl;
    cout << "1. Запустите Ayanami.exe вручную" << endl;
    cout << "2. Дождитесь полной загрузки" << endl;
    cout << "3. Нажмите Enter для замера" << endl;
    cin.get();

    DWORD pid = FindProcessId("Ayanami.exe");

    if (pid == 0) {
        cout << "\n[ОШИБКА] Аянами Ассистент не запущен!" << endl;
        cout << "Запустите программу и повторите тест." << endl;
        cin.get();
        return 1;
    }

    cout << "\n[OK] Процесс найден, ID: " << pid << endl;

    cout << "\n========================================" << endl;
    cout << "   РЕЗУЛЬТАТЫ ЗАМЕРОВ" << endl;
    cout << "========================================\n" << endl;

    SIZE_T mem1 = GetProcessMemoryMB(pid);
    cout << "  После загрузки модели: " << mem1 << " МБ" << endl;

    cout << "\n[ДЕЙСТВИЕ] Скажите любую команду (например 'привет')" << endl;
    cout << "Нажмите Enter после выполнения команды" << endl;
    cin.get();
    cin.get();

    SIZE_T mem2 = GetProcessMemoryMB(pid);
    cout << "  После распознавания команды: " << mem2 << " МБ" << endl;

    cout << "\n[ДЕЙСТВИЕ] Скажите длинную фразу (5-10 слов)" << endl;
    cout << "Нажмите Enter после выполнения" << endl;
    cin.get();

    SIZE_T mem3 = GetProcessMemoryMB(pid);
    cout << "  После длинной фразы: " << mem3 << " МБ" << endl;

    cout << "\n[ДЕЙСТВИЕ] Скажите 'пауза' или 'стоп'" << endl;
    cout << "Нажмите Enter после выполнения" << endl;
    cin.get();

    SIZE_T mem4 = GetProcessMemoryMB(pid);
    cout << "  После команды паузы: " << mem4 << " МБ" << endl;

    cout << "\n========================================" << endl;
    cout << "   ТАБЛИЦА ПОТРЕБЛЕНИЯ ПАМЯТИ" << endl;
    cout << "========================================\n" << endl;

    cout << "| Режим работы | Память (МБ) | Норма |" << endl;
    cout << "|--------------|-------------|-------|" << endl;
    cout << "| После загрузки модели | " << mem1 << " | < 150 МБ |" << endl;
    cout << "| После распознавания | " << mem2 << " | < 200 МБ |" << endl;
    cout << "| После длинной фразы | " << mem3 << " | < 200 МБ |" << endl;
    cout << "| После команды паузы | " << mem4 << " | < 200 МБ |" << endl;

    cout << "\n[НОРМЫ]" << endl;
    cout << "- Модель Vosk small: 50-80 МБ" << endl;
    cout << "- Стандартный голос SAPI: 30-50 МБ" << endl;
    cout << "- Ядро программы: 20-30 МБ" << endl;
    cout << "- ИТОГО: 100-160 МБ" << endl;

    cout << "\n========================================" << endl;
    cout << "   ТЕСТ ЗАВЕРШЕН" << endl;
    cout << "========================================" << endl;

    cin.get();
    return 0;
}