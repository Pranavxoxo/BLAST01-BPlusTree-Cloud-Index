// InputUtils.cpp

#include "InputUtils.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#define STDIN_IS_TTY() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define STDIN_IS_TTY() isatty(fileno(stdin))
#endif

namespace input {

bool isScripted() {
    static const bool scripted = !STDIN_IS_TTY();
    return scripted;
}

static std::string trim(const std::string& s) {
    size_t b = 0, e = s.size();
    while (b < e && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
    while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
    return s.substr(b, e - b);
}

std::string readLine(const std::string& prompt) {
    std::cout << prompt << std::flush;
    std::string line;
    if (!std::getline(std::cin, line)) {
        std::cout << "\n";
        throw InputClosed();
    }
    line = trim(line);
    if (isScripted()) std::cout << line << "\n";
    return line;
}

// Parses the whole line as an integer: "12abc" and "" are rejected, not
// silently truncated the way `cin >> int` would.
static bool parseInt(const std::string& text, long& value) {
    if (text.empty()) return false;
    char* end = nullptr;
    value = std::strtol(text.c_str(), &end, 10);
    return *end == '\0';
}

int readInt(const std::string& prompt, int minValue, int maxValue) {
    while (true) {
        std::string text = readLine(prompt);
        long value = 0;
        if (!parseInt(text, value)) {
            std::cout << "  ! \"" << text << "\" is not a whole number. Try again.\n";
        } else if (value < minValue || value > maxValue) {
            std::cout << "  ! Enter a value from " << minValue << " to " << maxValue << ".\n";
        } else {
            return static_cast<int>(value);
        }
    }
}

double readDouble(const std::string& prompt, double minValue, double maxValue) {
    while (true) {
        std::string text = readLine(prompt);
        char* end = nullptr;
        double value = text.empty() ? 0.0 : std::strtod(text.c_str(), &end);
        if (text.empty() || *end != '\0') {
            std::cout << "  ! \"" << text << "\" is not a number. Try again.\n";
        } else if (value < minValue || value > maxValue) {
            std::cout << "  ! Enter a value from " << minValue << " to " << maxValue << ".\n";
        } else {
            return value;
        }
    }
}

// Tenant names: letters, digits, '-' and '_' only, like a real account slug.
std::string readName(const std::string& prompt, size_t maxLength) {
    while (true) {
        std::string text = readLine(prompt);
        bool ok = !text.empty() && text.size() <= maxLength;
        for (char c : text)
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '_') ok = false;
        if (ok) return text;
        std::cout << "  ! Use 1-" << maxLength
                  << " characters: letters, digits, '-' or '_' (no spaces).\n";
    }
}

static bool isLeapYear(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

std::string readDate(const std::string& prompt) {
    static const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    while (true) {
        std::string text = readLine(prompt);
        bool shapeOk = text.size() == 10 && text[4] == '-' && text[7] == '-';
        for (size_t i = 0; shapeOk && i < text.size(); ++i)
            if (i != 4 && i != 7 && !std::isdigit(static_cast<unsigned char>(text[i]))) shapeOk = false;
        if (shapeOk) {
            int y = std::atoi(text.substr(0, 4).c_str());
            int m = std::atoi(text.substr(5, 2).c_str());
            int d = std::atoi(text.substr(8, 2).c_str());
            int maxDay = (m >= 1 && m <= 12) ? daysInMonth[m - 1] + (m == 2 && isLeapYear(y)) : 0;
            if (y >= 2000 && y <= 2100 && maxDay > 0 && d >= 1 && d <= maxDay) return text;
        }
        std::cout << "  ! Enter a real date between 2000 and 2100 as YYYY-MM-DD.\n";
    }
}

int readChoice(const std::string& title, const std::vector<std::string>& options) {
    std::cout << "  " << title << "\n";
    for (size_t i = 0; i < options.size(); ++i)
        std::cout << "    " << (i + 1) << ") " << options[i] << "\n";
    return readInt("  Choose 1-" + std::to_string(options.size()) + ": ", 1,
                   static_cast<int>(options.size())) - 1;
}

bool readYesNo(const std::string& prompt) {
    while (true) {
        std::string text = readLine(prompt);
        if (text == "y" || text == "Y" || text == "yes") return true;
        if (text == "n" || text == "N" || text == "no") return false;
        std::cout << "  ! Please answer y or n.\n";
    }
}

}  // namespace input
