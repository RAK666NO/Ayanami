// main.cpp - Аянами Ассистент
#define _CRT_SECURE_NO_WARNINGS
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN

#pragma warning(disable: 4996)

#include <windows.h>
#include <sapi.h>
#include <sphelper.h>
#include <string>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstring>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <ctime>

extern "C" {
#include "vosk_api.h"
}

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "sapi.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "libvosk.lib")

using namespace std;

// ==================== КОНСТАНТЫ ====================
constexpr int SAMPLE_RATE = 16000;
constexpr int FRAME_DURATION_MS = 20;
constexpr int FRAME_SIZE = SAMPLE_RATE * FRAME_DURATION_MS / 1000;
constexpr int WINDOW_WIDTH = 400;
constexpr int WINDOW_HEIGHT = 400;

// ==================== ГЛОБАЛЬНЫЕ ПЕРЕМЕННЫЕ ====================
atomic<bool> g_running(true);
queue<vector<short>> g_audioQueue;
mutex g_queueMutex;
condition_variable g_queueCV;
VoskModel* g_model = nullptr;
VoskRecognizer* g_recognizer = nullptr;
ISpVoice* g_pVoice = nullptr;
HWND g_hwnd = nullptr;
bool g_dragging = false;
POINT g_dragOffset;
USHORT g_currentVolume = 100;

// ==================== КОМАНДЫ ====================

// Похвала
vector<string> PRAISE_COMMANDS = {
    "ты умница", "умница", "ты молодец", "молодец",
    "ты хорошая", "хорошая", "ты лучшая", "лучшая",
    "отлично сработано", "прекрасно справляешься", "я тобой горжусь", "горжусь тобой",
    "ты крутая", "крутая", "ты классная", "классная",
    "ты замечательная", "замечательная", "ты прелесть", "прелесть",
    "аянами ты лучшая", "аянами умница", "аянами молодец",
    "спасибо ты очень помогла", "очень помогла", "хорошая работа",
    "отлично", "прекрасно", "великолепно", "супер", "класс", "круто",
    "умничка", "хорошая девочка", "так держать", "восхитительно",
    "браво", "потрясающе", "зашибись", "огонь", "клёво", "отпад"
};

vector<string> PRAISE_RESPONSES = {
    "Спасибо, мне очень приятно!",
    "Вы меня смущаете...",
    "Благодарю, я стараюсь",
    "Рада это слышать",
    "Ваша похвала вдохновляет меня",
    "Ой, спасибо! Я стараюсь быть полезной",
    "Приятно, что вы цените мою работу",
    "Спасибо! Вы очень добры",
    "Ваши слова греют мне душу",
    "Благодарю за тёплые слова!"
};

// Приветствия
vector<string> GREETINGS = {
    "привет", "здравствуй", "добрый день", "доброе утро", "добрый вечер",
    "здорово", "хай", "салют", "приветик", "приветствую", "здравия желаю",
    "ку", "дарова", "привет аянами", "аянами привет", "хеллоу", "здрасте"
};

// Как дела
vector<string> HOW_ARE_YOU = {
    "как дела", "как ты", "как жизнь", "как настроение", "как поживаешь",
    "как ты там", "как самочувствие", "как твои дела", "как успехи",
    "чё как", "что нового", "как живешь", "как сама", "как настроение"
};

// Прощание
vector<string> FAREWELLS = {
    "пока", "до свидания", "прощай", "до встречи", "выйти", "закрыть",
    "закройся", "завершить работу", "выход", "выключись", "отключись",
    "остановись", "хватит", "всё", "пока пока", "бай-бай", "до завтра",
    "увидимся", "счастливо", "всего хорошего"
};

// Пауза
vector<string> PAUSE_COMMANDS = {
    "пауза", "стоп", "останови", "прекрати", "замолчи", "тишина",
    "поставь на паузу", "на паузу", "сделай паузу", "поставь паузу",
    "остановить", "остановись", "перестань", "затихни",
    "поставь музыку на паузу", "останови музыку", "музыку стоп", "стоп музыка",
    "музыку на паузу", "пауза музыка", "выключи звук", "звук выключить"
};

// Продолжить
vector<string> RESUME_COMMANDS = {
    "продолжи", "играй дальше", "возобнови", "включи обратно",
    "продолжай", "дальше", "сними с паузы", "продолжить",
    "включи музыку обратно", "обратно включи"
};

// Следующая песня
vector<string> NEXT_TRACK_COMMANDS = {
    "следующая", "следующая песня", "следующий трек", "дальше", "переключи",
    "следующую", "следующую песню", "следующий", "дальше песню",
    "включи следующую", "переключи песню", "следующая музыка",
    "следующий трек", "следующая композиция", "скип", "пропусти"
};

// Громче
vector<string> VOLUME_UP_COMMANDS = {
    "громче", "прибавь звук", "увеличь громкость", "сделай громче",
    "прибавь громкость", "громкость выше", "громкости больше"
};

// Тише
vector<string> VOLUME_DOWN_COMMANDS = {
    "тише", "убавь звук", "уменьши громкость", "сделай тише",
    "убавь громкость", "громкость ниже", "громкости меньше"
};

// Что умеешь
vector<string> CAPABILITIES = {
    "что ты умеешь", "расскажи о себе", "кто ты", "представься", "что можешь",
    "твои возможности", "какие у тебя функции", "для чего ты"
};

// Как зовут
vector<string> ASK_NAME = {
    "как тебя зовут", "твое имя", "имя", "назови имя", "скажи имя",
    "как тебя называть", "кто такая аянами", "ты кто"
};

// Помощь
vector<string> HELP = {
    "помощь", "помоги", "инструкция", "подскажи", "что делать"
};

// Спасибо
vector<string> THANKS = {
    "спасибо", "благодарю", "мерси", "спасибо большое", "спс"
};

// Время
vector<string> TIME_QUERIES = {
    "сколько времени", "который час", "текущее время", "время сейчас"
};

// Дата
vector<string> DATE_QUERIES = {
    "какое сегодня число", "сегодняшняя дата", "какая дата", "какое число"
};

// ==================== ФУНКЦИИ ====================
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
    const char* months[] = { "января", "февраля", "марта", "апреля", "мая", "июня", "июля", "августа", "сентября", "октября", "ноября", "декабря" };
    char buffer[100];
    snprintf(buffer, sizeof(buffer), "Сегодня %d %s", ltm->tm_mday, months[ltm->tm_mon]);
    return string(buffer);
}

// ==================== УПРАВЛЕНИЕ ГРОМКОСТЬЮ ====================
void SetAyanamiVolume(int volume) {
    if (!g_pVoice) return;
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    g_currentVolume = (USHORT)volume;
    g_pVoice->SetVolume(g_currentVolume);
}

void VolumeUp() {
    int newVolume = g_currentVolume + 10;
    if (newVolume > 100) newVolume = 100;
    SetAyanamiVolume(newVolume);
}

void VolumeDown() {
    int newVolume = g_currentVolume - 10;
    if (newVolume < 0) newVolume = 0;
    SetAyanamiVolume(newVolume);
}

// ==================== УПРАВЛЕНИЕ МУЗЫКОЙ (ИСПРАВЛЕННОЕ) ====================

// Коды виртуальных клавиш для медиа-клавиш
#define VK_MEDIA_NEXT_TRACK 0xB0
#define VK_MEDIA_PREV_TRACK 0xB1
#define VK_MEDIA_PLAY_PAUSE 0xB3

// Отправка медиа-команды через keybd_event (надежный способ)
void SendMediaCommand(BYTE vkCode) {
    UINT scanCode = MapVirtualKey(vkCode, 0);
    keybd_event(vkCode, (BYTE)scanCode, 0, 0);
    keybd_event(vkCode, (BYTE)scanCode, KEYEVENTF_KEYUP, 0);
}

void PauseMusic() {
    SendMediaCommand(VK_MEDIA_PLAY_PAUSE);
}

void NextTrack() {
    SendMediaCommand(VK_MEDIA_NEXT_TRACK);
}

void PrevTrack() {
    SendMediaCommand(VK_MEDIA_PREV_TRACK);
}

// ==================== ОБРАБОТКА КОМАНД ====================
string ProcessCommand(const string& recognizedText) {
    // Похвала
    if (ContainsAny(recognizedText, PRAISE_COMMANDS)) {
        return PRAISE_RESPONSES[rand() % PRAISE_RESPONSES.size()];
    }
    // Приветствия
    else if (ContainsAny(recognizedText, GREETINGS)) {
        vector<string> responses = { "Привет, я Аянами", "Здравствуйте!", "Приветствую вас!", "Рада вас видеть!" };
        return responses[rand() % responses.size()];
    }
    // Как дела
    else if (ContainsAny(recognizedText, HOW_ARE_YOU)) {
        vector<string> responses = { "У меня всё отлично, спасибо", "Я в порядке, как вы?", "Всё замечательно, рада помочь" };
        return responses[rand() % responses.size()];
    }
    // Прощание
    else if (ContainsAny(recognizedText, FAREWELLS)) {
        g_running = false;
        PostQuitMessage(0);
        return "До свидания!";
    }
    // Пауза
    else if (ContainsAny(recognizedText, PAUSE_COMMANDS)) {
        PauseMusic();
        return "Пауза";
    }
    // Продолжить
    else if (ContainsAny(recognizedText, RESUME_COMMANDS)) {
        PauseMusic();  // Та же клавиша для паузы/продолжения
        return "Продолжаю";
    }
    // Следующая
    else if (ContainsAny(recognizedText, NEXT_TRACK_COMMANDS)) {
        NextTrack();
        return "Следующая";
    }
    // Громче
    else if (ContainsAny(recognizedText, VOLUME_UP_COMMANDS)) {
        VolumeUp();
        char buf[50];
        snprintf(buf, sizeof(buf), "Громкость %d", g_currentVolume);
        return string(buf);
    }
    // Тише
    else if (ContainsAny(recognizedText, VOLUME_DOWN_COMMANDS)) {
        VolumeDown();
        char buf[50];
        snprintf(buf, sizeof(buf), "Громкость %d", g_currentVolume);
        return string(buf);
    }
    // Что умеешь
    else if (ContainsAny(recognizedText, CAPABILITIES)) {
        return "Я умею разговаривать, отвечать на вопросы, ставить музыку на паузу, включать следующий трек, менять громкость, показывать время и дату. Моё окно можно перетаскивать мышкой. А ещё я люблю, когда меня хвалят!";
    }
    // Как зовут
    else if (ContainsAny(recognizedText, ASK_NAME)) {
        return "Меня зовут Аянами";
    }
    // Помощь
    else if (ContainsAny(recognizedText, HELP)) {
        return "Доступные команды: привет, как дела, пауза, продолжи, следующая, громче, тише, молодец, умница, что ты умеешь, как тебя зовут, сколько времени, какое сегодня число, спасибо, пока";
    }
    // Спасибо
    else if (ContainsAny(recognizedText, THANKS)) {
        return "Пожалуйста, всегда рада помочь";
    }
    // Время
    else if (ContainsAny(recognizedText, TIME_QUERIES)) {
        return GetTimeString();
    }
    // Дата
    else if (ContainsAny(recognizedText, DATE_QUERIES)) {
        return GetDateString();
    }
    return "";
}

// ==================== РИСОВАНИЕ АВАТАРА ====================
void DrawAvatar(HDC hdc, int width, int height) {
    RECT rect = { 0, 0, width, height };
    HBRUSH bgBrush = CreateSolidBrush(RGB(255, 200, 220));
    FillRect(hdc, &rect, bgBrush);
    DeleteObject(bgBrush);

    HBRUSH skinBrush = CreateSolidBrush(RGB(255, 220, 200));
    SelectObject(hdc, skinBrush);
    Ellipse(hdc, width / 4, height / 6, width * 3 / 4, height * 5 / 6);
    DeleteObject(skinBrush);

    HBRUSH hairBrush = CreateSolidBrush(RGB(40, 40, 80));
    SelectObject(hdc, hairBrush);
    Ellipse(hdc, width / 4 - 10, height / 6 - 10, width * 3 / 4 + 10, height / 3);
    DeleteObject(hairBrush);

    HBRUSH eyeBrush = CreateSolidBrush(RGB(30, 30, 120));
    SelectObject(hdc, eyeBrush);
    Ellipse(hdc, width / 2 - 60, height / 2 - 30, width / 2 - 20, height / 2 + 10);
    Ellipse(hdc, width / 2 + 20, height / 2 - 30, width / 2 + 60, height / 2 + 10);
    DeleteObject(eyeBrush);

    SelectObject(hdc, GetStockObject(WHITE_BRUSH));
    Ellipse(hdc, width / 2 - 50, height / 2 - 20, width / 2 - 35, height / 2);
    Ellipse(hdc, width / 2 + 35, height / 2 - 20, width / 2 + 50, height / 2);

    HBRUSH blushBrush = CreateSolidBrush(RGB(255, 150, 150));
    SelectObject(hdc, blushBrush);
    Ellipse(hdc, width / 2 - 80, height * 2 / 3, width / 2 - 40, height * 2 / 3 + 30);
    Ellipse(hdc, width / 2 + 40, height * 2 / 3, width / 2 + 80, height * 2 / 3 + 30);
    DeleteObject(blushBrush);

    HPEN pen = CreatePen(PS_SOLID, 2, RGB(200, 80, 80));
    SelectObject(hdc, pen);
    Arc(hdc, width / 2 - 30, height * 2 / 3 + 10, width / 2 + 30, height * 2 / 3 + 50,
        width / 2 - 20, height * 2 / 3 + 30, width / 2 + 20, height * 2 / 3 + 30);
    DeleteObject(pen);
}

// ==================== ОКНО ====================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        DrawAvatar(hdc, WINDOW_WIDTH, WINDOW_HEIGHT);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_LBUTTONDOWN: {
        g_dragging = true;
        SetCapture(hwnd);
        POINT pt;
        GetCursorPos(&pt);
        RECT rc;
        GetWindowRect(hwnd, &rc);
        g_dragOffset.x = pt.x - rc.left;
        g_dragOffset.y = pt.y - rc.top;
        break;
    }
    case WM_MOUSEMOVE:
        if (g_dragging && (wParam & MK_LBUTTON)) {
            POINT pt;
            GetCursorPos(&pt);
            SetWindowPos(hwnd, NULL, pt.x - g_dragOffset.x, pt.y - g_dragOffset.y,
                0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
        break;
    case WM_LBUTTONUP:
        if (g_dragging) {
            g_dragging = false;
            ReleaseCapture();
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

HWND CreateAvatarWindow() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = "AyanamiAvatar";

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        "AyanamiAvatar", "Аянами Ассистент", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );

    return hwnd;
}

// ==================== ЗАХВАТ ЗВУКА ====================
void AudioCaptureThread() {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    HWAVEIN hWaveIn;
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = 2;
    wfx.nAvgBytesPerSec = SAMPLE_RATE * 2;

    if (waveInOpen(&hWaveIn, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        CoUninitialize();
        return;
    }

    const int NUM_BUFFERS = 4;
    vector<short> buffers[NUM_BUFFERS];
    WAVEHDR headers[NUM_BUFFERS];

    for (int i = 0; i < NUM_BUFFERS; i++) {
        buffers[i].resize(FRAME_SIZE);
        headers[i].lpData = (LPSTR)buffers[i].data();
        headers[i].dwBufferLength = FRAME_SIZE * sizeof(short);
        headers[i].dwFlags = 0;
        waveInPrepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
        waveInAddBuffer(hWaveIn, &headers[i], sizeof(WAVEHDR));
    }

    waveInStart(hWaveIn);

    while (g_running) {
        Sleep(FRAME_DURATION_MS);

        for (int i = 0; i < NUM_BUFFERS; i++) {
            if (headers[i].dwFlags & WHDR_DONE) {
                {
                    lock_guard<mutex> lock(g_queueMutex);
                    if (g_audioQueue.size() < 20) {
                        g_audioQueue.push(buffers[i]);
                    }
                }
                g_queueCV.notify_one();

                waveInUnprepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
                headers[i].dwFlags = 0;
                waveInPrepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
                waveInAddBuffer(hWaveIn, &headers[i], sizeof(WAVEHDR));
            }
        }
    }

    waveInStop(hWaveIn);
    for (int i = 0; i < NUM_BUFFERS; i++) {
        waveInUnprepareHeader(hWaveIn, &headers[i], sizeof(WAVEHDR));
    }
    waveInClose(hWaveIn);
    CoUninitialize();
}

string GetExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    string path(buffer);
    size_t pos = path.find_last_of("\\/");
    return path.substr(0, pos + 1);
}

// ==================== ВЫБОР ГОЛОСА ====================
void SetRussianVoice() {
    if (!g_pVoice) return;

    CComPtr<IEnumSpObjectTokens> cpEnum;
    HRESULT hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);

    if (SUCCEEDED(hr)) {
        CComPtr<ISpObjectToken> cpVoiceToken;
        ULONG ulCount = 0;

        while (cpEnum->Next(1, &cpVoiceToken, &ulCount) == S_OK && ulCount > 0) {
            CComPtr<ISpDataKey> cpAttributes;
            cpVoiceToken->OpenKey(L"Attributes", &cpAttributes);

            WCHAR* pszLanguage = NULL;
            if (cpAttributes) {
                cpAttributes->GetStringValue(L"Language", &pszLanguage);
            }

            if (pszLanguage) {
                wstring lang(pszLanguage);
                if (lang.find(L"419") != wstring::npos || lang.find(L"RU") != wstring::npos) {
                    g_pVoice->SetVoice(cpVoiceToken);
                    CoTaskMemFree(pszLanguage);
                    return;
                }
                CoTaskMemFree(pszLanguage);
            }
            cpVoiceToken.Release();
        }
    }
}

void Speak(const string& text) {
    if (!g_pVoice) return;

    int wideSize = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, NULL, 0);
    wstring wtext(wideSize, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wtext[0], wideSize);
    wtext.pop_back();

    g_pVoice->Speak(wtext.c_str(), 0, NULL);
}

// ==================== РАСПОЗНАВАНИЕ ====================
void VoiceRecognitionThread() {
    string exePath = GetExePath();
    string modelPath = exePath + "models/vosk-model-small-ru-0.22";

    g_model = vosk_model_new(modelPath.c_str());
    if (!g_model) {
        g_model = vosk_model_new("models/vosk-model-small-ru-0.22");
    }

    if (!g_model) {
        return;
    }

    srand((unsigned)time(NULL));
    g_recognizer = vosk_recognizer_new(g_model, (float)SAMPLE_RATE);
    SetAyanamiVolume(100);
    Speak("Аянами активирована. Я слушаю вас");

    while (g_running) {
        vector<short> audioData;
        {
            unique_lock<mutex> lock(g_queueMutex);
            if (g_audioQueue.empty()) {
                g_queueCV.wait_for(lock, chrono::milliseconds(50));
                continue;
            }
            audioData = g_audioQueue.front();
            g_audioQueue.pop();
        }

        int result = vosk_recognizer_accept_waveform(g_recognizer, (const char*)audioData.data(),
            (int)(audioData.size() * sizeof(short)));

        if (result == 1) {
            const char* finalResult = vosk_recognizer_result(g_recognizer);
            if (finalResult) {
                string resultStr(finalResult);

                size_t pos = resultStr.find("\"text\"");
                if (pos != string::npos) {
                    size_t q1 = resultStr.find("\"", resultStr.find(":", pos));
                    if (q1 != string::npos) {
                        size_t q2 = resultStr.find("\"", q1 + 1);
                        if (q2 != string::npos) {
                            string recognizedText = resultStr.substr(q1 + 1, q2 - q1 - 1);

                            if (!recognizedText.empty() && recognizedText.length() > 1) {
                                string response = ProcessCommand(recognizedText);
                                if (!response.empty()) {
                                    Speak(response);
                                }
                            }
                        }
                    }
                }
            }
        }

        this_thread::sleep_for(chrono::milliseconds(10));
    }

    if (g_recognizer) vosk_recognizer_free(g_recognizer);
    if (g_model) vosk_model_free(g_model);
}

// ==================== MAIN ====================
int main() {
    SetConsoleOutputCP(CP_UTF8);

    CoInitialize(NULL);

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&g_pVoice);
    if (FAILED(hr) || !g_pVoice) {
        CoUninitialize();
        return 1;
    }

    SetRussianVoice();
    g_pVoice->SetRate(0);

    g_hwnd = CreateAvatarWindow();
    if (!g_hwnd) {
        g_pVoice->Release();
        CoUninitialize();
        return 1;
    }

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    thread captureThread(AudioCaptureThread);
    thread recognitionThread(VoiceRecognitionThread);

    MSG msg;
    while (g_running && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    g_running = false;
    captureThread.join();
    recognitionThread.join();

    g_pVoice->Release();
    CoUninitialize();

    return 0;
}