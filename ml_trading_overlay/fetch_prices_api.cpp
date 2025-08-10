#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static size_t WriteCallback(void* c, size_t s, size_t n, std::string* out) {
    out->append((char*)c, s*n);
    return s*n;
}

int main() {
    std::string buf;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (auto* curl = curl_easy_init()) {
        const char* url =
            "https://api.binance.com/api/v3/klines?symbol=BTCUSDT&interval=1m&limit=10";
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
        auto res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl error: " << curl_easy_strerror(res) << "\n";
            curl_easy_cleanup(curl);
            curl_global_cleanup();
            return 1;
        }
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    auto j = json::parse(buf, nullptr, false);
    if (j.is_discarded() || !j.is_array()) {
        std::cerr << "bad JSON or not an array\n";
        return 1;
    }

    // Each element is: [openTime, open, high, low, close, volume, closeTime, ...]
    for (const auto& k : j) {
        long long openTime = k[0].get<long long>();
        double open  = std::stod(k[1].get<std::string>());
        double high  = std::stod(k[2].get<std::string>());
        double low   = std::stod(k[3].get<std::string>());
        double close = std::stod(k[4].get<std::string>());
        double vol   = std::stod(k[5].get<std::string>());

        std::cout << openTime << " O:" << open
                  << " H:" << high << " L:" << low
                  << " C:" << close << " V:" << vol << "\n";
    }
    return 0;
}
