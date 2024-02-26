//
// Created by RobinQu on 2024/2/26.
//

#ifndef RESULTSET_HPP
#define RESULTSET_HPP

#include <ranges>

#include "CoreGlobals.hpp"



LC_CORE_NS {
    /**
     * \brief Iterator interface
     * \tparam T
     */
    template<typename T>
    class ResultIterator {
    public:
        virtual ~ResultIterator()=default;
        [[nodiscard]] virtual bool HasNext() const = 0;

        virtual T& Next() const = 0;
    };


    /**
     * \brief Vector-based container conforming ResultIterator interface
     * \tparam T
     */
    template<typename T>
    class ResultVecIterator : public ResultIterator<T>{
        std::vector<T> data_;
        typename std::vector<T>::size_type current_;
    public:
        explicit ResultVecIterator(std::vector<T> data)
            : data_(std::move(data)) {
        }

        explicit ResultVecIterator(std::ranges::input_range auto range): data_(range.begin(), range.end()) {

        }

        [[nodiscard]] bool HasNext() {
            return current_ == data_.size();
        }

        T& Next() const override {
            ++current_;
            return data_[current_];
        }

        std::vector<T>& GetRange() {
            return data_;
        }
    };

    template<typename R, typename Fn=std::is_invocable<R>, typename T = std::invoke_result_t<Fn, R>>
    // template<typename T = >
    class TransofromResultView: public ResultIterator<T> {
        Fn fn_;
        // owning pointer
        ResultIterator<R>* base_;
    public:
        TransofromResultView(Fn fn, ResultIterator<R>* base): fn_(std::move(fn)), base_(base) {

        }

        ~TransofromResultView() {
            delete base_;
        }

        [[nodiscard]] bool HasNext() const override {
            return base_->HasNext();
        }

        [[nodiscard]] T& Next() const override {
            return std::invoke(fn_, std::forward<R>(base_->Next()));
        }
    };

    template<typename R, typename Fn=std::is_invocable<R>, typename T = std::invoke_result_t<Fn, R>>
    ResultIterator<T>* create_transform(Fn&& fn, ResultIterator<R> *itr) {
        return new TransofromResultView<T>(fn, itr);
    }

    auto create_from_range(std::ranges::input_range auto && r) {
        return new ResultVecIterator<std::ranges::range_value_t<decltype(r)>>(r);
    }

}


#endif //RESULTSET_HPP
