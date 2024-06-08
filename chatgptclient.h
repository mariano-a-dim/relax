#ifndef CHATGPTCLIENT_H
#define CHATGPTCLIENT_H

#include <string>
#include <json/json.h>

std::string ask_chatgpt(const std::string &chatgpt_key, const std::string &question, Json::Value &conversation_history);
size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *s);

#endif // CHATGPTCLIENT_H