#include <string>
#include <iostream>
#include <filesystem>
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <atlstr.h>

#define N 2000

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

string readFromPipe(string name)
{
    wcout << "Connecting to pipe..." << endl;
    string pipeName = "\\\\.\\pipe\\my_pipe";
    // Open the named pipe
    // Most of these parameters aren't very relevant for pipes.
    wstring temp = wstring(pipeName.begin(), pipeName.end());
    const wchar_t* nameW = temp.c_str();
    HANDLE pipe = CreateFile(
        nameW,
        GENERIC_READ, // only need read access
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    if (pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to connect to pipe." << endl;
        cout<<GetLastError();
        system("pause");
        return "";
    }
    wcout << "Reading data from pipe..." << endl;
    // The read operation will block until there is data to read
    wchar_t buffer[N+1];
    DWORD numBytesRead = 0;
    BOOL result = ReadFile(
        pipe,
        buffer, // the data from the pipe will be put here
        N * sizeof(wchar_t), // number of bytes allocated
        &numBytesRead, // this will store number of bytes actually read
        NULL // not using overlapped IO
    );
    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0'; // null terminate the string
        wcout << "Number of bytes read: " << numBytesRead << endl;
        wcout << "Message: " << buffer << endl;
    }
    else {
        wcout << "Failed to read data from the pipe." << endl;
    }
    // Close our pipe handle
    CloseHandle(pipe);
    wcout << "Done." << endl;
    wstring ws(buffer);
    // your new String
    string str(ws.begin(), ws.end());
    return str;
}

void writeToPipe(string name, string content)
{
    wcout << "Creating an instance of a named pipe..." << endl;
    string pipeName = "\\\\.\\pipe\\my_pipe";
    // Create a pipe to send data
    HANDLE pipe = CreateNamedPipe(
        wstring(pipeName.begin(), pipeName.end()).c_str(), // name of the pipe
        PIPE_ACCESS_OUTBOUND, // 1-way pipe -- send only
        PIPE_TYPE_BYTE, // send data as a byte stream
        1, // only allow 1 instance of this pipe
        0, // no outbound buffer
        0, // no inbound buffer
        0, // use default wait time
        NULL // use default security attributes
    );
    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) 
    {
        wcout << "Failed to create outbound pipe instance.";
        cout << GetLastError() << endl;
        system("pause");
        return;
    }
    wcout << "Waiting for a client to connect to the pipe..." << endl;
    // This call blocks until a client process connects to the pipe
    BOOL result = ConnectNamedPipe(pipe, NULL);
    if (!result) 
    {
        wcout << "Failed to make connection on named pipe." << endl;
        // look up error code here using GetLastError()
        CloseHandle(pipe); // close the pipe
        system("pause");
        return;
    }
    wcout << "Sending data to pipe..." << endl;
    // This call blocks until a client process reads all the data
    wstring temp = wstring(content.begin(), content.end());
    const wchar_t* data = temp.c_str();
    DWORD numBytesWritten = 0;
    result = WriteFile(
        pipe, // handle to our outbound pipe
        data, // data to send
        wcslen(data) * sizeof(wchar_t), // length of data to send (bytes)
        &numBytesWritten, // will store actual amount of data sent
        NULL // not using overlapped IO
    );
    if (result) 
    {
        wcout << "Number of bytes sent: " << numBytesWritten << endl;
    }
    else 
    {
        wcout << "Failed to send data." << endl;
        // look up error code here using GetLastError()
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
            command.append(" NotMain");
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
    //cout << content << endl;
    if(!isMain)
        writeToPipe(folderName, content);
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
    if (isMain)
    {
        STARTUPINFO si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        string command = "C:\\Studia\\PR\\Laby\\Lab1\\Project1\\Debug\\Project1.exe ";
        command.append(".");
        command.append(" NotMain");
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
    }
    else
        writeToPipe("", "It lives");
    if(argc==2)
        cout << readFromPipe(str);
    
    system("pause");

}