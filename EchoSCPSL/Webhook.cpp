#include "Webhook.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Webhook::Webhook(const std::string& url) : webhookUrl(url) {}

void Webhook::sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix) {
    std::string payload = buildJsonPayload(username, loki, midnight, cyrix);

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

std::string Webhook::buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix) {
    std::string title = (loki || midnight || cyrix) ? "DETECTED" : "CLEAR";
    std::string description = (loki || midnight || cyrix) ?
        "The anticheat detected a software that gave an advantage to the player.\n\n" :
        "No unauthorized software detected.\n\n";

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

    // Add "diff" formatting to the fields if any detections happened
    if (!fields.empty()) {
        fields = "```diff" + fields + "\n```";
    }

    // Build the JSON payload.
    std::string payload = "{";
    payload += "\"content\": null,";
    payload += "\"embeds\": [{";
    payload += "\"title\": \"" + title + "\",";
    payload += "\"description\": \"" + description + "\",";
    payload += "\"color\": " + color + ",";

    // Only add "What has been founded" field if there are any detections
    if (loki || midnight || cyrix) {
        payload += "\"fields\": [{";
        payload += "\"name\": \"What has been founded:\",";
        payload += "\"value\": \"" + fields + "\"";
        payload += "}, {";
    }
    else {
        payload += "\"fields\": [{";
        payload += "\"name\": \"\",";
        payload += "\"value\": \"" + fields + "\"";
        payload += "}, {";
    }

    // Always add the "Other" field
    payload += "\"name\": \"Other\",";
    payload += "\"value\": \"" + std::string((loki || midnight || cyrix) ? "" : "No issues detected") + "\""; // Placeholder if no detections
    payload += "}],";  // Close fields array (or empty if no detections)

    payload += "\"author\": {";
    payload += "\"name\": \"" + username + "\"";
    payload += "},";
    payload += "\"footer\": {";
    payload += "\"text\": \"SCPSLAC\"";
    payload += "},";
    payload += "\"timestamp\": \"" + getCurrentTimestamp() + "\",";  // Insert dynamic timestamp here

    // Select the appropriate thumbnail based on loki and midnight status
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
