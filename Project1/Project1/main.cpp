#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <atlstr.h>

namespace fs = std::filesystem;

using namespace std;

struct Folder;
struct Finfo
{
    bool isFile;
    string name;
    __int64 length;
    fs::file_time_type modificationDate;
    Folder* parent;
};

struct Folder : public Finfo
{
    list<Finfo*> children;
};

void writeFiles(Folder* folder)
{
    cout << folder->name<<endl;
    int level=0;
    for (Folder* current = folder->parent; current != NULL; current = current->parent)
        level++;
    for (Finfo* current : folder->children)
    {
        for (int i = 0; i < level; i++)
            cout << "\t";
        if (current->isFile)
            cout << current->name<<endl;
        else
            writeFiles((Folder*)current);
    }
}

Folder* getFiles(string folderName)
{
    Folder* folder = new Folder();
    fs::directory_entry* entry = new fs::directory_entry(fs::path(folderName));
    folder->name = folderName;
    folder->isFile = false;
    folder->modificationDate = entry->last_write_time();
    folder->length = entry->file_size();
    folder->parent = NULL;
    cout << folderName << endl;

    for (const auto& entry : fs::directory_iterator(folderName))
    {
        Finfo* current;
        cout << "\t" << entry.path().string() << endl;
        if (entry.is_directory())
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            string command = "C:\\Studia\\PR\\Laby\\Lab1\\Project1\\Debug\\Project1.exe ";
            command.append(entry.path().string());
            TCHAR commandTChar[200];
            _tcscpy_s(commandTChar, CA2T(command.c_str()));

            // Start the child process. 
            if (!CreateProcess(NULL,   // No module name (use command line)
                commandTChar,        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                CREATE_NEW_CONSOLE,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory 
                &si,            // Pointer to STARTUPINFO structure
                &pi)           // Pointer to PROCESS_INFORMATION structure
                )
            {
                printf("CreateProcess failed (%d).\n", GetLastError());
                return NULL;
            }
            //current = getFiles(entry.path().string());

            // Wait until child process exits.
            // WaitForSingleObject(pi.hProcess, 100);

            // Close process and thread handles. 
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else
        {
            current = new Finfo();
            current->isFile = true;
            current->length = entry.file_size();
            current->modificationDate = entry.last_write_time();
            current->parent = folder;
            current->name = entry.path().string();
            current->parent = folder;
            folder->children.push_back(current);
        }

    }


    delete entry;
    return folder;
}

void main(int argc, char* argv[])
{

    if (argc != 2)
    {
        printf("Usage: %s [cmdline]\n", argv[0]);
        return;
    }

    std::string str(argv[1]);
    getFiles(str);
    system("pause");
}