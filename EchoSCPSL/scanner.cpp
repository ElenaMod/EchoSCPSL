#include "scanner.h"
#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <cctype>
#include <TlHelp32.h>

bool loki = false;
bool midnight = false;
bool cyrix = false;


// Function to get the process ID based on process name.
DWORD getProcessID(const std::string& processName) {
    DWORD pid = 0;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(pe);

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (processName == pe.szExeFile) {
                pid = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }
    CloseHandle(hSnapshot);
    return pid;
}

// Function to check if a memory region contains any word from the list (case-insensitive).
// Function to check if a memory region contains any word from the list (case-insensitive).
bool containsWord(const std::string& memoryContent, const std::vector<std::string>& wordList) {
    for (const auto& word : wordList) {
        if (memoryContent.find(word) != std::string::npos) {
            return true;
        }
    }
    return false;
}

// Function to read memory from a process.
std::string readProcessMemory(HANDLE hProcess, LPCVOID address, SIZE_T size) {
    char* buffer = new char[size];
    SIZE_T bytesRead;
    if (ReadProcessMemory(hProcess, address, buffer, size, &bytesRead)) {
        std::string result(buffer, bytesRead);
        delete[] buffer;
        return result;
    }
    delete[] buffer;
    return "";
}

// Function to scan memory regions of a process.
void scanMemoryRegions(DWORD pid, const std::vector<std::string>& lokiStrings, const std::vector<std::string>& midnightStrings, const std::vector<std::string>& cyrixStrings) {
    HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProcess) {
        std::cerr << "Failed to open process for reading." << std::endl;
        return;
    }

    MEMORY_BASIC_INFORMATION mbi;
    LPCVOID address = 0;

    while (VirtualQueryEx(hProcess, address, &mbi, sizeof(mbi))) {
        // Check only regions that are readable.
        if (mbi.State == MEM_COMMIT && mbi.Protect != PAGE_NOACCESS) {
            if (mbi.Type == MEM_PRIVATE || mbi.Type == MEM_IMAGE || mbi.Type == MEM_MAPPED) {
                std::string memoryContent = readProcessMemory(hProcess, address, mbi.RegionSize);

                // Case-insensitive search for words from each list.
                if (containsWord(memoryContent, lokiStrings)) {
                    loki = true;
                }
                if (containsWord(memoryContent, midnightStrings)) {
                    midnight = true;
                }
                if (containsWord(memoryContent, cyrixStrings)) {
                    cyrix = true;
                }
            }
        }

        // Move to the next region.
        address = (char*)address + mbi.RegionSize;
    }

    CloseHandle(hProcess);
}
