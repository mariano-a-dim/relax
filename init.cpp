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
#include "chatgptclient.h"

namespace po = boost::program_options;

// Global logfile
std::ofstream logfile;


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
        std::cout << "\033[32mUser: \033[0m" << question << "\033[32m\n";

        std::string response = ask_chatgpt(chatgpt_key, question, conversation_history);

        std::cout << "\033[34mIA: " << response << "\033[0m\n";  // Blue color
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
        std::cout << "\033[32mUser: \033[0m";
        std::getline(std::cin, question);

        // Check if the user wants to exit
        if (question == "stop")
        {
            break;
        }

        // Call the function to ask the question to chatgpt
        std::string response = ask_chatgpt(chatgpt_key, question, conversation_history);
        std::cout << "\033[34mIA: " << response << "\033[0m\n";  // Blue color
    }

    // Close the logfile
    logfile.close();

    return 0;
}



