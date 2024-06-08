#include "chatgptclient.h"
#include <curl/curl.h>
#include <sstream>
#include <fstream>

extern std::ofstream logfile;

// Ask a question to chatgpt
std::string ask_chatgpt(const std::string &chatgpt_key, const std::string &question, Json::Value &conversation_history)
{
    // Setup curl and API request
    CURL *curl = curl_easy_init();
    std::string response_string;
    std::string url = "https://api.openai.com/v1/chat/completions";
    // https://platform.openai.com/docs/api-reference/chat
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + chatgpt_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Prepare JSON payload
    Json::Value json;
    json["model"] = "gpt-4o";
    json["temperature"] = 0.5;

    // Append the new user message to the conversation history
    Json::Value user_message;
    user_message["role"] = "user";
    user_message["content"] = question;
    conversation_history.append(user_message);

    // Insert the conversation history into the JSON payload
    json["messages"] = conversation_history;

    std::string json_string = json.toStyledString();

    logfile << "Request to chatgpt: " << json_string << std::endl;

    // Set curl options and perform the request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  // Enable verbose output
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Parse and return the response from chatgpt
    Json::Value response_json;
    std::istringstream response_stream(response_string);
    response_stream >> response_json;

    // Append the assistant's response to the conversation history
    Json::Value assistant_message;
    assistant_message["role"] = "assistant";
    assistant_message["content"] = response_json["choices"][0]["message"]["content"].asString();
    conversation_history.append(assistant_message);

    return assistant_message["content"].asString();
}

// Callback function for writing curl response
size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *s)
{
    size_t new_length = size * nmemb;
    size_t old_length = s->size();
    try
    {
        s->resize(old_length + new_length);
    }
    catch (std::bad_alloc &e)
    {
        return 0;
    }
    std::copy((char *)contents, (char *)contents + new_length, s->begin() + old_length);
    // Print the response to the logfile
    // logfile << "Response from curl: " << *s << std::endl;
    return new_length;
}