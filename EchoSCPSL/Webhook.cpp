#include "Webhook.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Webhook::Webhook(const std::string& url) : webhookUrl(url) {}

void Webhook::sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames) {
    std::string payload = buildJsonPayload(username, loki, midnight, cyrix, usernames);

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

std::string Webhook::buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames) {
    std::string title = (loki || midnight || cyrix) ? "DETECTED" : "CLEAR";
    std::string description = (loki || midnight || cyrix) ?
        "The anticheat detected a software that gave an advantage to the player.\n\n" :
        "The anticheat didn't detected any suspicious software. Remember, this is not perfect!\n\n";

    std::string color = (loki || midnight || cyrix) ? "16711680" : "65280"; // Red if detected, green if clear.

    std::string fields = "";
    if (loki) {
        fields += "\n- Founded Loki Strings in game";
    }
    if (midnight) {
        fields += "\n- Founded Midnight Strings in game";
    }
    if (cyrix) {
        fields += "\n- Founded Cyrix Strings in game";
    }

    if (!fields.empty()) {
        fields = "```diff" + fields + "\n```";
    }

    std::string accountsSection = "Steam accounts:\n";
    for (const auto& account : usernames) {
        accountsSection += "```" + account + "```";
    }

    // Build the JSON payload.
    std::string payload = "{";
    payload += "\"content\": null,";
    payload += "\"embeds\": [{";
    payload += "\"title\": \"" + title + "\",";
    payload += "\"description\": \"" + description + "\",";
    payload += "\"color\": " + color + ",";

    // Fields section (including detection messages and accounts)
    payload += "\"fields\": [{";
    payload += "\"name\": \"What has been founded:\",";
    payload += "\"value\": \"" + fields + "\"";
    payload += "}, {";
    payload += "\"name\": \"Other\",";
    payload += "\"value\": \"" + accountsSection + "\"";
    payload += "}],";

    payload += "\"author\": {";
    payload += "\"name\": \"" + username + "\"";
    payload += "},";
    payload += "\"footer\": {";
    payload += "\"text\": \"SCPSLAC\"";
    payload += "},";
    payload += "\"timestamp\": \"" + getCurrentTimestamp() + "\",";

    payload += "\"thumbnail\": {";
    if (midnight || loki || cyrix) {
        payload += "\"url\": \"https://cdn-icons-png.freepik.com/256/17385/17385231.png?semt=ais_hybrid\"";
    }
    else {
        payload += "\"url\": \"https://cdn-icons-png.freepik.com/256/5610/5610944.png?semt=ais_hybrid\"";
    }
    payload += "}";

    payload += "}],";
    payload += "\"attachments\": []";
    payload += "}";

    return payload;
}
