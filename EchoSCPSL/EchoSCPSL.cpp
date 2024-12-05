#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <regex>
#include <vector>
#include <Windows.h>
#include <chrono>
#include "scanner.h"
#include "WebHook.h"
#include <iomanip>
#include <ctime>

bool CheckServiceStatus(const std::string& serviceName, std::string& uptime) {
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (hSCManager == NULL) {
        std::cerr << "Error opening service manager." << std::endl;
        return false;
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName.c_str(), SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG);
    if (hService == NULL) {
        std::cerr << "Error opening service: " << serviceName << std::endl;
        CloseServiceHandle(hSCManager);
        return false;
    }

    SERVICE_STATUS_PROCESS ssp;
    DWORD bytesNeeded;
    if (!QueryServiceStatusEx(hService, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp, sizeof(ssp), &bytesNeeded)) {
        std::cerr << "Error querying service status for service: " << serviceName << std::endl;
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return false;
    }

    bool isRunning = false;
    SYSTEMTIME systemTime;
    std::ostringstream timeStream;

    if (ssp.dwCurrentState == SERVICE_RUNNING) {
        isRunning = true;

        // Query the service's process ID and get the process start time using GetProcessTimes
        DWORD processID = ssp.dwProcessId;
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processID);
        if (hProcess != NULL) {
            FILETIME creationTime, exitTime, kernelTime, userTime;
            if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                // Calculate uptime based on the creation time of the process
                ULARGE_INTEGER start;
                start.LowPart = creationTime.dwLowDateTime;
                start.HighPart = creationTime.dwHighDateTime;

                FILETIME currentTime;
                GetSystemTimeAsFileTime(&currentTime);

                ULARGE_INTEGER now;
                now.LowPart = currentTime.dwLowDateTime;
                now.HighPart = currentTime.dwHighDateTime;

                // Calculate uptime
                LONGLONG uptimeIn100ns = now.QuadPart - start.QuadPart;
                LONGLONG uptimeInSeconds = uptimeIn100ns / 10000000; // Convert from 100ns to seconds

                int hours = uptimeInSeconds / 3600;
                int minutes = (uptimeInSeconds % 3600) / 60;
                int seconds = uptimeInSeconds % 60;

                timeStream << std::setw(2) << std::setfill('0') << hours << "H:"
                    << std::setw(2) << std::setfill('0') << minutes << "M:"
                    << std::setw(2) << std::setfill('0') << seconds << "S";
            }
            else {
                std::cerr << "Error getting process times for service: " << serviceName << std::endl;
            }
            CloseHandle(hProcess);
        }
        else {
            std::cerr << "Error opening process for service: " << serviceName << std::endl;
        }
    }
    else {
        uptime = "00H:00M:00S"; // Default uptime if stopped or disabled
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    uptime = timeStream.str();
    return isRunning;
}

// A function to get the last modification time of files in the Recycle Bin
std::string getRecycleBinClearTime() {
    const std::string recycleBinPath = "C:\\$Recycle.Bin"; // Default path for Recycle Bin
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((recycleBinPath + "\\*").c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return "Could not open Recycle Bin folder."; // Return error message if folder is inaccessible
    }

    FILETIME lastWriteTime;
    bool foundFile = false;

    // Iterate over all files in the Recycle Bin
    do {
        // Skip "." and ".."
        if (findFileData.cFileName[0] == '.' && (findFileData.cFileName[1] == '\0' || (findFileData.cFileName[1] == '.' && findFileData.cFileName[2] == '\0'))) {
            continue;
        }

        // Get the last write time of the file
        lastWriteTime = findFileData.ftLastWriteTime;
        foundFile = true;

    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);

    if (!foundFile) {
        return "Recycle Bin is empty."; // If no files found, return message
    }

    SYSTEMTIME systemTime;
    FileTimeToSystemTime(&lastWriteTime, &systemTime);

    // Convert the last write time to time_t for easy comparison
    std::tm lastWriteTM = {};
    lastWriteTM.tm_year = systemTime.wYear - 1900;
    lastWriteTM.tm_mon = systemTime.wMonth - 1;
    lastWriteTM.tm_mday = systemTime.wDay;
    lastWriteTM.tm_hour = systemTime.wHour;
    lastWriteTM.tm_min = systemTime.wMinute;
    lastWriteTM.tm_sec = systemTime.wSecond;
    std::time_t lastWrite = std::mktime(&lastWriteTM);

    // Get the current time using chrono
    auto now = std::chrono::system_clock::now();
    auto currentTime = std::chrono::system_clock::to_time_t(now);

    // Subtract one hour from the current time to avoid DST issues
    auto correctedTime = std::chrono::system_clock::from_time_t(currentTime) - std::chrono::hours(1);
    auto correctedTimeT = std::chrono::system_clock::to_time_t(correctedTime);

    // Calculate the time difference using chrono
    auto duration = std::chrono::system_clock::from_time_t(correctedTimeT) - std::chrono::system_clock::from_time_t(lastWrite);
    auto hours = std::chrono::duration_cast<std::chrono::hours>(duration);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration) % 60;
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(duration) % 60;

    // Format the output into "HH:MM:SS"
    std::ostringstream timeStream;
    timeStream << hours.count() << "H:" << (minutes.count() < 10 ? "0" : "") << minutes.count() << "M:" << (seconds.count() < 10 ? "0" : "") << seconds.count() << "S";

    return timeStream.str();
}
std::string getFormattedUsernameFromVDF(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return "";
    }

    std::string line;
    std::string firstUsername;
    std::string personaName;
    std::string accountName;
    bool inUserBlock = false;

    // Read the file line by line
    while (std::getline(file, line)) {
        // Trim leading/trailing whitespaces
        line = std::regex_replace(line, std::regex("^\\s+|\\s+$"), "");

        // Check for the start of a user block, e.g., "76561199196678505"
        if (line.empty()) continue;
        if (line[0] == '"') { // Start of a new user block
            size_t pos = line.find_first_of('"', 1); // Find the closing quote
            if (pos != std::string::npos) {
                inUserBlock = true;
            }
        }

        // Extract the AccountName (username) and PersonaName (display name) for the current user
        if (inUserBlock) {
            if (line.find("AccountName") != std::string::npos) {
                size_t startPos = line.find('"', line.find("AccountName") + 12) + 1;
                size_t endPos = line.find('"', startPos);
                if (startPos != std::string::npos && endPos != std::string::npos) {
                    accountName = line.substr(startPos, endPos - startPos);
                }
            }

            if (line.find("PersonaName") != std::string::npos) {
                size_t startPos = line.find('"', line.find("PersonaName") + 12) + 1;
                size_t endPos = line.find('"', startPos);
                if (startPos != std::string::npos && endPos != std::string::npos) {
                    personaName = line.substr(startPos, endPos - startPos);
                }
            }

            // Stop after finding both fields
            if (!personaName.empty() && !accountName.empty()) {
                firstUsername = personaName + " (" + accountName + ")";
                break;
            }
        }
    }

    file.close();
    return firstUsername;
}

std::vector<std::string> getAllAccountNamesFromVDF(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return {};
    }

    std::vector<std::string> accountNames;
    std::string line;

    while (std::getline(file, line)) {
        // Trim leading/trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Check for AccountName
        if (line.find("\"AccountName\"") != std::string::npos) {
            size_t startPos = line.find('"', line.find("AccountName") + 12) + 1;
            size_t endPos = line.find('"', startPos);
            if (startPos != std::string::npos && endPos != std::string::npos) {
                accountNames.push_back(line.substr(startPos, endPos - startPos));
            }
        }
    }

    file.close();
    return accountNames;
}

int main() {
    // Word lists to search.
    std::vector<std::string> lokiStrings = {
        "person-arrow-down-to-line",
        "anchor - circle - exclamation",
        "plane - circle - xmark"
    };

    std::vector<std::string> midnightStrings = {
        "0u&HcA",
        "ezx|j",
        "tSIcA8D"
    };

    std::vector<std::string> cyrixStrings = {
        "?id@?$codecvt@_SDU_Mbstatet@@@std@@2V0locale@2@A",
        "eSilH",
        "!g.NavScoringRectScreen.IsInverted()",
        "%Y+4I+"
    };

    // Get the process ID of SCPSL.exe
    DWORD pid = getProcessID("SCPSL.exe");
    if (pid == 0) {
        std::cerr << "Could not find the SCPSL.exe process.\n";
        system("pause");
        return -1;
    }

    // Get the formatted username from loginusers.vdf
    std::string filePath = "C:\\Program Files (x86)\\Steam\\config\\loginusers.vdf";  // Adjust path if needed
    std::string username = getFormattedUsernameFromVDF(filePath);
    if (username.empty()) {
        std::cerr << "Failed to extract username from loginusers.vdf\n";
        system("pause");
        return -1;
    }

    // Start scanning memory regions.
    scanMemoryRegions(pid, lokiStrings, midnightStrings, cyrixStrings);
    std::cerr << "done\n";
    std::vector<std::string> accountNames = getAllAccountNamesFromVDF(filePath);

    // Webhook to send the message with the extracted username
    Webhook webhook("https://discord.com/api/webhooks/1311777600888246323/nV8g9srlCIa38yWxR7rhONJjaM9p4QOTWlZGXqrD_OF9IdoOZDYgeop9DrN8krrDFa3B");

    std::string recycleBinClearTime = getRecycleBinClearTime();

    // Variables to store status and uptime of each service
    bool isDiagTrackRunning, isDpsRunning, isPcaSvcRunning;
    bool isSysMainRunning, isCdpUserSvcRunning, isCdpSvcRunning, isSsdpsrvRunning;
    bool isUmRdpServiceRunning;

    std::string uptimeDiagTrack, uptimeDps, uptimePcaSvc;
    std::string uptimeSysMain, uptimeCdpUserSvc, uptimeCdpSvc, uptimeSsdpsrv;
    std::string uptimeUmRdpService;

    // Check status and uptime for each service
    isDiagTrackRunning = CheckServiceStatus("DiagTrack", uptimeDiagTrack);
    isDpsRunning = CheckServiceStatus("dps", uptimeDps);
    isPcaSvcRunning = CheckServiceStatus("pcasvc", uptimePcaSvc);
    isSysMainRunning = CheckServiceStatus("SysMain", uptimeSysMain);
    isCdpUserSvcRunning = CheckServiceStatus("cdpusersvc", uptimeCdpUserSvc);
    isCdpSvcRunning = CheckServiceStatus("CDPSvc", uptimeCdpSvc);
    isSsdpsrvRunning = CheckServiceStatus("SSDPSRV", uptimeSsdpsrv);
    isUmRdpServiceRunning = CheckServiceStatus("UmRdpService", uptimeUmRdpService);

    // Send webhook message using the formatted username
    webhook.sendWebhookMessage(username, loki, midnight, cyrix, accountNames, recycleBinClearTime, isDiagTrackRunning, uptimeDiagTrack, isDpsRunning, uptimeDps, isPcaSvcRunning, uptimePcaSvc, isSysMainRunning, uptimeSysMain, isCdpSvcRunning, uptimeCdpSvc, isSsdpsrvRunning, uptimeSsdpsrv, isUmRdpServiceRunning, uptimeUmRdpService);

    // Get and print the Recycle Bin clear time
    
    std::cout << "Recycle Bin was last cleared at: " << recycleBinClearTime << "\n\n";

    system("pause");
    return 0;
}
