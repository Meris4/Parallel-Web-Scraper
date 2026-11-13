#pragma once
#include <string>
#include <vector>
#include <unordered_map>

size_t writeCallbackSerial(char* ptr, size_t size, size_t nmemb, void* userdata);
std::string fetchUrlSerial(const std::string& url, long timeout, int maxRetries);
void downloadPagesSerial(
    const std::vector<std::string>& urls,
    std::vector<std::string>& htmlPages,
    std::unordered_map<std::string, bool>& visitedUrls
);