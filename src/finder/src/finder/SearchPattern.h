#pragma once
#include <algorithm>
#include <array>
#include <string>
#include <vector>

namespace pattern {
#ifdef _WIN32
#define OS_WINDOWS
#else
#define OS_LINUX
#endif

// clang-format off
#ifdef OS_WINDOWS
constexpr std::array<char, 85> allowedFileNameChars = {{
  'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
  '0','1','2','3','4','5','6','7','8','9',
  '!','#','$','%','&','\'','(',')','-','@','^','_','`','{','}','~','+','=','.',',',';',']','['
}};
#endif

#ifdef OS_LINUX
constexpr std::array<char, 95> allowedFileNameChars = {{
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '!', '#', '$', '%', '&', '\'', '(', ')', '-', '@', '^', '_', '`', '{', '}', '~', '+', '=', '.', ',', ';', ']', '[', ' ', '\\', '*', '?', ':', '|', '\"', '<', '>', '\''
}};
#endif
} // namespace pattern
// clang-format on


class SearchPattern {
 public:
  virtual ~SearchPattern() = default;

  virtual std::vector<std::string> generateNeedles(const std::string& needle) const = 0;
};


class ExactMatchPattern : public SearchPattern {
 public:
  std::vector<std::string> generateNeedles(const std::string& needle) const override {
    return {{needle}};
  }
};


class FuzzyMatchPattern : public SearchPattern {
 public:
  std::vector<std::string> generateNeedles(const std::string& needle) const override {
    std::vector<std::string> needles;
    for (size_t i = 0; i < needle.length(); ++i) {
      std::string modified = needle;
      for (char c = 'a'; c <= 'z'; ++c) {
        if (c != needle[i]) {
          modified[i] = c;
          needles.push_back(modified);
        }
      }
    }
    return needles;
  }
};


class SubsetPattern : public SearchPattern {
  std::size_t min_subsize;

 public:
  explicit SubsetPattern(std::size_t min_subsize) : min_subsize(min_subsize) {}

  std::vector<std::string> generateNeedles(const std::string& needle) const override {
    std::vector<std::string> subsets;

    if (min_subsize > needle.size()) {
      return subsets;
    }

    // Loop through the needle to generate all possible subsets of size >= min_subsize
    for (std::size_t start = 0; start < needle.size() - min_subsize; ++start) {
      for (std::size_t length = min_subsize; length <= needle.size() - start; ++length) {
        std::string subset = needle.substr(start, length);
        subsets.emplace_back(subset);
      }
    }

    return subsets;
  }
};

class WildcardPattern : public SearchPattern {
  char _wildcard;

 public:
  explicit WildcardPattern(char wildcard) : _wildcard(wildcard) {}

  std::vector<std::string> generateNeedles(const std::string& needle) const override {
    std::vector<std::string> subsets;
    generateNeedlesRecursively(needle, 0, subsets);
    return subsets;
  }

 private:
  void generateNeedlesRecursively(const std::string& needle,
                                  size_t index,
                                  std::vector<std::string>& subsets) const {
    // Find the next occurrence of the wildcard
    size_t wildcardPos = needle.find(_wildcard, index);

    // If there are no more wildcards, add the current needle to subsets
    if (wildcardPos == std::string::npos) {
      subsets.push_back(needle);
      return;
    }

    // For each allowed character, replace the wildcard and recurse
    for (char allowedChar : pattern::allowedFileNameChars) {
      std::string modifiedNeedle = needle;
      modifiedNeedle[wildcardPos] = allowedChar;  // Replace the wildcard
      generateNeedlesRecursively(modifiedNeedle, wildcardPos + 1, subsets);  // Recurse with the next position
    }
  }
};
