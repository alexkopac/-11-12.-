#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <windows.h>

using namespace std;
namespace fs = std::filesystem;

// Конвертація рядка UTF-8 у std::wstring для роботи з кириличними шляхами у Windows
wstring utf8ToWide(const string& str) {
    if (str.empty()) return wstring();
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), nullptr, 0);
    wstring result(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &result[0], sizeNeeded);
    return result;
}

// Конвертація std::wstring у рядок UTF-8 для виведення в консоль
string wideToUtf8(const wstring& wstr) {
    if (wstr.empty()) return string();
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), nullptr, 0, nullptr, nullptr);
    string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &result[0], sizeNeeded, nullptr, nullptr);
    return result;
}

// Перевірка відповідності імені файлу масці (підтримує '*' та '?')
bool matchMask(const string& filename, const string& mask) {
    size_t f = 0, m = 0;
    size_t starIdx = string::npos, matchIdx = 0;

    while (f < filename.size()) {
        if (m < mask.size() && (mask[m] == '?' || mask[m] == filename[f])) {
            f++;
            m++;
        } else if (m < mask.size() && mask[m] == '*') {
            starIdx = m;
            matchIdx = f;
            m++;
        } else if (starIdx != string::npos) {
            m = starIdx + 1;
            matchIdx++;
            f = matchIdx;
        } else {
            return false;
        }
    }

    while (m < mask.size() && mask[m] == '*') {
        m++;
    }

    return m == mask.size();
}

// Пошук слова всередині файлу
bool containsWord(const fs::path& filePath, const string& word, int& firstLineFound) {
    ifstream file(filePath);
    if (!file.is_open()) {
        return false;
    }

    string line;
    int lineNumber = 0;
    while (getline(file, line)) {
        lineNumber++;
        if (line.find(word) != string::npos) {
            firstLineFound = lineNumber;
            return true;
        }
    }
    return false;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    string inputPath, mask, searchWord;

    cout << "Введіть шлях до папки (наприклад, . або C:\\Folder): ";
    getline(cin, inputPath);

    cout << "Введіть маску файлів (наприклад, *.txt або task*.cpp): ";
    getline(cin, mask);

    cout << "Введіть слово для пошуку всередині файлів: ";
    getline(cin, searchWord);

    fs::path rootPath(utf8ToWide(inputPath));

    error_code ec;
    if (!fs::exists(rootPath, ec) || !fs::is_directory(rootPath, ec)) {
        cout << "\nПомилка: вказаний шлях не існує або не є папкою.\n";
        return 1;
    }

    cout << "\n=== Результати пошуку ===\n";
    int matchedFilesCount = 0;
    int filesWithWordCount = 0;

    for (const auto& entry : fs::recursive_directory_iterator(rootPath, fs::directory_options::skip_permission_denied, ec)) {
        if (entry.is_regular_file(ec)) {
            string fileName = wideToUtf8(entry.path().filename().wstring());

            if (matchMask(fileName, mask)) {
                matchedFilesCount++;
                string fullPathStr = wideToUtf8(entry.path().wstring());
                int lineNum = 0;

                if (containsWord(entry.path(), searchWord, lineNum)) {
                    filesWithWordCount++;
                    cout << "[ЗНАЙДЕНО]   " << fullPathStr << " (рядок " << lineNum << ")\n";
                } else {
                    cout << "[НЕМАЄ]      " << fullPathStr << "\n";
                }
            }
        }
    }

    cout << "\nЗнайдено файлів за маскою «" << mask << "»: " << matchedFilesCount << '\n';
    cout << "Із них містять слово «" << searchWord << "»: " << filesWithWordCount << '\n';

    return 0;
}