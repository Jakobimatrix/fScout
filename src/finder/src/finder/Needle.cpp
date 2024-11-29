#include <finder/Needle.h>


Needle::Needle(std::wstring needle)
    : _search_string(std::make_shared<std::wstring>(std::move(needle))) {}

void Needle::setFuzzySearch(const size_t num_fuzzy_changes) {
  _fuzzy_search = true;
  _num_fuzzy_changes = num_fuzzy_changes;
}

void Needle::useWildCard(const wchar_t wildcard) {
  _use_wildcard = true;
  _wildcard = wildcard;
}

bool Needle::canDoFuzzySearch() const { return _num_fuzzy_changes > 0; }

void Needle::useFuzzySeaerch(const wchar_t) { --_num_fuzzy_changes; }

void Needle::useFuzzySeaerch() { --_num_fuzzy_changes; }


bool Needle::nextIsWildCard() const {
  return _use_wildcard && (*_search_string)[currentIndex] == _wildcard;
}

wchar_t Needle::front() const { return (*_search_string)[currentIndex]; }

void Needle::nextIndex() {
  ++currentIndex;
  assert(currentIndex < _search_string->size());
}

bool Needle::found() const {
  return currentIndex >= _search_string->size() - 1;
}

bool Needle::notIncremented() const { return currentIndex == 0; }

size_t Needle::getSize() const { return _search_string->size(); }

size_t Needle::getMinNecessaryDepth() const {
  return _search_string->size() - currentIndex - _num_fuzzy_changes;
}
