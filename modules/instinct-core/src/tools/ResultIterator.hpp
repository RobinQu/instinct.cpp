//
// Created by RobinQu on 2024/2/26.
//

#ifndef RESULTSET_HPP
#define RESULTSET_HPP
#include "CoreGlobals.hpp"


namespace INSTINCT_CORE_NS {
    /**
     * \brief Iterator interface
     * \tparam T
     */
    template<typename T>
    class ResultIterator {
    public:
        virtual ~ResultIterator()=default;
        [[nodiscard]] virtual bool HasNext() const = 0;

        virtual T& Next()  = 0;
    };


    /**
     * \brief Vector-based container conforming ResultIterator interface
     * \tparam T
     */
    template<typename T>
    class ResultVecIterator : public ResultIterator<T>{
        std::vector<T> data_;
        typename std::vector<T>::size_type current_ = {0};
    public:
        explicit ResultVecIterator(std::vector<T> data)
            : data_(std::move(data)) {
        }

        explicit ResultVecIterator(std::ranges::input_range auto range): data_(range.begin(), range.end()) {
        }

        [[nodiscard]] bool HasNext() const override {
            return current_ < data_.size();
        }

        T& Next()  override {
            return data_[current_++];
        }

        std::vector<T>& GetRange() {
            return data_;
        }
    };

    template<typename R, typename Fn, typename T=std::invoke_result_t<Fn, R>>
    requires std::is_invocable_r_v<T, Fn, R>
    class TransofromResultView: public ResultIterator<T> {
        Fn fn_;
        // owning pointer
        ResultIterator<R>* base_;
        std::vector<T> data_;
    public:
        TransofromResultView(Fn fn, ResultIterator<R>* base): fn_(std::move(fn)), base_(base) {

        }
        ~TransofromResultView() override {
            delete base_;
        }

        [[nodiscard]] bool HasNext() const override {
            return base_->HasNext();
        }

        [[nodiscard]] T& Next()  override {
            data_.push_back(std::invoke(fn_, base_->Next()));
            return data_.back();
        }
    };

    template<typename T>
    using ResultIteratorPtr = std::shared_ptr<ResultIterator<T>>;

    template<typename R, typename Fn, typename T=std::invoke_result_t<Fn, R>>
    requires std::is_invocable_r_v<T, Fn, R>
    ResultIteratorPtr<T> create_result_itr_with_transform(Fn&& fn, ResultIteratorPtr<R> itr) {
        return std::make_shared<TransofromResultView<R, Fn, T>>(fn, itr);
    }

    auto create_result_itr_from_range(std::ranges::input_range auto && r) {
        return std::make_shared<ResultVecIterator<std::ranges::range_value_t<decltype(r)>>>(r);
    }

}


#endif //RESULTSET_HPP
