#pragma once
#include <string>
#include <tbb/concurrent_vector.h>
#include "BookInfo.hpp"


void writeResults(
    const tbb::concurrent_vector<BookInfo>& books,
    int count5stars,
    const std::string& longestTitle,
    const std::string& shortestTitle,
    size_t numPages,
    size_t numUniqueUrls,
    double seconds,
    double throughput,
    double avgPrice,
    const std::string& maxTitle,
    double maxPrice,
    const std::string& minTitle,
    double minPrice,
    int count1star
);

void calculateStatistics(
    const tbb::concurrent_vector<BookInfo>& books,
    int& count5Stars,
    std::string& longestTitle,
    std::string& shortestTitle,
    double& maxPrice,
    std::string& maxTitle,
    double& minPrice,
    std::string& minTitle,
    int& count1Star,
    double& avgPrice
);


