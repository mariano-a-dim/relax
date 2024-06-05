# Relax Program

Relax is a command-line program that manages and validates chatgpt keys. It uses the Boost library to handle command-line options and libcurl to interact with the OpenAI API. It also includes a simple logging feature for debugging.

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

## Internet Connection Check

The program checks for an internet connection before making a request to the OpenAI API. If there's no internet connection, it will log an error message to a file named `debug.log`.

## OpenAI API Interaction

The program uses libcurl to make a POST request to the OpenAI API. It sends a question to the chatgpt model and receives a response, which it logs to the `debug.log` file.

## Dependencies

Boost Library
jsoncpp Library
Curl Libreary


## Logging

The program logs debug information to a file named `debug.log`. This includes the result of the internet connection check and the response from the OpenAI API.

## License

