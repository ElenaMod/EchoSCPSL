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

void Webhook::sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime, bool isDiagTrackRunning, const std::string& uptimeDiagTrack, bool isDpsRunning, const std::string& uptimeDps, bool isPcaSvcRunning, const std::string& uptimePcaSvc, bool isSysMainRunning, const std::string& uptimeSysMain, bool isCdpSvcRunning, const std::string& uptimeCdpSvc, bool isSsdpsrvRunning, const std::string& uptimeSsdpsrv, bool isUmRdpServiceRunning, const std::string& uptimeUmRdpService) {
    std::string payload = buildJsonPayload(username, loki, midnight, cyrix, usernames, recycleBinClearTime, isDiagTrackRunning, uptimeDiagTrack, isDpsRunning, uptimeDps, isPcaSvcRunning, uptimePcaSvc, isSysMainRunning, uptimeSysMain, isCdpSvcRunning, uptimeCdpSvc, isSsdpsrvRunning, uptimeSsdpsrv, isUmRdpServiceRunning, uptimeUmRdpService);

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

std::string Webhook::buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime, bool isDiagTrackRunning, const std::string& uptimeDiagTrack, bool isDpsRunning, const std::string& uptimeDps, bool isPcaSvcRunning, const std::string& uptimePcaSvc, bool isSysMainRunning, const std::string& uptimeSysMain, bool isCdpSvcRunning, const std::string& uptimeCdpSvc, bool isSsdpsrvRunning, const std::string& uptimeSsdpsrv, bool isUmRdpServiceRunning, const std::string& uptimeUmRdpService){
    bool recy = false;
    bool det = false;
    bool uns = false;
    std::string foundedField;
    std::string title = "CLEAR";
    std::string description = "The anticheat didn't detect any suspicious software. Remember, this is not perfect!\n\n";
    std::string color = "65280"; // Green if clear.

    std::string trimmedTime = trim(recycleBinClearTime);

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


        if (duration < 30 || !isDiagTrackRunning || !isDpsRunning || !isPcaSvcRunning || !isSysMainRunning || !isCdpSvcRunning || !isSsdpsrvRunning || !isUmRdpServiceRunning) {
            title = "UNSURE";
            description = "Some suspitious actions have been found, but not acctual proof.\n\n";
            if (duration < 30) { recy = true; }
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

    std::string url = "https://raw.githubusercontent.com/ElenaMod/EchoSCPSL/refs/heads/master/images/clear.png";
    if (uns && !det) {
        url = "https://raw.githubusercontent.com/ElenaMod/EchoSCPSL/refs/heads/master/images/unsure.png";
    }
    else if (det) {
        url = "https://raw.githubusercontent.com/ElenaMod/EchoSCPSL/refs/heads/master/images/detected.png";
    }

    // Build fields dynamically
    std::string fields = "";
    if (recy) {
        fields = "```ansi\n\\u001b[0;41m/ Recycle bin was modified not much time ago```";
    }
    if (!isDiagTrackRunning) {
        fields += "```ansi\n\\u001b[0;41m/ DiagTrack has been manually stopped```";
    }
    if (!isDpsRunning) {
        fields += "```ansi\n\\u001b[0;41m/ DPS has been manually stopped```";
    }
    if (!isPcaSvcRunning) {
        fields += "```ansi\n\\u001b[0;41m/ PcaSvc has been manually stopped```";
    }
    if (!isSysMainRunning) {
        fields += "```ansi\n\\u001b[0;41m SysMain has been manually stopped```";
    }
    if (!isCdpSvcRunning) {
        fields += "```ansi\n\\u001b[0;41m/ CdpSvc has been manually stopped```";
    }
    if (!isSsdpsrvRunning) {
        fields += "```ansi\n\\u001b[0;41m/ Ssdpsrv has been manually stoppe.```";
    }
    if (!isUmRdpServiceRunning) {
        fields += "```ansi\n\\u001b[0;41m/ UmRdpService has been manually stopped```";
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
    if (det || uns) {
        foundedField = "\"name\": \"What has been founded:\",\"value\": \"" + fields + "\"";
    }
    else {
        foundedField = "\"name\": \"\",\"value\": \"\"";
    }

    std::string accountsSection = "Steam accounts:\n";
    for (const auto& account : usernames) {
        accountsSection += "```" + account + "```";
    }
    accountsSection += "\n";

    std::string recycleBinInfo = recycleBinClearTime.empty() ? "" : "Recycle bin time: \n```ansi\n\\u001b[2;45m" + recycleBinClearTime + "```\n";

    std::string diagTrackInfo = isDiagTrackRunning ? "DiagTrack time: \n```ansi\n\\u001b[2;45m" + uptimeDiagTrack + "```\n" : "DiagTrack time: \n```diff\n- Service is not running```\n";
    std::string dpsInfo = isDpsRunning ? "DPS time: \n```ansi\n\\u001b[2;45m" + uptimeDps + "```\n" : "DPS time: \n```diff\n- Service is not running```\n";
    std::string pcaSvcInfo = isPcaSvcRunning ? "PcaSvc time: \n```ansi\n\\u001b[2;45m" + uptimePcaSvc + "```\n" : "PcaSvc time: \n```diff\n- Service is not running```\n";
    std::string sysMainInfo = isSysMainRunning ? "SysMain time: \n```ansi\n\\u001b[2;45m" + uptimeSysMain + "```\n" : "SysMain time: \n```diff\n- Service is not running```\n";
    std::string cdpSvcInfo = isCdpSvcRunning ? "CdpSvc time: \n```ansi\n\\u001b[2;45m" + uptimeCdpSvc + "```\n" : "CdpSvc time: \n```diff\n- Service is not running```\n";
    std::string ssdpsrvInfo = isSsdpsrvRunning ? "Ssdpsrv time: \n```ansi\n\\u001b[2;45m" + uptimeSsdpsrv + "```\n" : "Ssdpsrv time: \n```diff\n- Service is not running```\n";
    std::string umRdpServiceInfo = isUmRdpServiceRunning ? "UmRdpService time: \n```ansi\n\\u001b[2;45m" + uptimeUmRdpService + "```\n" : "UmRdpService time: \n```diff\n- Service is not running```\n";

    std::string payload = "{";
    payload += "\"content\": null,";
    payload += "\"embeds\": [{";
    payload += "\"title\": \"" + title + "\",";
    payload += "\"description\": \"" + description + "\",";
    payload += "\"color\": " + color + ",";
    payload += "\"fields\": [{";
    if (!foundedField.empty()) {
        payload += foundedField + ",";
    }
    payload += "\"value\": \"" + fields + "\"";
    payload += "}, {";
    payload += "\"name\": \"Other\",";
    payload += "\"value\": \"" + accountsSection + recycleBinInfo + diagTrackInfo + dpsInfo + pcaSvcInfo + sysMainInfo + cdpSvcInfo + ssdpsrvInfo + umRdpServiceInfo + "\"";
    payload += "}],";
    payload += "\"author\": {\"name\": \"" + username + "\"},";
    payload += "\"footer\": {\"text\": \"SCPSLAC\"},";
    payload += "\"timestamp\": \"" + getCurrentTimestamp() + "\",";
    payload += "\"thumbnail\": {\"url\": \"" + url + "\"}";
    payload += "}],";
    payload += "\"attachments\": []}";


    return payload;
}
