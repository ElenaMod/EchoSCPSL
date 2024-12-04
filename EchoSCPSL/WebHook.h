#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <string>
#include <vector>

class Webhook {
public:
    Webhook(const std::string& url);
    void sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames);

private:
    std::string webhookUrl;
    std::string buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames);
    std::string getCurrentTimestamp();
};

#endif
