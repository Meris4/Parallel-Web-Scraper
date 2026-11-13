#pragma once
#include <vector>
#include <string>
#include <gumbo.h>
#include "BookInfo.hpp"
#include <tbb/concurrent_vector.h>

std::string getText(const GumboNode* node);
const GumboNode* findPriceP(const GumboNode* node);
const GumboNode* findRatingP(const GumboNode* node);
void parseBookArticle(const GumboNode* node, std::vector<BookInfo>& books);
void parseBooks(const std::string& html, std::vector<BookInfo>& books);
void findPaginationLinks(const GumboNode* node, std::vector<std::string>& links);
void parallelIndexSite(const std::string& startUrl, tbb::concurrent_vector<std::string>& foundUrls);
void parseAllPages(const tbb::concurrent_vector<std::string>& htmlPages, tbb::concurrent_vector<BookInfo>& books);