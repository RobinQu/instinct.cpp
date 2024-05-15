//
// Created by RobinQu on 2024/2/12.
//

#ifndef COREGLOBALS_H
#define COREGLOBALS_H

#include <core.pb.h>
#include <fmt/core.h>
#include <nlohmann/json.hpp>
#include <unicode/unistr.h>
#include <unicode/uversion.h>
#include <google/protobuf/message.h>
#include <fmtlog/fmtlog.h>
#include <BS_thread_pool.hpp>

#define INSTINCT_CORE_NS instinct::core

// another set of marcos to hide log implementation
#define LOG_ERROR loge
#define LOG_WARN logw
#define LOG_INFO logi
#define LOG_DEBUG logd

namespace INSTINCT_CORE_NS {

    static void SetupLogging() {
        // TODO setup coloring for each level
        fmtlog::setLogLevel(fmtlog::DBG);
        fmtlog::startPollingThread();
    }

    template<typename R>
    concept sized_range = std::ranges::input_range<R> && std::ranges::sized_range<R>;

    template<typename T>
    concept numberic = std::integral<T> || std::floating_point<T>;

    template<class>
    inline constexpr bool always_false_v = false;

    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };
    // explicit deduction guide (not needed as of C++20)
    template<class... Ts>
    overloaded(Ts...) -> overloaded<Ts...>;

    template <typename R, typename V>
        concept RangeOf = std::ranges::range<R> && std::same_as<std::ranges::range_value_t<R>, V>;

    using U32String = U_ICU_NAMESPACE::UnicodeString;

    template <typename T>
    concept IsProtobufMessage = std::derived_from<T, google::protobuf::Message>;

    static const std::string METADATA_SCHEMA_PARENT_DOC_ID_KEY = "parent_doc_id";

    static const std::string METADATA_SCHEMA_PAGE_NO_KEY = "page_no";

    static const std::string METADATA_SCHEMA_FILE_SOURCE_KEY = "file_source";

    using MetadataSchemaPtr = std::shared_ptr<MetadataSchema>;

    template<typename  T>
    using Futures = BS::multi_future<T>;

    using ThreadPool = BS::thread_pool;

    /**
     * Heper function to support continuation for future, which is not available even in C++ 20.
     * https://stackoverflow.com/questions/14489935/implementing-futurethen-equivalent-for-asynchronous-execution-in-c11
     * @tparam Work
     * @param f
     * @param w
     * @return
     */
    template <typename Fut, typename Work>
    auto then(Fut& f, Work w) -> std::shared_future<decltype(w(f.get()))>
    {
        return std::async([=]{ w(f.get()); });
    }

    template<typename T>
    static std::vector<T> to_vector(const google::protobuf::RepeatedField<T>& input) {
        std::vector<T> result;
        for(const auto& item: input) {
            result.push_back(item);
        }
        return result;
    }


}

/**
 * format support for PrimitiveType
 */
template <> struct fmt::formatter<INSTINCT_CORE_NS::PrimitiveType>: formatter<string_view> {
    template <typename FormatContext>
    auto format(INSTINCT_CORE_NS::PrimitiveType c, FormatContext& ctx) {
        string_view name = "";
        switch (c) {
            case INSTINCT_CORE_NS::INT32:   name = "INT32"; break;
            case INSTINCT_CORE_NS::INT64: name = "INT64"; break;
            case INSTINCT_CORE_NS::FLOAT: name = "FLOAT"; break;
            case INSTINCT_CORE_NS::DOUBLE: name = "DOUBLE"; break;
            case INSTINCT_CORE_NS::BOOL: name = "BOOL"; break;
            case INSTINCT_CORE_NS::VARCHAR: name = "VARCHAR"; break;
            default: name = "";
        }
        return formatter<string_view>::format(name, ctx);
    }
};


/**
 * format support for std::filesystem::path
 */
template <> struct fmt::formatter<std::filesystem::path>: formatter<string_view> {
    template <typename FormatContext>
    auto format(const std::filesystem::path& c, FormatContext& ctx) {
        return formatter<string_view>::format(c.string(), ctx);
    }
};

#endif //COREGLOBALS_H
