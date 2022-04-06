#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <atlstr.h>
#include <cstdlib>
#include <Windows.h>
#include <csignal>

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

string readFromPipe()
{
    string pipeName = "\\\\.\\pipe\\pipe";
    // Open the named pipe
    // Most of these parameters aren't very relevant for pipes.
    wstring temp = wstring(pipeName.begin(), pipeName.end());
    const wchar_t* nameW = temp.c_str();
    HANDLE pipe;
    do
    {
        pipe = CreateFile(
            nameW,
            GENERIC_READ, // only need read access
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );
    } while (pipe == INVALID_HANDLE_VALUE);
    Sleep(1);//It can't get to readfile before Connect to named pipe is called


    wchar_t buffer[2001];
    DWORD numBytesRead = 0;
    BOOL result;

    // The read operation will block until there is data to read
    result = ReadFile(
        pipe,
        buffer, // the data from the pipe will be put here
        2000 * sizeof(wchar_t), // number of bytes allocated
        &numBytesRead, // this will store number of bytes actually read
        NULL // not using overlapped IO
    );
    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
        //cout << "Number of bytes read: " << numBytesRead << endl;
        //cout << "Message: " << buffer << endl;
    }
    else {
        cout << "Failed to read data from the pipe." << endl;
        cout << GetLastError() << endl;
        return "ERROR READING";
    }
    // Close our pipe handle
    CloseHandle(pipe);
    wstring ws(buffer);
    // your new String
    string str(ws.begin(), ws.end());
    return str;
}

void writeToPipe(string content)
{
    cout << "Creating an instance of a named pipe..." << endl;
    string pipeName = "\\\\.\\pipe\\pipe";
    // Create a pipe to send data
    wstring temp = wstring(pipeName.begin(), pipeName.end());
    const wchar_t* nameW = temp.c_str();
    HANDLE pipe = CreateNamedPipe(
        nameW, // name of the pipe
        PIPE_ACCESS_DUPLEX, // 1-way pipe -- send only
        PIPE_TYPE_BYTE, // send data as a byte stream
        20, // allow 20 instance of this pipe
        0, // no outbound buffer
        0, // no inbound buffer
        0, // use default wait time
        NULL // use default security attributes
    );
    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) 
    {
        cout << "Failed to create outbound pipe instance.";
        cout << GetLastError() << endl;
        system("pause");
        return;
    }
    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);

    //cout << "Sending data to pipe..." << endl;
    // This call blocks until a client process reads all the data
    temp = wstring(content.begin(), content.end());
    const wchar_t* data = temp.c_str();
    DWORD numBytesWritten = 0;
    result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );
    if (!result) 
    {
        cout << "Failed to send data." << endl;
        cout << GetLastError() << endl;
    }
    // Close the pipe (automatically disconnects client too)
    CloseHandle(pipe);
}

void getFiles(string folderName, bool isMain)
{
    string content;
    content += folderName + "\n";
    //cout << folderName << endl;

    for (const auto& entry : fs::directory_iterator(folderName))
    {
        //cout << "\t" << entry.path().string() << endl;
        if (entry.is_directory())
        {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory(&si, sizeof(si));
            si.cb = sizeof(si);
            ZeroMemory(&pi, sizeof(pi));

            string command = "C:\\Studia\\PR\\Laby\\Lab1\\Project1\\Debug\\Project1.exe ";
            command.append(entry.path().string());
            command.append(" ItIsNotMain");
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
                return;
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
            content += "\t" + entry.path().string() + "\n";
        }
    }
    cout << content << endl;
    if(!isMain)
        writeToPipe(content);
}

void main(int argc, char* argv[])
{
    
    if (argc < 2)
    {
        printf("Usage: %s [cmdline]\n", argv[0]);
        return;
    }
    bool isMain = true;
    if (argc == 3)
        isMain = false;

    std::string str(argv[1]);
    getFiles(argv[1], isMain);
    if (isMain)
    {
        while (true)
        {
            cout << readFromPipe();
        }
    }
        
    system("pause");

}