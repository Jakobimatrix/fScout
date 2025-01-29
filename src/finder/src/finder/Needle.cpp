#include <finder/Needle.h>


Needle::Needle(const std::wstring &needle)
    : _search_string(needle), _fuzzy_changes(), _fuzzy_changes_indices() {
  _search_string_low = _search_string;
  std::transform(_search_string_low.begin(),
                 _search_string_low.end(),
                 _search_string_low.begin(),
                 ::tolower);
}

void Needle::setFuzzySearch(const size_t num_fuzzy_changes) {
  _fuzzy_search = true;
  _num_fuzzy_changes = num_fuzzy_changes;

  _fuzzy_changes = std::vector<short>(num_fuzzy_changes, 0);
  _fuzzy_changes_indices = std::vector<size_t>(num_fuzzy_changes, NOTHING);
}

void Needle::useWildCard(const wchar_t wildcard) {
  _use_wildcard = true;
  _wildcard = wildcard;
}

bool Needle::canDoFuzzySearch() const { return _num_fuzzy_changes > 0; }

void Needle::useFuzzySearchRemoveLetter() {
  assert(_num_fuzzy_changes != 0);
  --_num_fuzzy_changes;
  _fuzzy_changes[_num_fuzzy_changes] = REMOVE_LETTER;
  _fuzzy_changes_indices[_num_fuzzy_changes] = currentIndex;
  ++currentIndex;
}

void Needle::useFuzzySearchAddLetter() {
  assert(_num_fuzzy_changes != 0);
  --_num_fuzzy_changes;
  _fuzzy_changes[_num_fuzzy_changes] = ADD_LETTER;
  _fuzzy_changes_indices[_num_fuzzy_changes] = currentIndex;
}

void Needle::useFuzzySearchLetterAsWildCard() {
  assert(_num_fuzzy_changes != 0);
  --_num_fuzzy_changes;
  _fuzzy_changes[_num_fuzzy_changes] = NOTHING;
  _fuzzy_changes_indices[_num_fuzzy_changes] = currentIndex;
  ++currentIndex;
}

void Needle::undo_useFuzzySearchRemoveLetter() {
  --currentIndex;
  _fuzzy_changes[_num_fuzzy_changes] = NOTHING;
  ++_num_fuzzy_changes;
}

void Needle::undo_useFuzzySearchAddLetter() {
  _fuzzy_changes[_num_fuzzy_changes] = NOTHING;
  ++_num_fuzzy_changes;
}

void Needle::undo_useFuzzySearchLetterAsWildCard() {
  --currentIndex;
  //_fuzzy_changes[_num_fuzzy_changes] = NOTHING;
  ++_num_fuzzy_changes;
}


bool Needle::nextIsWildCard() const {
  return _use_wildcard && _search_string_low[currentIndex] == _wildcard;
}

wchar_t Needle::getCurrentLetter() const {
  return _search_string_low[currentIndex];
}

void Needle::nextIndex() {
  ++currentIndex;
  assert(currentIndex < _search_string.size() + 1);
}

void Needle::undo_nextIndex() {
  assert(currentIndex != 0);
  --currentIndex;
}

std::wstring Needle::getEffectiveSearchString() const {
  std::wstring currentSearch = _search_string;
  // The info in _fuzzy_changes is reverse, so I dont need to take care if currentSearch got greater or smaller
  for (size_t i = 0; i < _fuzzy_changes.size(); ++i) {
    if (_fuzzy_changes[i] == ADD_LETTER) {
      currentSearch.insert(currentSearch.begin() + _fuzzy_changes_indices[i], '*');
    } else if (_fuzzy_changes[i] == REMOVE_LETTER) {
      currentSearch.erase(currentSearch.begin() + _fuzzy_changes_indices[i]);
    }
  }
  return currentSearch;
}

bool Needle::found() const { return currentIndex == _search_string.size(); }

bool Needle::notIncremented() const { return currentIndex == 0; }

size_t Needle::getSize() const { return _search_string.size(); }

size_t Needle::getMinNecessaryDepth() const {
  return _search_string.size() - currentIndex - _num_fuzzy_changes;
}
