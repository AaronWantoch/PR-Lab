#pragma comment(lib, "Shlwapi.lib")

#ifndef UNICODE
#define UNICODE
#endif 


#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <list>
#include "Shlwapi.h"

using namespace std;

struct Folder;
struct Finfo
{
    bool isFile;
    string name;
    __int64 length;
    string modificationDate;
    Folder* parent;
};
struct Folder : public Finfo
{
    list<Finfo*> children;
};

string convert(LPCTSTR name)
{
    wstring w;
    w = name;
    return string(w.begin(), w.end());
}

void FindFilesRecursively(Folder* folder)
{
    LPCTSTR lpFolder = _T(folder->name);
    cout << convert(lpFolder) << endl;

    TCHAR szFullPattern[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFindFile;
    // first we are going to process any subdirectories
    PathCombine(szFullPattern, lpFolder, _T("*"));
    hFindFile = FindFirstFile(szFullPattern, &FindFileData);
    FindNextFile(hFindFile, &FindFileData);
    FindNextFile(hFindFile, &FindFileData);
    if (hFindFile != INVALID_HANDLE_VALUE)
    {
        do
        {
            PathCombine(szFullPattern, lpFolder, FindFileData.cFileName);

            if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                // found a subdirectory; recurse into it
                FindFilesRecursively(szFullPattern);
            }
            else
            {
                cout << convert(szFullPattern) << endl;
            }
        } while (FindNextFile(hFindFile, &FindFileData));
        FindClose(hFindFile);
    }
}

int main()
{
    FindFilesRecursively(_T("C:\\Riot Games"));
}