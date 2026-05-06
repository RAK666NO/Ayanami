// tests.cpp - Модульные тесты для проверки распознавания команд

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cassert>

using namespace std;

// ==================== ТЕСТИРУЕМЫЕ ФУНКЦИИ ====================

string ToLower(string str) {
    transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

bool ContainsAny(const string& text, const vector<string>& synonyms) {
    string lowerText = ToLower(text);
    for (const auto& synonym : synonyms) {
        if (lowerText.find(ToLower(synonym)) != string::npos) {
            return true;
        }
    }
    return false;
}

string GetTimeString() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Сейчас %d часов %d минут", ltm->tm_hour, ltm->tm_min);
    return string(buffer);
}

string GetDateString() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    const char* months[] = { "января", "февраля", "марта", "апреля", "мая", "июня",
                            "июля", "августа", "сентября", "октября", "ноября", "декабря" };
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Сегодня %d %s", ltm->tm_mday, months[ltm->tm_mon]);
    return string(buffer);
}

// ==================== ТЕСТЫ ====================

void TestGreetings() {
    cout << "\n[ТЕСТ 1] Проверка приветствий" << endl;

    vector<string> testPhrases = {
        "привет", "ПРИВЕТ", "ПрИвЕт", "здравствуй", "добрый день",
        "хай", "салют", "хеллоу", "дарова"
    };

    vector<string> synonyms = {
        "привет", "здравствуй", "добрый день", "доброе утро", "добрый вечер",
        "здорово", "хай", "салют", "приветик", "приветствую", "хеллоу", "дарова"
    };

    int passed = 0;
    for (const auto& phrase : testPhrases) {
        if (ContainsAny(phrase, synonyms)) {
            cout << "  [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testPhrases.size() << " пройдено" << endl;
    assert(passed == testPhrases.size());
}

void TestPauseCommands() {
    cout << "\n[ТЕСТ 2] Проверка команд паузы" << endl;

    vector<string> testPhrases = {
        "пауза", "стоп", "останови", "тишина", "замолчи",
        "поставь на паузу", "музыку стоп", "выключи звук"
    };

    vector<string> synonyms = {
        "пауза", "стоп", "останови", "тишина", "замолчи",
        "поставь на паузу", "на паузу", "музыку стоп", "выключи звук"
    };

    int passed = 0;
    for (const auto& phrase : testPhrases) {
        if (ContainsAny(phrase, synonyms)) {
            cout << "  [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testPhrases.size() << " пройдено" << endl;
    assert(passed == testPhrases.size());
}

void TestNextTrackCommands() {
    cout << "\n[ТЕСТ 3] Проверка команд следующей песни" << endl;

    vector<string> testPhrases = {
        "следующая", "дальше", "переключи", "скип", "следующую песню",
        "включи следующую", "следующий трек"
    };

    vector<string> synonyms = {
        "следующая", "следующая песня", "следующий трек", "дальше",
        "переключи", "скип", "пропусти", "включи следующую"
    };

    int passed = 0;
    for (const auto& phrase : testPhrases) {
        if (ContainsAny(phrase, synonyms)) {
            cout << "  [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testPhrases.size() << " пройдено" << endl;
    assert(passed == testPhrases.size());
}

void TestVolumeCommands() {
    cout << "\n[ТЕСТ 4] Проверка команд громкости" << endl;

    cout << "  Часть 1: Команды 'громче'" << endl;
    vector<string> volumeUpTest = { "громче", "прибавь звук", "увеличь громкость", "сделай громче" };
    vector<string> volumeUpSyn = { "громче", "прибавь звук", "увеличь громкость", "сделай громче" };

    int passed = 0;
    for (const auto& phrase : volumeUpTest) {
        if (ContainsAny(phrase, volumeUpSyn)) {
            cout << "    [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "    [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "    Результат: " << passed << " из " << volumeUpTest.size() << " пройдено" << endl;
    assert(passed == volumeUpTest.size());

    cout << "  Часть 2: Команды 'тише'" << endl;
    vector<string> volumeDownTest = { "тише", "убавь звук", "уменьши громкость", "сделай тише" };
    vector<string> volumeDownSyn = { "тише", "убавь звук", "уменьши громкость", "сделай тише" };

    passed = 0;
    for (const auto& phrase : volumeDownTest) {
        if (ContainsAny(phrase, volumeDownSyn)) {
            cout << "    [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "    [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "    Результат: " << passed << " из " << volumeDownTest.size() << " пройдено" << endl;
    assert(passed == volumeDownTest.size());
}

void TestPraiseCommands() {
    cout << "\n[ТЕСТ 5] Проверка команд похвалы" << endl;

    vector<string> testPhrases = {
        "ты умница", "молодец", "ты лучшая", "супер", "класс",
        "отлично", "прекрасно", "круто", "умничка"
    };

    vector<string> synonyms = {
        "умница", "молодец", "лучшая", "супер", "класс",
        "отлично", "прекрасно", "круто", "умничка", "браво"
    };

    int passed = 0;
    for (const auto& phrase : testPhrases) {
        if (ContainsAny(phrase, synonyms)) {
            cout << "  [OK] '" << phrase << "' - распознано" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testPhrases.size() << " пройдено" << endl;
    assert(passed == testPhrases.size());
}

void TestTimeFunction() {
    cout << "\n[ТЕСТ 6] Проверка функции получения времени" << endl;

    string timeStr = GetTimeString();
    cout << "  Результат: " << timeStr << endl;

    assert(!timeStr.empty());
    assert(timeStr.find("Сейчас") != string::npos);
    assert(timeStr.find("часов") != string::npos || timeStr.find("минут") != string::npos);

    cout << "  [OK] Функция GetTimeString() работает корректно" << endl;
}

void TestDateFunction() {
    cout << "\n[ТЕСТ 7] Проверка функции получения даты" << endl;

    string dateStr = GetDateString();
    cout << "  Результат: " << dateStr << endl;

    assert(!dateStr.empty());
    assert(dateStr.find("Сегодня") != string::npos);

    cout << "  [OK] Функция GetDateString() работает корректно" << endl;
}

void TestCaseInsensitivity() {
    cout << "\n[ТЕСТ 8] Проверка регистронезависимости" << endl;

    vector<string> synonyms = { "привет", "здравствуй" };
    vector<string> testCases = { "ПРИВЕТ", "ПрИвЕт", "ЗДРАВСТВУЙ", "Здравствуй" };

    int passed = 0;
    for (const auto& phrase : testCases) {
        if (ContainsAny(phrase, synonyms)) {
            cout << "  [OK] '" << phrase << "' - распознано (разный регистр)" << endl;
            passed++;
        }
        else {
            cout << "  [FAIL] '" << phrase << "' - НЕ распознано" << endl;
        }
    }
    cout << "  Результат: " << passed << " из " << testCases.size() << " пройдено" << endl;
    assert(passed == testCases.size());
}

void TestEmptyText() {
    cout << "\n[ТЕСТ 9] Проверка обработки пустого текста" << endl;

    vector<string> emptySynonyms;
    bool result = ContainsAny("", emptySynonyms);

    assert(result == false);
    cout << "  [OK] Пустой текст обрабатывается корректно" << endl;
}

void TestBoundaryConditions() {
    cout << "\n[ТЕСТ 10] Проверка граничных условий" << endl;

    vector<string> synonyms = { "команда" };

    string longText(10000, 'а');
    bool result = ContainsAny(longText, synonyms);
    assert(result == false);
    cout << "  [OK] Длинная строка (10000 символов) обработана" << endl;

    string specialText = "!@#$%^&*()";
    result = ContainsAny(specialText, synonyms);
    assert(result == false);
    cout << "  [OK] Специальные символы обработаны" << endl;
}

// ==================== MAIN ====================

int main() {
    cout << "========================================" << endl;
    cout << "   МОДУЛЬНЫЕ ТЕСТЫ АЯНАМИ АССИСТЕНТ" << endl;
    cout << "========================================" << endl;

    TestGreetings();
    TestPauseCommands();
    TestNextTrackCommands();
    TestVolumeCommands();
    TestPraiseCommands();
    TestTimeFunction();
    TestDateFunction();
    TestCaseInsensitivity();
    TestEmptyText();
    TestBoundaryConditions();

    cout << "\n========================================" << endl;
    cout << "   ВСЕ ТЕСТЫ УСПЕШНО ЗАВЕРШЕНЫ" << endl;
    cout << "========================================" << endl;

    return 0;
}