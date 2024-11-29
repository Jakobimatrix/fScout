#pragma once

#include <assert.h>

#include <memory>
#include <string>

class Needle {
  std::shared_ptr<const std::wstring> _search_string;
  bool _fuzzy_search{false};
  size_t _num_fuzzy_changes{0};
  bool _use_wildcard{false};
  wchar_t _wildcard{'*'};

  size_t currentIndex{0};

  int score;

 public:
  Needle(std::wstring needle);

  void setFuzzySearch(const size_t num_fuzzy_changes);

  void useWildCard(const wchar_t wildcard);

  bool canDoFuzzySearch() const;

  void useFuzzySeaerch(const wchar_t);

  void useFuzzySeaerch();

  bool nextIsWildCard() const;

  wchar_t front() const;

  void nextIndex();

  bool found() const;

  bool notIncremented() const;

  size_t getSize() const;

  size_t getMinNecessaryDepth() const;
};
