#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <string>

class Webhook {
public:
    Webhook(const std::string& url);
    void sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix);

private:
    std::string webhookUrl;
    std::string buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix);
    std::string getCurrentTimestamp();
};

#endif
