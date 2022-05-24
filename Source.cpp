#include "Header.h"

#define MAX_KEY_LEN 256

void init(ProgramInfo* p) {
	p->name = (char*)"unknown";
	p->publisher = (char*)"unknown";
	p->version = (char*)"unknown";
	p->installDate = {};
	p->size = 0;
}

int main(int argc, char const* argv[]) {

	do {
		printf("Search for Program: ");
		char* name = (char*)calloc(MAX_PATH, sizeof(char));
		fgets(name, MAX_PATH, stdin);
		name[strlen(name) - 1] = '\0';

		ScanItem scanItem = {};
		time_t t = time(NULL);

		// get scan time
		clock_t start = clock();

		vector<ProgramInfo> p = searchProgramInfo64(name);
		vector<ProgramInfo> p32 = searchProgramInfo32(name);
		scanItem.scanSeconds = double(clock() - start) / CLOCKS_PER_SEC;

		// get scan date
		scanItem.scanDate = (char*)calloc(26, sizeof(char));
		if (scanItem.scanDate != 0) {
			ctime_s(scanItem.scanDate, 26, &t);
			scanItem.scanDate[strlen(scanItem.scanDate) - 1] = '\0';
		}

		//merge 64bit and 32bit programs list
		p.insert(p.end(), p32.begin(), p32.end());
		scanItem.pList = p;

		writeLog("output.txt", scanItem);

		printProgramInfo(p);
		printf("Found %d programs in %f second\n", p.size(), scanItem.scanSeconds);
		printf("Result is saved in output.txt\n\n");


		printf("Press any key to continue or ESC to exit...\n");
	} while (_getch() != 27);



	return 0;
}

vector<ProgramInfo> searchProgramInfo64(char* name) {
	HKEY hUninstallKey = NULL;
	HKEY hAppKey = NULL;
	char root64[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

	char appKeyName[MAX_KEY_LEN] = {};
	char subKey[MAX_KEY_LEN] = {};

	char* displayName = NULL;
	char* displayVersion = NULL;
	char* publisher = NULL;
	char* installDate = NULL;
	DWORD size = 0;

	DWORD ret = ERROR_SUCCESS;
	DWORD dwType = KEY_ALL_ACCESS;
	DWORD bufferSize = 0;

	vector<ProgramInfo> pList;

	// open uninstall key
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, root64, 0, KEY_READ, &hUninstallKey) != ERROR_SUCCESS) {
		printf("[-] Failed to open registry key\n");
		return pList;
	}

	for (int i = 0; ret == ERROR_SUCCESS; i++) {
		bufferSize = sizeof(appKeyName);
		ret = RegEnumKeyExA(hUninstallKey, i, appKeyName, &bufferSize, NULL, NULL, NULL, NULL);

		if (ret == ERROR_SUCCESS) {

			sprintf_s(subKey, MAX_KEY_LEN, "%s\\%s", root64, appKeyName);

			// open uninstall's subkey
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS) {
				printf("[-] Failed to open registry key\n");
				RegCloseKey(hUninstallKey);
				RegCloseKey(hAppKey);
				return pList;
			}

			// entry must have dislayName
			displayName = (char*)calloc(MAX_KEY_LEN, sizeof(char));
			bufferSize = MAX_KEY_LEN * sizeof(char);
			if (RegQueryValueExA(hAppKey, "DisplayName", NULL, &dwType, (LPBYTE)displayName, &bufferSize) == ERROR_SUCCESS && displayName != "" && isMatch(displayName, name) != NULL) {

				ProgramInfo p;
				init(&p);
				p.name = displayName;

				// version
				displayVersion = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "DisplayVersion", NULL, &dwType, (LPBYTE)displayVersion, &bufferSize) == ERROR_SUCCESS) {
					p.version = displayVersion;
				}

				// publisher
				publisher = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "Publisher", NULL, &dwType, (LPBYTE)publisher, &bufferSize) == ERROR_SUCCESS) {
					p.publisher = publisher;
				}

				// installDate
				installDate = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "InstallDate", NULL, &dwType, (LPBYTE)installDate, &bufferSize) == ERROR_SUCCESS) {
					p.installDate = parseDate(installDate);
					free(installDate);
				}

				// size
				bufferSize = sizeof(size);
				if (RegQueryValueExA(hAppKey, "EstimatedSize", NULL, &dwType, (LPBYTE)&size, &bufferSize) == ERROR_SUCCESS) {
					p.size = size;
				}

				pList.push_back(p);
			}
			RegCloseKey(hAppKey);
		}
	}
	RegCloseKey(hUninstallKey);
	return pList;
}
vector<ProgramInfo> searchProgramInfo32(char* name) {
	HKEY hUninstallKey = NULL;
	HKEY hAppKey = NULL;
	char root32[] = "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

	char appKeyName[MAX_KEY_LEN] = {};
	char subKey[MAX_KEY_LEN] = {};

	char* displayName = NULL;
	char* displayVersion = NULL;
	char* publisher = NULL;
	char* installDate = NULL;
	DWORD size = 0;

	DWORD ret = ERROR_SUCCESS;
	DWORD dwType = KEY_ALL_ACCESS;
	DWORD bufferSize = 0;

	vector<ProgramInfo> pList;

	// open Uninstall key
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, root32, 0, KEY_READ, &hUninstallKey) != ERROR_SUCCESS) {
		printf("[-] Failed to open registry key\n");
		return pList;
	}

	for (int i = 0; ret == ERROR_SUCCESS; i++) {
		bufferSize = sizeof(appKeyName);
		ret = RegEnumKeyExA(hUninstallKey, i, appKeyName, &bufferSize, NULL, NULL, NULL, NULL);

		if (ret == ERROR_SUCCESS) {

			sprintf_s(subKey, MAX_KEY_LEN, "%s\\%s", root32, appKeyName);

			// open uninstall's subkey
			if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hAppKey) != ERROR_SUCCESS) {
				printf("[-] Failed to open registry key\n");
				RegCloseKey(hUninstallKey);
				RegCloseKey(hAppKey);
				return pList;
			}

			displayName = (char*)calloc(MAX_KEY_LEN, sizeof(char));
			bufferSize = MAX_KEY_LEN * sizeof(char);
			// entry must have displayName
			if (RegQueryValueExA(hAppKey, "DisplayName", NULL, &dwType, (LPBYTE)displayName, &bufferSize) == ERROR_SUCCESS && displayName != "" && isMatch(displayName, name) != NULL) {

				ProgramInfo p;
				init(&p);
				p.name = displayName;

				// version
				displayVersion = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "DisplayVersion", NULL, &dwType, (LPBYTE)displayVersion, &bufferSize) == ERROR_SUCCESS) {
					p.version = displayVersion;
				}

				// publisher
				publisher = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "Publisher", NULL, &dwType, (LPBYTE)publisher, &bufferSize) == ERROR_SUCCESS) {
					p.publisher = publisher;
				}

				// installDate
				installDate = (char*)calloc(MAX_KEY_LEN, sizeof(char));
				bufferSize = MAX_KEY_LEN * sizeof(char);
				if (RegQueryValueExA(hAppKey, "InstallDate", NULL, &dwType, (LPBYTE)installDate, &bufferSize) == ERROR_SUCCESS) {
					p.installDate = parseDate(installDate);
				}

				// size
				bufferSize = sizeof(size);
				if (RegQueryValueExA(hAppKey, "EstimatedSize", NULL, &dwType, (LPBYTE)&size, &bufferSize) == ERROR_SUCCESS) {
					p.size = size;
				}

				pList.push_back(p);
			}
			RegCloseKey(hAppKey);
		}
	}
	RegCloseKey(hUninstallKey);
	return pList;
}

Date parseDate(char* string) {
	Date d = {};
	if (string != NULL || string != "" || string != 0) {
		sscanf_s(string, "%4d", &d.year);
		sscanf_s(string + 4, "%2d", &d.month);
		sscanf_s(string + 6, "%2d", &d.day);
	}
	return d;
}

void printProgramInfo(vector<ProgramInfo> pList) {
	for (ProgramInfo p : pList) {
		printf("[+] Name: %s\n", p.name);
		printf("\t[+] Version: %s\n", p.version);
		printf("\t[+] Publisher: %s\n", p.publisher);
		printf("\t[+] Install Date: %d/%d/%d\n", p.installDate.day, p.installDate.month, p.installDate.year);
		printf("\t[+] Size: %d\n", p.size);
	}
}

void freeMemory(vector<ProgramInfo> pList) {
	for (ProgramInfo p : pList) {
		free(p.name);
		free(p.version);
		free(p.publisher);
	}
	pList.clear();
}

void writeLog(const char fileName[], ScanItem item) {
	FILE* f = NULL;
	bool isArray = false;

	int error = fopen_s(&f, fileName, "r+");
	if (error != 0 || f == NULL) {
		error = fopen_s(&f, fileName, "w+");
	}

	if (f != NULL && error == 0) {
		// get file size
		error = fseek(f, 0, SEEK_END);
		unsigned long size = ftell(f);

		if (size != 0) {
			error = fseek(f, -1, SEEK_END);
			char c = fgetc(f); // get last char
			if (c != ']') {    // check if it's an array

				// map the file
				char* buffer = (char*)calloc(size + 1, sizeof(char));
				if (buffer != NULL) {
					fseek(f, 0, SEEK_SET);
					fread(buffer, sizeof(char), size, f);
				}
				fseek(f, 0, SEEK_SET);
				fprintf_s(f, "[%s,\n", buffer);
			}
			else {
				fseek(f, -1, SEEK_CUR);
				fprintf(f, ",\n");
			}
			isArray = true;
		}

		fprintf_s(f, "{\n\t\"scanTime\":\"%s\",\n\t\"scanDuration\":\"%f\",\n\t\"products\":[\n", item.scanDate, item.scanSeconds);
		for (ProgramInfo p : item.pList) {
			fprintf_s(f, "\t\t%s,\n", toJson(p));
		}
		fseek(f, 0, SEEK_END);
		fprintf(f, "\t]\n}");
		if (isArray) {
			fprintf(f, "]");
		}


		fclose(f);
	}
}

char* toJson(ProgramInfo p) {
	char* json = (char*)calloc(1024, sizeof(char));
	if (json != NULL) {
		sprintf_s(json, 1024, "{\"name\":\"%s\",\"version\":\"%s\",\"publisher\":\"%s\",\"installDate\":\"%d/%d/%d\",\"size\":%d}", p.name, p.version, p.publisher, p.installDate.day, p.installDate.month, p.installDate.year, p.size);
	}
	return json;
}

bool isMatch(char* string, char* pattern) {	
	regex reg(pattern, regex_constants::icase);
	cmatch cmatch;

	return regex_search(string, cmatch, reg);
}