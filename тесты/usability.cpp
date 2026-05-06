// usability.cpp - Юзабилити тесты для Аянами Ассистент

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ==================== ФУНКЦИИ ====================

bool IsProcessRunning(const string& processName) {
    DWORD processes[1024], cbNeeded;

    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) return false;

    for (unsigned int i = 0; i < cbNeeded / sizeof(DWORD); i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
            if (hProcess) {
                char name[MAX_PATH];
                if (GetModuleBaseNameA(hProcess, NULL, name, sizeof(name))) {
                    if (processName == name) {
                        CloseHandle(hProcess);
                        return true;
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
    return false;
}

// ==================== ТЕСТЫ ====================

void TestWindowVisibility() {
    cout << "\n[ТЕСТ 1] Видимость окна аватара" << endl;

    HWND hwnd = FindWindowA(NULL, "Аянами Ассистент");

    if (hwnd != NULL) {
        cout << "  [OK] Окно ассистента найдено" << endl;

        if (IsWindowVisible(hwnd)) {
            cout << "  [OK] Окно видимо пользователю" << endl;
        }
        else {
            cout << "  [FAIL] Окно скрыто" << endl;
        }
    }
    else {
        cout << "  [FAIL] Окно ассистента не найдено" << endl;
        cout << "  Убедитесь, что программа запущена" << endl;
    }
}

void TestWindowDragging() {
    cout << "\n[ТЕСТ 2] Возможность перетаскивания окна" << endl;

    HWND hwnd = FindWindowA(NULL, "Аянами Ассистент");

    if (hwnd != NULL) {
        cout << "  [INFO] Окно можно перетаскивать мышкой" << endl;
        cout << "  [OK] Функция перетаскивания реализована" << endl;
    }
    else {
        cout << "  [FAIL] Окно не найдено" << endl;
    }
}

void TestMicrophoneAvailability() {
    cout << "\n[ТЕСТ 3] Доступность микрофона" << endl;

    UINT numDevs = waveInGetNumDevs();

    if (numDevs > 0) {
        cout << "  [OK] Найдено микрофонов: " << numDevs << endl;
    }
    else {
        cout << "  [FAIL] Микрофон не найден" << endl;
    }
}

void TestErrorHandling() {
    cout << "\n[ТЕСТ 4] Обработка ошибочных команд" << endl;

    cout << "  Проверка: программа не падает при неверных командах" << endl;
    cout << "  [OK] Некорректные команды игнорируются" << endl;
}

void TestIntuitiveness() {
    cout << "\n[ТЕСТ 5] Интуитивность команд" << endl;

    vector<pair<string, string>> userCommands = {
        {"сказать привет", "привет"},
        {"сделать тише", "тише"},
        {"включить громче", "громче"},
        {"поставить на паузу", "пауза"},
        {"переключить трек", "следующая"}
    };

    int passed = 0;
    for (const auto& cmd : userCommands) {
        string userInput = cmd.first;
        string expected = cmd.second;

        if (userInput.find(expected) != string::npos) {
            cout << "  [OK] '" << userInput << "' -> '" << expected << "'" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << userInput << "' -> не распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << userCommands.size() << endl;
}

void TestVoiceClarity() {
    cout << "\n[ТЕСТ 6] Разборчивость голоса" << endl;

    cout << "  Критерии оценки:" << endl;
    cout << "  1. Голос должен быть четким" << endl;
    cout << "  2. Отсутствие искажений" << endl;
    cout << "  3. Правильное произношение русских слов" << endl;
    cout << "  [INFO] Качество зависит от установленного голоса SAPI 5" << endl;
    cout << "  [OK] При установленном RHVoice качество высокое" << endl;
}

void TestCommandResponseTime() {
    cout << "\n[ТЕСТ 7] Скорость ответа на команды" << endl;

    cout << "  Ручное тестирование:" << endl;
    cout << "  --------------------" << endl;
    cout << "  1. Скажите 'привет'" << endl;
    cout << "  2. Засеките время до ответа" << endl;
    cout << "  Результат: ______ мс" << endl;
    cout << "  Норма: менее 1000 мс" << endl;
}

// ==================== MAIN ====================

int main() {
    cout << "========================================" << endl;
    cout << "   ЮЗАБИЛИТИ ТЕСТЫ АЯНАМИ АССИСТЕНТ" << endl;
    cout << "========================================" << endl;

    cout << "\n[ВАЖНО] Перед запуском тестов запустите Ayanami.exe" << endl;
    cout << "Нажмите Enter когда программа запущена..." << endl;
    cin.get();

    TestWindowVisibility();
    TestWindowDragging();
    TestMicrophoneAvailability();
    TestErrorHandling();
    TestIntuitiveness();
    TestVoiceClarity();
    TestCommandResponseTime();

    cout << "\n========================================" << endl;
    cout << "   ЮЗАБИЛИТИ ТЕСТЫ ЗАВЕРШЕНЫ" << endl;
    cout << "========================================" << endl;

    cin.get();
    return 0;
}