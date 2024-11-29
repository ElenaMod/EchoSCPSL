#ifndef SCANNER_H
#define SCANNER_H

#include <Windows.h>
#include <vector>
#include <string>

extern bool loki;
extern bool midnight;
extern bool cyrix;

// Function to find the process ID of a running process by name.
DWORD getProcessID(const std::string& processName);

// Function to scan memory regions of a process.
void scanMemoryRegions(DWORD pid, const std::vector<std::string>& lokiStrings, const std::vector<std::string>& midnightStrings, const std::vector<std::string>& cyrixStrings);

// Function to check if a memory region contains any of the words in a list (case-insensitive).
bool containsWord(const std::string& memoryContent, const std::vector<std::string>& wordList);

// Function to read memory from a process.
std::string readProcessMemory(HANDLE hProcess, LPCVOID address, SIZE_T size);

#endif // SCANNER_H
