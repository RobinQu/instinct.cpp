//
// Created by RobinQu on 2024/3/14.
//

#ifndef REACTIVEFUNCTIONS_HPP
#define REACTIVEFUNCTIONS_HPP


#include <rpp/rpp.hpp>
#include <instinct/core_global.hpp>
#include <instinct/exception/instinct_exception.hpp>

namespace INSTINCT_CORE_NS {
    template<typename T>
    using AsyncIterator = rpp::dynamic_observable<T>;


    template<typename T, typename R>
        requires RangeOf<R, T>
    static AsyncIterator<T> CreateAsyncIteratorWithRange(R&& range) {
        return rpp::source::from_iterable(std::forward<R>(range))
                .as_dynamic();
    }

    template<typename T>
    static AsyncIterator<T> CreateAsyncIteratorWithError(const std::string& msg) {
        return rpp::source::error<std::string>(std::make_exception_ptr(InstinctException(msg)));
    }

    /**
     * Error form upstream will be thrown in on_error callback
     * @tparam T
     * @param async_iterator
     * @return
     */
    template<typename T>
    static std::vector<T> CollectVector(const AsyncIterator<T>& async_iterator) {
        std::vector<T> result;
        async_iterator
                | rpp::operators::as_blocking()
                | rpp::operators::subscribe(
                    [&](auto&& t) { result.push_back(t); },
                    [](const std::exception_ptr& err) { if (err) std::rethrow_exception(err); }
                );
        return result;
    }

    template<typename T, typename OBV>
    requires rpp::constraint::observable_of_type<OBV,T>
    static void CollectVector(const OBV& async_iterator, std::vector<T>& result) {
        async_iterator
        | rpp::operators::as_blocking()
        | rpp::operators::subscribe(
                [&](auto&& t) { result.push_back(t); }
        );
    }


    template<typename T>
    static void PrintingSubscriber(const T& t) {
        LOG_INFO(t);
    }

}

#endif //REACTIVEFUNCTIONS_HPP
