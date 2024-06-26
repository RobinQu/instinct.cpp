//
// Created by RobinQu on 2024/3/4.
//

#ifndef CORPUS_HPP
#define CORPUS_HPP

#include <unicode/unistr.h>


#include <instinct/CoreGlobals.hpp>

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


    static U32String text5 = R"(The Great Gold Robbery took place on the night of 15 May 1855, when a shipment of gold to Paris was stolen from the guard's van of the rail service between London and Folkestone.

There were four robbers: two employees of the rail company, a former employee and Edward Agar, a career criminal. They took wax impressions of the keys to the train safes and made copies. One of them ensured he was on guard duty when a shipment was taking place, and Agar hid in the guard's van.

They emptied the safes of 224 pounds (102 kg) of gold, valued at £12,000 (approximately equivalent to £1,193,000 in 2021); the theft was only discovered in Paris.

When Agar was arrested for another crime, his former girlfriend, in need of funds, revealed the details of the theft. Agar admitted his guilt and testified as a witness. )";

    static U32String text6 = R"(位于巴西东北部的帕拉伊巴州当前下辖4个中级地理区、15个直接地理区及223个市镇。该州人口4,039,277人，占巴西人口的1.908%，在巴西各州及联邦区名列第15；面积56,467.242平方千米（21,802.124平方英里），占巴西国土的0.664%，在巴西各州及联邦区排名第21位。若昂佩索阿为帕拉伊巴州首府、人口数最多及人口密度最高的市镇，其人口超过80万人，而人口数最少的市镇为帕拉里，人口密度最低的市镇为圣若昂杜蒂格雷。蒙泰鲁为面积最大的市镇，而博博雷马则为面积最小的市镇。)";
}

#endif //CORPUS_HPP
