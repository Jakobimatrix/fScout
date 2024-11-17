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
constexpr size_t ALLOWED_CHARACTERS_SMALL_LETTERS_START_INDEX = 26;
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
      for (size_t j = pattern::ALLOWED_CHARACTERS_SMALL_LETTERS_START_INDEX;
           j < pattern::allowedFileNameChars.size();
           ++j) {
        if (pattern::allowedFileNameChars[j] != needle[i]) {
          modified[i] = pattern::allowedFileNameChars[j];
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
