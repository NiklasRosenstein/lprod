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

#include <iostream>
#include "lsystem.hpp"
#include "timer.hpp"


int main()
{
  lsystem L;
  L.define_rule('A', "B-A-B");
  L.define_rule('B', "A+B+A");

  std::vector<int> result;
  std::vector<int> axiom;
  axiom.push_back('A');

  timer t;
  for (int i = 0; i < 13; ++i) {
    L.produce_parallel(result, axiom);
    std::swap(result, axiom);
  }

  std::cout << std::string(axiom.begin(), axiom.end()) << "\n";
  return 0;
}
