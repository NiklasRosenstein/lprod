| Topic       | Project Type | Language | Platform | Preferred Buildsystem |
| ----------- | ------------ | -------- | -------- | --------------------- |
| [L-Systems] | playground   | C++17    | any      | [Craftr]              |

  [L-Systems]: https://en.wikipedia.org/wiki/L-system
  [Craftr]: https://github.com/craftr-build/craftr

# lprod

A fast L-System producer.

__Project Goals__

* Fast and memory-efficient production of [L-Systems]

__Personal Goals__

* Learn about L-Systems
* Play with C++ threads and synchronization
* Implementing a parallel batch transform function (see `parallel.hpp`)
* Try a different C++ coding style (placing opening curly braces of top level
  constructs on the next line)

__Building__

If you have [Craftr] installed, simply do

    $ craftr -cb --variant=release
    $ ./build/release/lprod/main/main-1.0-0
    usage: lprod [rule [rule [...]] axiom n [--each] [--no-prod] [--time]

Alternatively

    $ g++ src/main.cpp -lpthread -o main
    $ ./main
    usage: lprod [rule [rule [...]] axiom n [--each] [--no-prod] [--time]

__Usage__

A `rule` is of the form `X=S...` where `X` can be any single character and
`S...` is the character sequence that `X` is replaced by in the next iteration
of the L-System.

The `axiom` is the starting point for the `n` productions.

With `--each`, every iteration of the system is printed. Without, only the
final production is printed. Use `--time` to print the computation time of
every production to stderr. Hide the actual output of the production with
`--no-prod`.
