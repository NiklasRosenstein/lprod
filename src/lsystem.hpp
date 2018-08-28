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


namespace lsystem
{

  struct rule
  {
    std::vector<int> subst;

    inline rule() { }

    inline rule(std::vector<int>&& subst_) : subst(std::forward<std::vector<int>>(subst_)) { }

    inline rule(std::string const& subst_) : subst(subst_.begin(), subst_.end()) { }

    inline void append_to(std::vector<int>& v) const
    {
      v.insert(v.end(), subst.begin(), subst.end());
    }
  };

  struct rule_interface
  {
    virtual ~rule_interface() { }
    virtual void append_to(std::vector<int>&) = 0;
  };

  template <typename RuleImpl>
  struct rule_wrapper : rule_interface
  {
    using rule_impl = RuleImpl;

    rule_impl impl;

    inline rule_wrapper() : impl{} { }

    template <typename... Args>
    inline rule_wrapper(Args&&... args) : impl(std::forward<Args>(args)...) { }

    virtual ~rule_wrapper() { }

    virtual void append_to(std::vector<int>& v) override
    {
      impl.append_to(v);
    }

    virtual rule_impl* operator * () { return &impl; }

    virtual rule_impl const* operator * () const { return &impl; }
  };

  template <typename RuleType>
  struct lsystem
  {

    using rule_type = RuleType;

    std::unordered_map<int, rule_type> rules;

    inline void define_rule(int variable, rule_type&& r)
    {
      rules[variable] = std::forward<rule_type>(r);
    }

    template <typename... Args>
    inline void define_rule(int variable, Args&&... args)
    {
      rules[variable] = rule_type(std::forward<Args>(args)...);
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
          rule->second.append_to(result);
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

} // namespace lsystem
