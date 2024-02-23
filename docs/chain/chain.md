# Chaining

`Callable` interface:

```c++

 template <typename Input,typename Output,typename OutputChunk=Output>
class Callable {
    virtual Output Execute(const Input& input);
    virtual RangeOf<Output> Batch(RangeOf<Input> const auto& input);
    virtual RangeOf<OutputChunk> Stream(const Input& input);
    
    virtual std::future<Output> AsyncExecute(const Input& input);
    virtual Observable<Output> AsyncBatch(RangeOf<Input> const auto& input);
    virtual Observable<OutputChunk> AsyncStream(const Input& input);
}



```