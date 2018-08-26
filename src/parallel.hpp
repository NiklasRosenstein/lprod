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

#include <cassert>
#include <cstddef>

#include <algorithm>
#include <list>
#include <unordered_set>
#include <vector>

#include <atomic>
#include <condition_variable>
#include <thread>


namespace parallel
{

  struct distribution_policy
  {

    size_t max_threads = 0;        // Fallback to std::thread::hardware_concurrency()
    size_t min_batch_size = 1;     // No minimum
    size_t max_batch_size = 0;     // No maximum
    size_t batch_size_factor = 4;  // Higher results in smaller batches

    inline distribution_policy& with_max_threads(size_t max_threads_)
    {
      max_threads = max_threads_;
      return *this;
    }

    inline distribution_policy& with_min_batch_size(size_t min_batch_size_)
    {
      min_batch_size = min_batch_size_;
      return *this;
    }

    inline distribution_policy& with_max_batch_size(size_t max_batch_size_)
    {
      max_batch_size = max_batch_size_;
      return *this;
    }

    inline distribution_policy& with_batch_size_factor(size_t batch_size_factor_)
    {
      batch_size_factor = batch_size_factor_;
      return *this;
    }

    inline size_t num_parallel_threads() const
    {
      if (max_threads == 0)
        return std::thread::hardware_concurrency();
      return max_threads;
    }

    inline size_t batch_size(size_t data_size) const
    {
      size_t size = data_size / (num_parallel_threads() * batch_size_factor);
      size = std::max(size, min_batch_size);
      if (max_batch_size > 0) {
        size = std::min(size, max_batch_size);
      }
      return size;
    }

  };

  template <
    typename RESULT_CONTAINER,
    typename DATA_CONTAINER,
    typename FUNC,
    typename DISTRIBUTION_POLICY=distribution_policy
  >
  inline void batch_transform(
    RESULT_CONTAINER& results,
    DATA_CONTAINER const& data,
    FUNC&& fn,
    DISTRIBUTION_POLICY policy={})
  {
    if (data.empty()) {
      return;
    }

    results.clear();

    size_t batch_size = std::max<size_t>(1, std::min(policy.batch_size(data.size()), data.size()));
    size_t num_threads = std::max<size_t>(1, std::min(data.size() / batch_size, policy.num_parallel_threads()));

    using thread_data = std::tuple<std::thread, size_t, RESULT_CONTAINER, bool>;
    std::list<thread_data> running;
    std::list<thread_data> finished;

    // Used to wake up the main thread as soon as one of the processing
    // threads finishes.
    std::mutex m;
    std::condition_variable cv;

    size_t offset = 0;
    size_t batch_index = 0;
    int last_completed_index = -1;
    bool completed_flag = false;
    for (;;) {
      // Submit a new batch if we have the chance.
      if (offset < data.size() && running.size() < num_threads) {
        size_t end = std::min(offset + batch_size, data.size());

        running.emplace_back();
        auto& t = running.back();
        std::get<1>(t) = batch_index;
        std::get<0>(t) = std::thread([&, offset, end] () {
          RESULT_CONTAINER result = fn(data.begin() + offset, data.begin() + end);
          std::unique_lock<std::mutex> lk(m);
          std::get<2>(t) = std::move(result);
          std::get<3>(t) = true;
          completed_flag = true;
          cv.notify_one();
        });

        batch_index++;
        offset = end;
      }

      // If no threads are running we're done.
      if (running.empty() && finished.empty()) {
        break;
      }

      std::unique_lock<std::mutex> lk(m);

      // Wait until a thread is ready if we reached the thread limit.
      // Note: Need "completed_flag" because thread could be done before
      // we wait for it,  leading to a deadlock if only one thread used.
      if (running.size() >= num_threads && !completed_flag) {
        cv.wait(lk);
      }
      completed_flag = false;

      // Move finished threads to the other list/
      for (auto it = running.begin(); it != running.end();) {
        if (std::get<3>(*it)) { // thread is done
          std::get<0>(*it).join();
          auto insert_pos = std::lower_bound(finished.begin(), finished.end(), *it, [] (auto& a, auto& b) { return std::get<1>(a) < std::get<1>(b); });
          finished.insert(insert_pos, std::move(*it));
          running.erase(it++);
        }
        else {
          ++it;
        }
      }

      // Append to the result list as long as we can find continous finished batches.
      while (!finished.empty() && std::get<1>(finished.front()) == last_completed_index+1) {
        auto& t = finished.front();
        last_completed_index = std::get<1>(t);
        results.insert(results.end(), std::get<2>(t).begin(), std::get<2>(t).end());
        finished.pop_front();
      }
    }

    assert(running.size() == 0);
    assert(finished.size() == 0);
  }

} // namespace parallel
