#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <windows.h>
#include <winbase.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_PATH_SIZE 512
#define MAX_USERNAME_SIZE 256

// pobiera aktualną datę w formacie "rok_dzień.miesiąc"
void getDate(char* date, size_t size) {
    time_t t = time(NULL);
    struct tm tm;
    localtime_s(&tm, &t);
    snprintf(date, size, "%d_%02d.%02d", tm.tm_year + 1900, tm.tm_mday, tm.tm_mon + 1);
}

// pobiera nazwę użytkownika systemu
void getUser(char* username, DWORD* size) {
    if (!GetUserNameA(username, size)) {
        printf("błąd pobierania nazwy użytkownika: %ld\n", GetLastError());
        strcpy_s(username, *size, "unknown");
    }
}

// wyświetla dostępne tabele w bazie sqlite
void checkTables(sqlite3* db) {
    sqlite3_stmt* stmt;
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table';";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK) {
        printf("błąd zapytania: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("dostępne tabele:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%s\n", (const char*)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);
}

// określa przeglądarkę na podstawie ProgID
const char* detectBrowser(const char* progId) {
    if (_strnicmp(progId, "Chrome", 6) == 0) return "Chrome";
    if (_strnicmp(progId, "Firefox", 7) == 0) return "Firefox";
    if (_strnicmp(progId, "msedge", 6) == 0) return "Edge";
    if (_strnicmp(progId, "opera", 5) == 0) return "Opera";
    if (_strnicmp(progId, "brave", 5) == 0) return "Brave";
    return "Unknown";
}

// pobiera ścieżkę do domyślnego profilu Firefoxa
void getFirefoxProfile(char* browserPath, size_t size, const char* username) {
    char profilesIniPath[MAX_PATH_SIZE];
    snprintf(profilesIniPath, sizeof(profilesIniPath), "C:\\Users\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\profiles.ini", username);

    FILE* iniFile;
    if (fopen_s(&iniFile, profilesIniPath, "r") != 0 || iniFile == NULL) {
        printf("nie można otworzyć profiles.ini.\n");
        return;
    }

    char line[256], defaultProfile[256] = "";
    char currentPath[256] = "";

    while (fgets(line, sizeof(line), iniFile)) {
        if (strncmp(line, "[", 1) == 0) {
            currentPath[0] = '\0';
        }
        else if (strncmp(line, "Path=", 5) == 0) {
            sscanf_s(line, "Path=%255s", currentPath, (unsigned)_countof(currentPath));
        }
        else if (strncmp(line, "Default=1", 9) == 0) {
            if (currentPath[0] != '\0') {
                currentPath[0] = '\0';
            }
        }
        else if (currentPath[0] != '\0') {
            char* profilesPos = strstr(currentPath, "Profiles/");
            if (profilesPos != NULL) {
                memmove(currentPath, profilesPos + strlen("Profiles/"), strlen(profilesPos + strlen("Profiles/")) + 1);
            }
            strcpy_s(defaultProfile, sizeof(defaultProfile), currentPath);
            break;
        }
    }
    fclose(iniFile);

    if (strlen(defaultProfile) > 0) {
        snprintf(browserPath, size, "C:\\Users\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\places.sqlite", username, defaultProfile);
    }
    else {
        printf("nie znaleziono domyślnego profilu Firefoxa.\n");
    }
}

// pobiera lokalizację pliku historii przeglądarki
void getLocation(char* browserPath, size_t pathSize, const char* username, char* browserType) {
    HKEY hKey;
    char progId[MAX_PATH_SIZE];
    DWORD bufferSize = sizeof(progId);
    LONG result;

    result = RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\.html\\UserChoice", 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        printf("błąd otwierania rejestru: %ld\n", result);
        return;
    }

    result = RegQueryValueExA(hKey, "ProgId", NULL, NULL, (LPBYTE)progId, &bufferSize);
    RegCloseKey(hKey);

    if (result != ERROR_SUCCESS) {
        printf("błąd odczytu rejestru: %ld\n", result);
        return;
    }

    strcpy_s(browserType, 32, detectBrowser(progId));
    printf("domyślna przeglądarka: %s\n", browserType);

    if (_strcmpi(browserType, "Chrome") == 0) {
        snprintf(browserPath, pathSize, "C:\\Users\\%s\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History", username);
    }
    else if (_strcmpi(browserType, "Firefox") == 0) {
        getFirefoxProfile(browserPath, pathSize, username);
    }
    else if (_strcmpi(browserType, "Edge") == 0) {
        snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\History", username);

    }
    else if (_strcmpi(browserType, "Opera") == 0) {
        snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Roaming\\Opera Software\\Opera Stable\\History", username);

    }
    else if (_strcmpi(browserType, "Brave") == 0) {
        snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History", username);
    }
    else {
        printf("nieobsługiwana przeglądarka.\n");
        browserPath[0] = '\0';
    }
}

// eksportuje historię przeglądarki do pliku tekstowego
void sqliteConvert(const char* browserPath, const char* username, const char* date, const char* browserType) {
    sqlite3* db;
    sqlite3_stmt* stmt;

    if (strlen(browserPath) == 0) {
        printf("brak ścieżki do pliku historii.\n");
        return;
    }

    int rc = sqlite3_open_v2(browserPath, &db, SQLITE_OPEN_READONLY, NULL);
    if (rc != SQLITE_OK) {
        printf("nie można otworzyć bazy danych: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char* sql = (_strcmpi(browserType, "Firefox") == 0) ?
        "SELECT url, title, visit_count FROM moz_places ORDER BY last_visit_date DESC" :
        "SELECT url, title, visit_count FROM urls ORDER BY last_visit_time DESC";

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        printf("błąd zapytania SQL: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    DWORD dwSize = MAX_PATH;
    char volumeName[MAX_PATH];
    char driveLetter[4] = { 0 };
    HANDLE hFind = FindFirstVolume(volumeName, dwSize);
    if (hFind == INVALID_HANDLE_VALUE) {
        printf("Nie udało się znaleźć woluminów.\n");
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    do {
        // Pobierz ścieżki montażowe dla tego woluminu
        char volumeMountPoints[MAX_PATH * 10];
        DWORD charsReturned;
        char driveStrings[1024];
        if (GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings)) {
            char* p = driveStrings;
            while (*p) {
                if (GetDriveTypeA(p) == DRIVE_REMOVABLE) {
                    strncpy_s(driveLetter, sizeof(driveLetter), p, _TRUNCATE);
                    printf("Pierwszy znaleziony napęd wymienny: %s\n", driveLetter);
                    break;
                }
                p += strlen(p) + 1;
            }
            if (*p == '\0') {
                printf("Nie znaleziono napędu wymiennego.\n");
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return;
            }
        }
        else {
            printf("Błąd podczas pobierania listy napędów.\n");
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return;
        }
    } while (FindNextVolume(hFind, volumeName, dwSize));

    FindVolumeClose(hFind); // Poprawna funkcja do zamykania uchwytu

    char fileName[MAX_PATH_SIZE];
    snprintf(fileName, sizeof(fileName), "%s\\copied\\historia-%s-%s.txt", driveLetter, username, date);

    char copiedDir[MAX_PATH_SIZE];
    snprintf(copiedDir, sizeof(copiedDir), "%scopied", driveLetter);
    if (!CreateDirectoryA(copiedDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS) {
        printf("Błąd tworzenia katalogu %s: %ld\n", copiedDir, GetLastError());
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    FILE* file;
    if (fopen_s(&file, fileName, "w+") != 0) {
        char errBuf[256];
        if (strerror_s(errBuf, sizeof(errBuf), errno) == 0) {
            printf("błąd otwierania pliku: %s (%s)\n", fileName, errBuf);
        }
        else {
            printf("błąd otwierania pliku: %s\n", fileName);
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        fprintf(file, "URL: %s\nTytuł: %s\nOdwiedzin: %d\n\n",
            sqlite3_column_text(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_int(stmt, 2));
    }

    printf("historia zapisana do: %s\n", fileName);
    fclose(file);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    char username[MAX_USERNAME_SIZE];
    char browserPath[MAX_PATH_SIZE] = { 0 };
    char date[16];
    char browserType[32] = "Unknown";

    getDate(date, sizeof(date));
    DWORD userSize = sizeof(username);
    getUser(username, &userSize);
    getLocation(browserPath, sizeof(browserPath), username, browserType);
    sqliteConvert(browserPath, username, date, browserType);

    return 0;
}
