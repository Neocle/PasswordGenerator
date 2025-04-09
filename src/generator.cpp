#include "./include/generator.h"
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

std::string generatePassword(int length, bool easyToRead, bool easyToSay, bool includeNumbers, bool includeUpper, bool includeLower, bool includeSymbols) {
    std::string numbers = "0123456789";
    std::string upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::string lower = "abcdefghijklmnopqrstuvwxyz";
    std::string symbols = "!@#$%^&*()-_=+[]{}|;:,.<>?/";

    std::string readableNumbers = "2346789";
    std::string readableUpper = "ABCDEFGHJKLMNPQRSTUVWXYZ";
    std::string readableLower = "abcdefghijkmnopqrstuvwxyz";

    std::string charSet;

    if (easyToRead) {
        charSet = readableUpper + readableLower + readableNumbers;
    }
    else if (easyToSay) {
        std::vector<std::string> consonants = { "b", "c", "d", "f", "g", "h", "j", "k", "l", "m", "n", "p", "r", "s", "t", "v", "w", "y", "z" };
        std::vector<std::string> vowels = { "a", "e", "i", "o", "u" };

        srand(time(0));
        std::string password;
        for (int i = 0; i < length / 2; ++i) {
            password += consonants[rand() % consonants.size()];
            password += vowels[rand() % vowels.size()];
        }
        if (length % 2 == 1) {
            password += consonants[rand() % consonants.size()];
        }
        return password;
    }
    else {
        if (includeNumbers) charSet += numbers;
        if (includeUpper) charSet += upper;
        if (includeLower) charSet += lower;
        if (includeSymbols) charSet += symbols;
    }

    if (charSet.empty()) return "Error: No character set selected!";

    srand(time(0));
    std::string password;
    for (int i = 0; i < length; ++i) {
        password += charSet[rand() % charSet.length()];
    }
    return password;
}
