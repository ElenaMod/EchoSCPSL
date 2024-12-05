#include "Webhook.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

// Trim leading and trailing whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

Webhook::Webhook(const std::string& url) : webhookUrl(url) {}

void Webhook::sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime) {
    std::string payload = buildJsonPayload(username, loki, midnight, cyrix, usernames, recycleBinClearTime);

    // Escape the payload (handle escaping properly for curl to work)
    std::string escapedPayload;
    for (char c : payload) {
        if (c == '"') {
            escapedPayload += "\\\"";
        }
        else if (c == '\n') {
            escapedPayload += "\\n";
        }
        else if (c == '\r') {
            escapedPayload += "\\r";
        }
        else {
            escapedPayload += c;
        }
    }

    // Build and execute the curl command.
    std::string command = "curl -X POST -k -H \"Content-Type: application/json\" -d \"" + escapedPayload + "\" " + webhookUrl;
    system(command.c_str());
}

std::string Webhook::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::gmtime(&timeT);

    std::ostringstream timestamp;
    timestamp << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << ".000Z";
    return timestamp.str();
}

std::string Webhook::buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime) {
    bool recy = false;
    bool det = false;
    bool uns = false;
    std::string title = "CLEAR";
    std::string description = "The anticheat didn't detect any suspicious software. Remember, this is not perfect!\n\n";
    std::string color = "65280"; // Green if clear.

    std::string trimmedTime = trim(recycleBinClearTime);
    std::cout << "Trimmed recycleBinClearTime: \"" << trimmedTime << "\"" << std::endl;

    std::tm recycleBinTime = {};
    bool parsed = false;

    // Custom parsing for "HHH:MM:SS" or "HH:MM:SS"
    if (trimmedTime.find('H') != std::string::npos) {
        int hours, minutes, seconds;
        if (sscanf(trimmedTime.c_str(), "%dH:%dM:%dS", &hours, &minutes, &seconds) == 3) {
            recycleBinTime.tm_hour = hours;
            recycleBinTime.tm_min = minutes;
            recycleBinTime.tm_sec = seconds;
            parsed = true;
        }
    }

    if (parsed) {
        // Calculate total duration in seconds from parsed values
        long long totalSeconds = recycleBinTime.tm_hour * 3600 + recycleBinTime.tm_min * 60 + recycleBinTime.tm_sec;

        auto now = std::chrono::system_clock::now();
        auto recycleBinTimepoint = now - std::chrono::seconds(totalSeconds);

        auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - recycleBinTimepoint).count();

        std::cout << "Time difference in minutes: " << duration << " minutes." << std::endl;

        if (duration < 30) {
            title = "UNSURE";
            description = "Some suspitious actions have been found, but not accytual proof.\n\n";
            recy = true;
            color = "16742912";
            uns = true;
        }
    }


    // Suspicious software detection
    if (loki || midnight || cyrix) {
        title = "DETECTED";
        description = "The anticheat detected a software that gave an advantage to the player.\n\n";
        color = "16711680"; // Red if detected
        det = true;
    }

    std::string url = "https://cdn-icons-png.freepik.com/256/5610/5610944.png?semt=ais_hybrid";
    if (uns) {
        url = "https://cdn-icons-png.freepik.com/256/17385/17385231.png?semt=ais_hybrid";
    }
    else if (det) {
        url = "https://cdn-icons-png.freepik.com/256/17385/17385231.png?semt=ais_hybrid";
    }

    // Build fields dynamically
    std::string fields = "";
    if (recy) {
        fields = "```/ Recycle bin was cleared not so much time ago```";
    }
    if (loki) {
        fields += "```diff\n- Founded Loki Strings in game```";
    }
    if (midnight) {
        fields += "```diff\n- Founded Midnight Strings in game```";
    }
    if (cyrix) {
        fields += "```diff\n- Founded Cyrix Strings in game```";
    }

    std::string foundedField = fields.empty() ? "\"name\": \"\",\"value\": \"\"" : "\"name\": \"What has been founded:\",\"value\": \"" + fields + "\"";

    std::string accountsSection = "Steam accounts:\n";
    for (const auto& account : usernames) {
        accountsSection += "```" + account + "```";
    }
    accountsSection += "\n";

    std::string recycleBinInfo = recycleBinClearTime.empty() ? "" : "Recycle bin time: \n```" + recycleBinClearTime + "```";

    std::string payload = "{";
    payload += "\"content\": null,";
    payload += "\"embeds\": [{";
    payload += "\"title\": \"" + title + "\",";
    payload += "\"description\": \"" + description + "\",";
    payload += "\"color\": " + color + ",";
    payload += "\"fields\": [{";
    payload += "\"name\": \"What has been founded:\",";
    payload += "\"value\": \"" + fields + "\"";
    payload += "}, {";
    payload += "\"name\": \"Other\",";
    payload += "\"value\": \"" + accountsSection + recycleBinInfo + "\"";
    payload += "}],";
    payload += "\"author\": {\"name\": \"" + username + "\"},";
    payload += "\"footer\": {\"text\": \"SCPSLAC\"},";
    payload += "\"timestamp\": \"" + getCurrentTimestamp() + "\",";
    payload += "\"thumbnail\": {\"url\": \"" + url + "\"}";
    payload += "}],";
    payload += "\"attachments\": []}";

    std::cerr << payload << "\n\n";


    return payload;
}
