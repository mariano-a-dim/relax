#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <curl/curl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <json/json.h>
#include <arpa/inet.h>

namespace po = boost::program_options;

// Global logfile
std::ofstream logfile;

// Function declarations
bool is_connected();
size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *s);
std::string ask_chatgpt(const std::string &chatgpt_key, const std::string &question);

int main(int argc, char *argv[])
{
    // Variable declarations
    std::string chatgpt_key;
    std::string question;
    std::string key_file = "key.txt";

    // Open the logfile
    logfile.open("debug.log", std::ios_base::app);

    // Command line options setup
    po::options_description desc("Allowed options");
    desc.add_options()("help", "produce help message")("key", po::value<std::string>(&chatgpt_key), "set the chatgpt key")("ask", po::value<std::string>(&question), "ask a question to chatgpt");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    // Help option handling
    if (vm.count("help"))
    {
        std::cout << desc << "\n";
        return 1;
    }

    // Load chatgpt key from file or command line argument
    std::ifstream file(key_file);
    if (file)
    {
        std::getline(file, chatgpt_key);
        std::cout << "chatgpt key was loaded from file.\n";
    }
    else if (vm.count("key"))
    {
        std::regex key_format("^sk-.*$");
        if (std::regex_match(chatgpt_key, key_format))
        {
            std::ofstream out(key_file);
            out << chatgpt_key;
            out.close();
            std::cout << "chatgpt key was saved to file.\n";
        }
        else
        {
            std::cout << "Invalid chatgpt key format.\n";
            return 1;
        }
    }
    else
    {
        std::cout << "chatgpt key was not set.\n";
        return 1;
    }

    // Ask a question to chatgpt
    if (vm.count("ask"))
    {
        if (chatgpt_key.empty())
        {
            std::cout << "chatgpt key is not defined.\n";
            return 1;
        }
        std::string response = ask_chatgpt(chatgpt_key, question);
        std::cout << "Response from chatgpt: " << response << "\n";
    }

    // Close the logfile
    logfile.close();

    return 0;
}

// Check if there is an internet connection
bool is_connected()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &(addr.sin_addr)); // Google DNS
    int res = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr));
    close(sockfd);
    if (res != -1)
    {
        logfile << "Connection successful.\n";
    }
    else
    {
        logfile << "Connection failed.\n";
    }

    return res != -1;
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
    return new_length;
}




std::string ask_chatgpt(const std::string &chatgpt_key, const std::string &question)
{
    // Setup curl and API request
    CURL *curl = curl_easy_init();
    std::string response_string;
    std::string url = "https://api.openai.com/v1/chat/completions";
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + chatgpt_key).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Prepare JSON payload
    Json::Value json;
    Json::Value message;
    message["role"] = "user";
    message["content"] = question;
    json["model"] = "gpt-4o";
    json["messages"].append(message);
    std::string json_string = json.toStyledString();

    // Set curl options and perform the request
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Parse and return the response from chatgpt
    Json::Value response_json;
    std::istringstream response_stream(response_string);
    response_stream >> response_json;
    return response_json["choices"][0]["message"]["content"].asString();
}