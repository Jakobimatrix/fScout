#pragma once

#include <assert.h>

#include <string>
#include <vector>

class Needle {
  std::wstring _search_string;
  std::wstring _search_string_low;
  bool _fuzzy_search{false};
  size_t _num_fuzzy_changes{0};
  bool _use_wildcard{false};
  wchar_t _wildcard{'*'};

  std::vector<size_t> _fuzzy_changes_indices;
  std::vector<short> _fuzzy_changes;
  static constexpr short ADD_LETTER = 1;
  static constexpr short REMOVE_LETTER = -1;
  static constexpr short NOTHING = 0;

  size_t currentIndex{0};

 public:
  Needle(const std::wstring& needle);

  void setFuzzySearch(const size_t num_fuzzy_changes);

  void useWildCard(const wchar_t wildcard);

  bool canDoFuzzySearch() const;

  void useFuzzySearchRemoveLetter();
  void useFuzzySearchAddLetter();
  void useFuzzySearchLetterAsWildCard();
  void undo_useFuzzySearchRemoveLetter();
  void undo_useFuzzySearchAddLetter();
  void undo_useFuzzySearchLetterAsWildCard();

  bool nextIsWildCard() const;

  wchar_t getCurrentLetter() const;

  void nextIndex();
  void undo_nextIndex();

  bool found() const;

  bool notIncremented() const;

  size_t getSize() const;

  size_t getMinNecessaryDepth() const;

  std::wstring getEffectiveSearchString() const;
};
