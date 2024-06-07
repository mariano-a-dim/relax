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
std::string ask_chatgpt(const std::string &chatgpt_key, const std::string &question, Json::Value &conversation_history);

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

    // Initialize the conversation history with a system message
    Json::Value conversation_history;
    Json::Value system_message;
    system_message["role"] = "system";
    system_message["content"] = "Hello, I'm a chatbot that can answer your questions. You can ask me anything.";
    conversation_history.append(system_message);

    // Load chatgpt key from file or command line argument
    std::ifstream file(key_file);
    if (file)
    {
        std::getline(file, chatgpt_key);
        std::cout << "chatgpt key was loaded from file.\n";
    }

    if (vm.count("key"))
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
    else if (vm.count("ask"))
    {
        // Check if api key is defined
        if (chatgpt_key.empty())
        {
            std::cout << "chatgpt key is not defined.\n";
            return 1;
        }
        question = vm["ask"].as<std::string>(); // Set question with the ask value
        std::cout << "User: " << question << "\n";
        std::string response = ask_chatgpt(chatgpt_key, question, conversation_history);
        std::cout << "IA: " << response << "\n";
    }
    else
    {
        std::cout << "chatgpt key was not set.\n";
        return 1;
    }

    // Ask a question to chatgpt
    while (true)
    {
        // Ask the user for another question
        std::cout << "User: ";
        std::getline(std::cin, question);

        // Check if the user wants to exit
        if (question == "stop")
        {
            break;
        }

        // Call the function to ask the question to chatgpt
        std::string response = ask_chatgpt(chatgpt_key, question, conversation_history);
        std::cout << "IA: " << response << "\n";
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
    // Print the response to the logfile
    // logfile << "Response from curl: " << *s << std::endl;
    return new_length;
}

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