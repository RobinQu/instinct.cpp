# Functional chaining in `instinct.cpp`

Chaining gets popular these days, as `ranges` is available in C++ 20.

Let's look at a code snippet to process numbers. 

```c++
#include <iostream>
#include <ranges>
 
int main()
{
    auto even = [](int i) { return 0 == i % 2; };
    auto square = [](int i) { return i * i; };
 
    for (int i : std::views::iota(0, 6)
               | std::views::filter(even)
               | std::views::transform(square))
        std::cout << i << ' ';
    std::cout << '\n';
}
```

_Credits to [cppreference.com](https://en.cppreference.com/w/cpp/ranges/filter_view)._

In the example above:

* a non-owning source is created by `std::vies::iota`.
* two operations are applied for each data emitted from source.
* operations are concat using bitwise OR operator `|`, which is called pipe operator as well. This is kind of (good) abuse of bitwise operator as it is overridden to compose a sequence of operations other than manipulating bits data.

The whole processing flow is direct and easy to understand. And in `instinct.cpp`, such programing techniques are also employed to compose complex LLM-based pipeline.


## `StepFunction` and their friends


In `quick_start.cpp`, 


