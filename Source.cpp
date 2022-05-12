#include "Header.h"

#define MAX_KEY_LEN 256

int main(int argc, char const* argv[]) {
	HKEY hUninstallKey = NULL, hAppKey = NULL;
	char appKeyName[MAX_KEY_LEN] = {},
		subKey[MAX_KEY_LEN] = {},
		displayName[MAX_KEY_LEN] = {};
	char root64[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	char root32[] = "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
	DWORD ret = ERROR_SUCCESS;
	DWORD dwType = KEY_ALL_ACCESS;
	DWORD bufferSize = 0;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, root64, 0, KEY_READ, &hUninstallKey) != ERROR_SUCCESS) {
		if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, root32, 0, KEY_READ, &hUninstallKey) != ERROR_SUCCESS) {
			printf("[-] Failed to open registry key\n");
			return 1;
		}
	}

	for (DWORD i = 0; ret == ERROR_SUCCESS; i++) {
		bufferSize = sizeof(appKeyName);
		ret = RegEnumKeyEx(hUninstallKey, i, appKeyName, &bufferSize, NULL, NULL, NULL, NULL);

		if (ret == ERROR_SUCCESS) {
			sprintf_s(subKey, MAX_KEY_LEN, "%s\\%s", root64, appKeyName);
			DWORD temp = RegOpenKeyEx(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hAppKey);
			if (temp != ERROR_SUCCESS) {
				printf("[-] Failed to open registry key\n");
				RegCloseKey(hUninstallKey);
				RegCloseKey(hAppKey);
				return 1;
			}
			bufferSize = sizeof(subKey);
			temp = RegQueryValueEx(hAppKey, "DisplayName", NULL, &dwType, (LPBYTE)displayName, &bufferSize);
			if (temp == ERROR_SUCCESS) {
				printf("[+] %s\n", displayName);
			}
			RegCloseKey(hAppKey);
		}
	}

	RegCloseKey(hUninstallKey);

	return 0;
}
