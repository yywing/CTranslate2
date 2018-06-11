#pragma once

#include <set>
#include <unordered_map>
#include <string>
#include <vector>

#include "vocabulary.h"

namespace opennmt {

  class VocabularyMap {
  public:
    VocabularyMap(const std::string& map_path, const Vocabulary& vocabulary);
    void get_candidates(const std::vector<std::string>& tokens, std::set<size_t>& candidates) const;

    template <typename T>
    std::vector<T>
    get_batch_candidates(const std::vector<std::vector<std::string>>& batch_tokens) const {
      std::set<size_t> candidates;
      for (const auto& tokens : batch_tokens)
        get_candidates(tokens, candidates);
      return std::vector<T>(candidates.begin(), candidates.end());
    }

  private:
    std::set<size_t> _static_ids;
    std::vector<std::unordered_map<std::string, std::vector<size_t>>> _map_rules;
  };

}