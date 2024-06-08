# Relax Program

Relax is a command-line chatgpt terminal client. 
It uses the Boost library to handle command-line options and libcurl to interact with the OpenAI API. 
It uses gpt-4o model

## Installation

Clone the repository and build the program using Meson and Ninja:

```bash
git clone https://github.com/mariano-a-dim/relax.git
cd relax
meson builddir
ninja -C builddir
```

## Usage

You can pass the chatgpt key to the program using the --key option:

```bash
./relax --key YOUR_CHATGPT_KEY
```

The program will validate the key and save it to a file. If the key file already exists, the program will load the key from the file instead of using the one passed with the --key option.

## Key Validation

The program validates the chatgpt key format using a regular expression. The key should start with sk-.


## OpenAI API Interaction

The program uses libcurl to make POST request's to the OpenAI API. It sends question's to the chatgpt model and receives a response, which it logs to the `debug.log` file.


## Dependencies

- Boost Library
- jsoncpp Library
- Curl Libreary

### Instalation on Arch based linux

```bash
sudo pacman -Syu boost jsoncpp curl
```

### Instalation on Debian based linux

```bash
sudo apt-get update
sudo apt-get install libboost-all-dev libjsoncpp-dev libcurl4-openssl-dev
```
## Logging

The program logs debug information to a file named `debug.log`. the response from the OpenAI API.

## TODOs

- Add support to set temperature and model
- Save conversation to a file or SQL database table or not relational one
- Parse and resume previous conversations so that i can follow a new conversation with a seed conversation
- Generate package's to install on windows, linux and mac
- Add a option to skip a response (user does not want to save or have a partucular question in the conversation)
- Add support for test to speech ot the other way around

