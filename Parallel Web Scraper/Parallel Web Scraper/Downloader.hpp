#pragma once
#include <string>
#include <tbb/concurrent_vector.h>
#include <tbb/concurrent_hash_map.h>

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata);
std::string fetchUrl(const std::string& url, long timeout, int maxRetries);
void downloadPages(
    const tbb::concurrent_vector<std::string>& urls,
    tbb::concurrent_vector<std::string>& htmlPages,
    tbb::concurrent_hash_map<std::string, bool>& visitedUrls
);