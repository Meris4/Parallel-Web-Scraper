#pragma once
#include <string>
#include <vector>
#include "BookInfo.hpp"


void calculateStatisticsSerial(
    const std::vector<BookInfo>& books,
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

void writeResultsSerial(
    const std::vector<BookInfo>& books,
    int count5Stars,
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
    int count1Star
);

