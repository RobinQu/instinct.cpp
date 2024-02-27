//
// Created by RobinQu on 2024/2/2.
//

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>


namespace ns {
    // a simple struct to model a person
    struct person {
        std::string name;
        std::string address;
        int age;
    };
    void to_json(nlohmann::json& j, const person& p) {
        j = nlohmann::json{{"name", p.name}, {"address", p.address}, {"age", p.age}};
    }

    void from_json(const nlohmann::json& j, person& p) {
        j.at("name").get_to(p.name);
        j.at("address").get_to(p.address);
        j.at("age").get_to(p.age);
    }

    bool operator==(const person&l, const person&r) {
        return l.name == r.name && l.address == r.address && l.age == r.age;
    }
}

TEST(JSONTest, TestConvert) {


    ns::person p = {"Ned Flanders", "744 Evergreen Terrace", 60};

    nlohmann::json j = p;

    std::cout << j << std::endl;
    // {"address":"744 Evergreen Terrace","age":60,"name":"Ned Flanders"}

    // conversion: json -> person
    auto p2 = j.template get<ns::person>();

    // that's it
    assert(p == p2);

}

