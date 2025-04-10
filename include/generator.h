#ifndef PASSWORD_GENERATOR_H
#define PASSWORD_GENERATOR_H

#include <string>

std::string generatePassword(int length, bool easyToRead, bool easyToSay, bool includeNumbers, bool includeUpper, bool includeLower, bool includeSymbols, const std::string& excludedChars);

std::string removeExcludedChars(const std::string& base, const std::string& excluded);

#endif