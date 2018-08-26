/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2018  Niklas Rosenstein
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "parallel.hpp"


struct lsystem
{

  std::unordered_map<int, std::vector<int>> rules;

  inline void define_rule(int var, std::vector<int>&& production)
  {
    rules[var] = std::forward<std::vector<int>>(production);
  }

  inline void define_rule(int var, std::string const& production)
  {
    rules[var] = {production.begin(), production.end()};
  }

  template <typename It>
  inline void produce(std::vector<int>& result, It axiom_begin, It axiom_end) const
  {
    result.clear();
    for (auto it = axiom_begin; it != axiom_end; ++it) {
      auto rule = rules.find(*it);
      if (rule == rules.end()) {
        result.push_back(*it);
      }
      else {
        auto& production = rule->second;
        result.insert(result.end(), production.begin(), production.end());
      }
    }
  }

  inline void produce(std::vector<int>& result, std::vector<int> const& axiom) const
  {
    produce(result, axiom.begin(), axiom.end());
  }

  inline void produce(std::vector<int>& result, std::string const& axiom) const
  {
    produce(result, axiom.begin(), axiom.end());
  }

  inline void produce_parallel(std::vector<int>& result, std::vector<int> const& axiom) const
  {
    parallel::batch_transform(
      result,
      axiom,
      [this] (auto begin, auto end) {
        std::vector<int> partial_result;
        this->produce(partial_result, begin, end);
        return partial_result;
      },
      parallel::distribution_policy()
        .with_min_batch_size(8)
        .with_max_batch_size(4098)
    );
  }

};
