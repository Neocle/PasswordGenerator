#ifndef GENERATOR_H
#define GENERATOR_H

#include <string>

std::string generatePassword(int length, bool easyToRead, bool easyToSay,
    bool includeNumbers, bool includeUpper,
    bool includeLower, bool includeSymbols,
    const std::string& excludedChars);

std::string removeExcludedChars(const std::string& input, const std::string& excluded);

bool hasPatterns(const std::string& password);

bool meetsRequirements(const std::string& password, bool includeNumbers,
    bool includeUpper, bool includeLower, bool includeSymbols);

#endif