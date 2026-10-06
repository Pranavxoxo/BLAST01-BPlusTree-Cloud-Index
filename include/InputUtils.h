// InputUtils.h
// Line-based, validated console input. Every reader re-prompts until it gets
// something valid, so a typo never leaves std::cin in a failed state.
// If standard input ends (Ctrl-D / end of a piped file) they throw InputClosed
// so the menu can exit cleanly instead of looping forever.

#ifndef INPUT_UTILS_H
#define INPUT_UTILS_H

#include <stdexcept>
#include <string>
#include <vector>

struct InputClosed : std::runtime_error {
    InputClosed() : std::runtime_error("standard input closed") {}
};

namespace input {

// True when input is piped from a file. The app then echoes what it reads, so
// a recorded run shows the values that were "typed".
bool isScripted();

std::string readLine(const std::string& prompt);
int         readInt(const std::string& prompt, int minValue, int maxValue);
double      readDouble(const std::string& prompt, double minValue, double maxValue);
std::string readName(const std::string& prompt, size_t maxLength);
std::string readDate(const std::string& prompt);              // YYYY-MM-DD
int         readChoice(const std::string& title, const std::vector<std::string>& options);
bool        readYesNo(const std::string& prompt);

}  // namespace input

#endif
