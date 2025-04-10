#include "./include/generator.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <random>
#include <algorithm>

std::string generatePassword(int length, bool easyToRead, bool easyToSay, bool includeNumbers, bool includeUpper, bool includeLower, bool includeSymbols, const std::string& excludedChars) {
    std::string numbers = "0123456789";
    std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string lower = "abcdefghijklmnopqrstuvwxyz";
    std::string symbols = "!@#$%^&*()-_=+[]{}|;:,.<>?/";

    std::string readableNumbers = "2346789";
    std::string readableUpper = "ABCDEFGHJKLMNPQRSTUVWXYZ";
    std::string readableLower = "abcdefghijkmnopqrstuvwxyz";

    numbers = removeExcludedChars(numbers, excludedChars);
    upper = removeExcludedChars(upper, excludedChars);
    lower = removeExcludedChars(lower, excludedChars);
    symbols = removeExcludedChars(symbols, excludedChars);
    readableNumbers = removeExcludedChars(readableNumbers, excludedChars);
    readableUpper = removeExcludedChars(readableUpper, excludedChars);
    readableLower = removeExcludedChars(readableLower, excludedChars);

    std::random_device rd;
    std::mt19937 gen(rd());

    if (easyToRead) {
        std::string charSet;
        if (includeUpper) charSet += readableUpper;
        if (includeLower) charSet += readableLower;
        if (includeNumbers) charSet += readableNumbers;
        if (includeSymbols) charSet += removeExcludedChars("!@#$%", excludedChars);

        if (charSet.empty()) return "Error: No readable character set after exclusions!";

        std::uniform_int_distribution<> dist(0, charSet.size() - 1);
        std::string password;
        while (password.size() < length) {
            char c = charSet[dist(gen)];
            if (password.find(c) == std::string::npos || password.size() < 4) {
                password += c;
            }
        }

        std::shuffle(password.begin(), password.end(), gen);
        return password;
    }

    else if (easyToSay) {
        std::vector<std::string> consonants = {
            "b", "c", "d", "f", "g", "h", "j", "k", "l", "m",
            "n", "p", "r", "s", "t", "v", "w", "y", "z", "qu", "th", "ch", "ph", "sh"
        };
        std::vector<std::string> vowels = { "a", "e", "i", "o", "u", "ae", "oo", "ou", "ia", "ei" };

        consonants.erase(std::remove_if(consonants.begin(), consonants.end(),
            [&](const std::string& s) { return excludedChars.find(s[0]) != std::string::npos; }), consonants.end());

        vowels.erase(std::remove_if(vowels.begin(), vowels.end(),
            [&](const std::string& s) { return excludedChars.find(s[0]) != std::string::npos; }), vowels.end());

        if (consonants.empty() || vowels.empty()) return "Error: No valid syllables after exclusions!";

        std::uniform_int_distribution<> consonantDist(0, consonants.size() - 1);
        std::uniform_int_distribution<> vowelDist(0, vowels.size() - 1);
        std::uniform_int_distribution<> numberDist(0, readableNumbers.size() - 1);

        std::string password;
        while (password.length() < length) {
            password += consonants[consonantDist(gen)];
            password += vowels[vowelDist(gen)];

            if (includeNumbers && password.length() + 1 < length && rand() % 3 == 0) {
                password += readableNumbers[numberDist(gen)];
            }
        }

        return password.substr(0, length);
    }

    std::string charSet;
    if (includeNumbers) charSet += numbers;
    if (includeUpper) charSet += upper;
    if (includeLower) charSet += lower;
    if (includeSymbols) charSet += symbols;

    if (charSet.empty()) return "Error: No character set selected after exclusions!";

    std::uniform_int_distribution<> dist(0, charSet.length() - 1);
    std::string password;
    for (int i = 0; i < length; ++i) {
        password += charSet[dist(gen)];
    }
    return password;
}

std::string removeExcludedChars(const std::string& input, const std::string& excluded) {
    std::string result;
    for (char c : input) {
        if (excluded.find(c) == std::string::npos) {
            result += c;
        }
    }
    return result;
}