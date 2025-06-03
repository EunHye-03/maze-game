#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

void send_score(const char *username, int score, int level) {
    CURL *curl = curl_easy_init();
    if (curl) {
        char data[256];
        snprintf(data, sizeof(data),
                 "{\"username\":\"%s\",\"score\":%d,\"level\":%d}",
                 username, score, level);

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, "https://your-domain.com/submit");  // 🔁 실제 도메인으로 교체
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl failed: %s\n", curl_easy_strerror(res));
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
    }
}
