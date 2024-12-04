#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <regex>
#include <vector>
#include "scanner.h"
#include "WebHook.h"

// A simple VDF parsing function to extract the first username
std::string getFirstUsernameFromVDF(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return "";
    }

    std::string line;
    std::string firstUsername;
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

        // Extract the AccountName (username) for the current user
        if (inUserBlock && line.find("AccountName") != std::string::npos) {
            size_t startPos = line.find('"', line.find("AccountName") + 12) + 1;
            size_t endPos = line.find('"', startPos);
            if (startPos != std::string::npos && endPos != std::string::npos) {
                firstUsername = line.substr(startPos, endPos - startPos);
                break;  // Stop after finding the first username
            }
        }
    }

    file.close();
    return firstUsername;
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

    // Send webhook message using the formatted username
    webhook.sendWebhookMessage(username, loki, midnight, cyrix, accountNames);

    system("pause");
    return 0;
}

