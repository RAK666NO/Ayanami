// validation.cpp - Валидация входных данных

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <direct.h>

using namespace std;

// ==================== ФУНКЦИИ ДЛЯ ПРОВЕРКИ ====================

bool FileExists(const string& path) {
    ifstream file(path.c_str());
    return file.good();
}

bool DirectoryExists(const string& path) {
    ifstream file(path.c_str());
    return file.good();
}

string GetProgramRoot() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    string path(buffer);
    size_t pos = path.find_last_of("\\/");
    return path.substr(0, pos + 1);
}

bool ValidateText(const string& text) {
    if (text.empty()) return false;
    if (text.length() > 1000) return false;
    return true;
}

bool ValidateCommand(const string& command) {
    if (!ValidateText(command)) return false;

    for (char c : command) {
        if (!isalpha((unsigned char)c) && c != ' ' && c != '-' && c != '?' && c != '!') {
            return false;
        }
    }
    return true;
}

// ==================== ТЕСТЫ ВАЛИДАЦИИ ====================

void TestProgramRoot() {
    cout << "\n[ТЕСТ 1] Проверка корня программы" << endl;

    string root = GetProgramRoot();
    cout << "  Корень программы: " << root << endl;

    if (!root.empty()) {
        cout << "  [OK] Корень программы определен" << endl;
    }
    else {
        cout << "  [FAIL] Корень программы не определен" << endl;
    }
}

void TestModelsFolder() {
    cout << "\n[ТЕСТ 2] Проверка папки models" << endl;

    string root = GetProgramRoot();
    string modelsPath = root + "models";

    cout << "  Путь к models: " << modelsPath << endl;

    if (DirectoryExists(modelsPath)) {
        cout << "  [OK] Папка models существует" << endl;

        string modelPath = modelsPath + "\\vosk-model-small-ru-0.22";
        if (DirectoryExists(modelPath)) {
            cout << "  [OK] Модель Vosk найдена" << endl;
        }
        else {
            cout << "  [WARN] Модель Vosk не найдена" << endl;
        }
    }
    else {
        cout << "  [WARN] Папка models не найдена" << endl;
    }
}

void TestEmptyText() {
    cout << "\n[ТЕСТ 3] Валидация пустого текста" << endl;

    string emptyText = "";
    string normalText = "привет";

    if (!ValidateText(emptyText)) {
        cout << "  [OK] Пустой текст отвергнут" << endl;
    }
    else {
        cout << "  [FAIL] Пустой текст принят" << endl;
    }

    if (ValidateText(normalText)) {
        cout << "  [OK] Нормальный текст принят" << endl;
    }
    else {
        cout << "  [FAIL] Нормальный текст отвергнут" << endl;
    }
}

void TestLongText() {
    cout << "\n[ТЕСТ 4] Валидация длинного текста" << endl;

    string longText(2000, 'a');

    if (!ValidateText(longText)) {
        cout << "  [OK] Длинный текст (2000) отвергнут" << endl;
    }
    else {
        cout << "  [FAIL] Длинный текст принят" << endl;
    }
}

void TestInvalidCharacters() {
    cout << "\n[ТЕСТ 5] Валидация недопустимых символов" << endl;

    vector<pair<string, bool>> testCases = {
        {"привет", true},
        {"привет как дела", true},
        {"123", false},
        {"@#$%", false},
        {"привет123", false},
        {"hello", false},
        {"команда-пауза", true},
        {"", false}
    };

    int passed = 0;
    for (const auto& test : testCases) {
        bool result = ValidateCommand(test.first);
        bool expected = test.second;

        if (result == expected) {
            cout << "  [OK] '" << test.first << "' - " << (result ? "принято" : "отвергнуто") << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << test.first << "' - ошибка" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testCases.size() << endl;
}

void TestSpecialSymbols() {
    cout << "\n[ТЕСТ 6] Валидация спецсимволов" << endl;

    vector<string> testCases = { "пауза!", "привет?", "как-дела", "стоп." };

    int passed = 0;
    for (const auto& cmd : testCases) {
        if (ValidateCommand(cmd)) {
            cout << "  [OK] '" << cmd << "' - принято" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << cmd << "' - отвергнуто" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testCases.size() << endl;
}

void TestExeExists() {
    cout << "\n[ТЕСТ 7] Проверка наличия EXE" << endl;

    string root = GetProgramRoot();
    string exePath = root + "Ayanami.exe";

    cout << "  Путь: " << exePath << endl;

    if (FileExists(exePath)) {
        cout << "  [OK] Ayanami.exe найден" << endl;
    }
    else {
        cout << "  [WARN] Ayanami.exe не найден" << endl;
    }
}

// ==================== MAIN ====================

int main() {
    cout << "========================================" << endl;
    cout << "   ВАЛИДАЦИЯ АЯНАМИ АССИСТЕНТ" << endl;
    cout << "========================================" << endl;

    TestProgramRoot();
    TestModelsFolder();
    TestExeExists();
    TestEmptyText();
    TestLongText();
    TestInvalidCharacters();
    TestSpecialSymbols();

    cout << "\n========================================" << endl;
    cout << "   ВАЛИДАЦИЯ ЗАВЕРШЕНА" << endl;
    cout << "========================================" << endl;

    return 0;
}