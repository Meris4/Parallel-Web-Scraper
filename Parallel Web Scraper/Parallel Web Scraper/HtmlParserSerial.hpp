#pragma once
#include <vector>
#include <string>
#include <gumbo.h>
#include "BookInfo.hpp"

std::string getTextSerial(const GumboNode* node);
const GumboNode* findPricePSerial(const GumboNode* node);
const GumboNode* findRatingPSerial(const GumboNode* node);
void parseBookArticleSerial(const GumboNode* node, std::vector<BookInfo>& books);
void parseBooksSerial(const std::string& html, std::vector<BookInfo>& books);
void findPaginationLinksSerial(const GumboNode* node, std::vector<std::string>& links);
void indexSiteSerial(const std::string& start_url, std::vector<std::string>& found_urls);
void parseAllPagesSerial(const std::vector<std::string>& html_pages, std::vector<BookInfo>& books);