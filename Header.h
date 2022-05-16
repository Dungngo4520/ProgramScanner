#include <stdio.h>
#include <Windows.h>
#include <time.h>
#include <vector>

using namespace std;

struct Date {
    int day, month, year;
};

struct ProgramInfo {
    char *name;
    char *version;
    char *publisher;
    Date installDate;
    unsigned long size;

    BOOL operator==(ProgramInfo &other);
};

struct ScanItem {
    vector<ProgramInfo> pList;
    char *scanDate;
    double scanSeconds;
};

BOOL ProgramInfo::operator==(ProgramInfo &other) {
    return strcmp(name, other.name) == 0 &&
           strcmp(version, other.version) == 0 &&
           strcmp(publisher, other.publisher) == 0 &&
           installDate.day == other.installDate.day &&
           installDate.month == other.installDate.month &&
           installDate.year == other.installDate.year &&
           size == other.size;
}

vector<ProgramInfo> getProgramInfo64();
vector<ProgramInfo> getProgramInfo32();
Date parseDate(char *string);
void printProgramInfo(vector<ProgramInfo> pList);
void freeMemory(vector<ProgramInfo> pList);
char *toJson(ProgramInfo p);
char *toJson(ScanItem item);
void writeLog(const char fileName[], ScanItem scanItem);