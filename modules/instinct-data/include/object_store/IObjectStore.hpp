//
// Created by RobinQu on 2024/4/25.
//

#ifndef IOBJECTSTORE_HPP
#define IOBJECTSTORE_HPP
#include <data.pb.h>

#include "DataGlobals.hpp"

namespace INSTINCT_DATA_NS {
    template<typename Buffer=std::string, typename InputStream = std::istream, typename OutputStream = std::ostream>
    class IObjectStore {
    public:
        IObjectStore() = default;
        virtual ~IObjectStore() = default;
        IObjectStore(IObjectStore&&)=delete;
        IObjectStore(const IObjectStore&)=delete;

        /**
         * Upload object using input stream
         * @param bucket_name
         * @param object_key
         * @param input_stream
         * @return byte size uploaded
         */
        virtual OSSStatus PutObject(
            const std::string& bucket_name,
            const std::string& object_key,
            InputStream& input_stream
        ) = 0;

        /**
         * Upload object using buffer
         * @param bucket_name
         * @param object_key
         * @param buffer
         * @return
         */
        virtual OSSStatus PutObject(
            const std::string& bucket_name,
            const std::string& object_key,
            const Buffer& buffer
        ) = 0;

        /**
         * Get an object as output stream
         * @param bucket_name
         * @param object_key
         * @param output_stream
         * @return
         */
        virtual OSSStatus GetObject(
            const std::string& bucket_name,
            const std::string& object_key,
            OutputStream& output_stream
        ) = 0;

        /**
         * Get an object as buffer
         * @param bucket_name
         * @param object_key
         * @param buffer
         * @return
         */
        virtual OSSStatus GetObject(
            const std::string& bucket_name,
            const std::string& object_key,
            Buffer& buffer
        ) = 0;

        /**
         * Delete an object
         * @param bucket_name
         * @param object_key
         * @return
         */
        virtual OSSStatus DeleteObject(
            const std::string& bucket_name,
            const std::string& object_key
        ) = 0;

    };

    using ObjectStorePtr = std::shared_ptr<IObjectStore<>>;
}

#endif //IOBJECTSTORE_HPP
