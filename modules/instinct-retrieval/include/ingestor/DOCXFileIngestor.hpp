//
// Created by RobinQu on 2024/3/26.
//

#ifndef DOCXFILEINGESTOR_HPP
#define DOCXFILEINGESTOR_HPP

#include "BaseIngestor.hpp"

namespace INSTINCT_RETRIEVAL_NS {
    class DOCXFileIngestor final: public BaseIngestor {
    public:
        AsyncIterator<Document> Load() override;
    };
}


#endif //DOCXFILEINGESTOR_HPP
