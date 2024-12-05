#ifndef WEBHOOK_H
#define WEBHOOK_H

#include <string>
#include <vector>

class Webhook {
public:
    Webhook(const std::string& url);
    void sendWebhookMessage(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime, bool isDiagTrackRunning, const std::string& uptimeDiagTrack, bool isDpsRunning, const std::string& uptimeDps, bool isPcaSvcRunning, const std::string& uptimePcaSvc, bool isSysMainRunning, const std::string& uptimeSysMain, bool isCdpSvcRunning, const std::string& uptimeCdpSvc, bool isSsdpsrvRunning, const std::string& uptimeSsdpsrv, bool isUmRdpServiceRunning, const std::string& uptimeUmRdpService);

private:
    std::string webhookUrl;
    std::string buildJsonPayload(const std::string& username, bool loki, bool midnight, bool cyrix, const std::vector<std::string>& usernames, const std::string& recycleBinClearTime, bool isDiagTrackRunning, const std::string& uptimeDiagTrack, bool isDpsRunning, const std::string& uptimeDps, bool isPcaSvcRunning, const std::string& uptimePcaSvc, bool isSysMainRunning, const std::string& uptimeSysMain, bool isCdpSvcRunning, const std::string& uptimeCdpSvc, bool isSsdpsrvRunning, const std::string& uptimeSsdpsrv, bool isUmRdpServiceRunning, const std::string& uptimeUmRdpService);
    std::string getCurrentTimestamp();
};

#endif
