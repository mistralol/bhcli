
#include <sstream>
#include <syslog.h>

#include <json/json.h>
#include <curl/curl.h>


#include "bhcli.h"
#include "Rest.h"

static size_t callback(void *contents, size_t size, size_t nmemb, std::string *str) {
    str->append((char *) contents, size * nmemb);
    return size*nmemb;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winline"
int Rest::Post(const std::string &URL, const Json::Value &Request, Json::Value &Response) {
    static const std::string Agent = std::string("bhcli/") + VERSION;
    long ret = 500;
    CURL *curl = curl_easy_init();
    if (!curl) {
        abort();
    }

    std::stringstream ss;
    ss << Request << std::endl;
    std::string data = ss.str();
    std::string str = "";

    curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, Agent.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charsets: utf-8");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    do {
        CURLcode res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            LogError("curl_easy_perform() failed: %s", curl_easy_strerror(res));
            ret = 500;
            break;
        }

        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &ret);

        Json::Reader reader;
        if (reader.parse(str, Response) == false) {
            LogError("Cannot Parse json response: %s", str.c_str());
            ret = 500;
            break;
        }
    } while(0);

    /* always cleanup */
    curl_easy_cleanup(curl);

    curl_slist_free_all(headers);
    return (int) ret;
}
#pragma GCC diagnostic pop
