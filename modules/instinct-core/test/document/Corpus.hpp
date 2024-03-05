//
// Created by RobinQu on 2024/3/4.
//

#ifndef CORPUS_HPP
#define CORPUS_HPP

#include <unicode/unistr.h>


#include "CoreGlobals.hpp"

namespace INSTINCT_CORE_NS::corpus {
    using namespace U_ICU_NAMESPACE;

    static U32String text1 =
            R"(At age nine, Swift became interested in musical theater and performed in four Berks Youth Theatre Academy productions.[19] She also traveled regularly to New York City for vocal and acting lessons.[20] Swift later shifted her focus toward country music, inspired by Shania Twain's songs, which made her "want to just run around the block four times and daydream about everything".[21] She spent weekends performing at local festivals and events.)";

    static U32String text2 =
            R"(<|endoftext|>The llama (/ˈlɑːmə/; Spanish pronunciation: [ˈʎama] or [ˈʝama]) (Lama glama) is a domesticated South American camelid, widely used as a meat and pack animal by Andean cultures since the pre-Columbian era.
Llamas are social animals and live with others as a herd. Their wool is soft and contains only a small amount of lanolin.[2] Llamas can learn simple tasks after a few repetitions. When using a pack, they can carry about 25 to 30% of their body weight for 8 to 13 km (5–8 miles).[3] The name llama (in the past also spelled "lama" or "glama") was adopted by European settlers from native Peruvians.[4]
The ancestors of llamas are thought to have originated from the Great Plains of North America about 40 million years ago, and subsequently migrated to South America about three million years ago during the Great American Interchange. By the end of the last ice age (10,000–12,000 years ago), camelids were extinct in North America.[3] As of 2007, there were over seven million llamas and alpacas in South America and over 158,000 llamas and 100,000 alpacas, descended from progenitors imported late in the 20th century, in the United States and Canada.[5]
<|fim_prefix|>In Aymara mythology, llamas are important beings. The Heavenly Llama is said to drink water from the ocean and urinates as it rains.[6] According to Aymara eschatology,<|fim_suffix|> where they come from at the end of time.[6]<|fim_middle|> llamas will return to the water springs and ponds<|endofprompt|>)";

    static U32String text3 =
            R"(At age nine, Swift became interested in musical theater and performed in four Berks Youth Theatre Academy productions.[19] She also traveled regularly to New York City for vocal and acting lessons.[20] Swift later shifted her focus toward country music, inspired by Shania Twain's songs, which made her "want to just run around the block four times and daydream about everything".[21] She spent weekends performing at local festivals and events.)";

    static U32String text4 = "hello world 👋";
}

#endif //CORPUS_HPP
