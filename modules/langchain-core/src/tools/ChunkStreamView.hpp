//
// Created by RobinQu on 2024/2/25.
//

#ifndef DELEGATECHUNKSTREAM_HPP
#define DELEGATECHUNKSTREAM_HPP

#include "CoreGlobals.hpp"
#include <ranges>

#include <iterator>

LC_CORE_NS {

    // template<typename ChunkRange, typename Itr = std::ranges::iterator_t<ChunkRange>>
    // requires std::ranges::input_range<ChunkRange>
    // template<std::ranges::view V
    // ,typename T=std::iter_value_t<Itr>
    // >
    // requires std::input_iterator<Itr>





    template<typename T>
    class ChunkStreamView {
        // using X = std::ranges::range_value_t<C>;

         // base_;
        // using Itr = std::iter_value_t<>

    public:
        virtual ~ChunkStreamView()=default;
        virtual T& Next() const = 0;
        [[nodiscard]] virtual bool HasNext() const = 0;
    };

    template<typename C, typename T = std::ranges::range_value_t<C>, typename Itr=typename C::iteartor>
    class DelegateChunkStreamView: public ChunkStreamView<T> {
        C base_;
        Itr current_;
        public:
            explicit DelegateChunkStreamView(C base)
                : base_(std::forward<C>(base)), current_(base.begin()) {
            }

            // explicit ChunkStreamView(const C& base)
            //     : base_(base) {
            // }


        T& Next() const override {
            T tmp = *current_;
            ++current_;
            return tmp;
        }

        [[nodiscard]] bool HasNext() const override {
            return base_.end() == current_;
        }
    };

    //
    // template<typename C>
    // requires std::ranges::input_range<C>
    // class ChunkStreamView {
    //     C base_;
    // public:
    //
    //     explicit ChunkStreamView(C base)
    //         : base_(std::forward<C>(base)) {
    //     }
    //
    //     // explicit ChunkStreamView(const C& base)
    //     //     : base_(base) {
    //     // }
    //
    //
    //     [[nodiscard]] auto begin() const{
    //         return base_.begin();
    //     }
    //
    //     [[nodiscard]] auto end() const {
    //         return base_.end();
    //     }
    //
    // };

    template<typename C>
    static auto wrap_chunk_stream_view(C&& c) {
        return DelegateChunkStreamView<C>(c);
    }

}


#endif //DELEGATECHUNKSTREAM_HPP
