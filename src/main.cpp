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

#include <cstring>
#include <iostream>
#include "lsystem.hpp"
#include "timer.hpp"


int stoi_whole(char const* str)
{
  try {
    size_t s;
    int n = std::stoi(str, &s);
    if (s != strlen(str)) throw std::invalid_argument("");
    return n;
  }
  catch (std::invalid_argument) {
    throw std::invalid_argument("not a number");
  }
}


void pop_arg(int& argc, char**(& argv), int& i)
{
  for (int j = i+1; j < argc; ++j) {
    argv[j-1] = argv[j];
  }
  argc--;
  i--;
}


std::ostream& operator << (std::ostream& s, std::vector<int> const& v)
{
  return s << std::string(v.begin(), v.end());
}


[[noreturn]]
void usage(int code, char const* fatal_msg=nullptr)
{
  printf("usage: lprod [rule [rule [...]] axiom n [--each] [--no-prod] [--time]\n");
  if (fatal_msg) {
    printf("fatal: %s\n", fatal_msg);
  }
  exit(code);
}


int main(int argc, char** argv)
{
  --argc, ++argv;

  if (argc == 0) usage(0);
  if (argc == 1) usage(1);

  bool each = false;
  bool time = false;
  bool no_prod = false;

  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--each") == 0) {
      each = true;
    }
    else if (strcmp(argv[i], "--time") == 0) {
      time = true;
    }
    else if (strcmp(argv[i], "--no-prod") == 0) {
      no_prod = true;
    }
    else continue;
    pop_arg(argc, argv, i);
  }

  // Parse L-System rules and flags from the command-line.
  lsystem::lsystem<lsystem::rule> L;
  for (; argc > 2; --argc, ++argv) {
    if (strchr(*argv, '=') != *argv+1) {
      usage(1, "invalid rule");
    }
    L.define_rule(**argv, (*argv)+2);
  }

  // Parse axiom.
  std::vector<int> prod(*argv, *argv + strlen(*argv));
  --argc, ++argv;

  // Parse number of iterations.
  int n;
  try { n = stoi_whole(*argv); }
  catch (std::invalid_argument e) { usage(1, e.what()); }

  // Generate L-System production.
  std::vector<int> temp;
  long ms_sum = 0;
  for (int i = 0; i < n; ++i) {
    timer t1;
    L.produce_parallel(temp, prod);
    long ms = t1.elapsed();
    ms_sum += ms;
    std::swap(temp, prod);
    if (!no_prod && (each || i == (n-1))) {
      std::cout << prod << "\n";
    }
    if (each && time) {
      std::cerr << "time (" << i << "): " << (double) ms / 1000.0 << "s" << std::endl;
    }
  }

  if (time) {
    std::cerr << "time(all): " << (double) ms_sum / 1000.0 << "s" << std::endl;
  }

  return 0;
}
