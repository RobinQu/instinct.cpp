//
// Created by RobinQu on 2024/5/31.
//

#ifndef SQLBUILDER_HPP
#define SQLBUILDER_HPP
#include "RetrievalGlobals.hpp"


namespace INSTINCT_RETRIEVAL_NS {

    namespace details {
        static void build_bool_query(const BoolQuery& bool_query, std::string& sql);
        static void build_term_query(const TermQuery& term_query, std::string& sql);
        static void build_terms_query(const TermsQuery& terms_query, std::string& sql);
        static void build_search_query(const SearchQuery& search_query, std::string& sql);

        static void build_search_query(const SearchQuery& search_query, std::string& sql) {
            if (search_query.has_bool_()) {
                build_bool_query(search_query.bool_(), sql);
            } else if (search_query.has_term()) {
                build_term_query(search_query.term(), sql);
            } else if(search_query.has_terms()) {
                build_terms_query(search_query.terms(), sql);
            }
        }

        static void build_term_value(const Value& value, std::string& sql) {
            if (value.has_bool_value()) {
                sql += value.bool_value() ? '1' : '0';
            }
            if (value.has_string_value()) {
                sql += "'";
                sql += StringUtils::EscapeSQLText(value.string_value());
                sql += "'";
            }
            if (value.has_number_value()) {
                sql += std::to_string(value.number_value());
            }
            if (value.has_null_value()) {
                sql += "NULL";
            }
        }

        static void build_terms_query(const TermsQuery& terms_query, std::string& sql) {
            sql += terms_query.name();
            sql += " IN (";
            const auto term_values = terms_query.terms() | std::views::transform([&](const Value& term) {
                std::string v;
                build_term_value(term,v);
                return v;
            });
            sql += StringUtils::JoinWith(term_values, ", ");
            sql += ")";
        }

        static void build_term_query(const TermQuery& term_query, std::string& sql) {
            sql += term_query.name();
            sql += "=";
            build_term_value(term_query.term(), sql);

        }

        static void build_bool_query(const BoolQuery& bool_query, std::string& sql) {
            std::vector<std::string> predicates;
            if (bool_query.must_size()>0) {
                std::string predicate;
                for(int i=0; i< bool_query.must_size();++i) {
                    predicate+= "(";
                    build_search_query(bool_query.must(i), predicate);
                    predicate += i==bool_query.must_size()-1 ?  ")" : ") AND ";
                }
                predicates.push_back(predicate);
            }
            if (bool_query.should_size()>0) {
                std::string predicate;
                for(int i=0; i< bool_query.should_size();++i) {
                    predicate+= "(";
                    build_search_query(bool_query.should(i), predicate);
                    predicate += i==bool_query.should_size()-1 ?  ")" : ") OR ";
                }
                predicates.push_back(predicate);
            }
            if (bool_query.mustnot_size()>0) {
                std::string predicate;
                for(int i=0; i< bool_query.mustnot_size();++i) {
                    predicate+= "(NOT ";
                    build_search_query(bool_query.mustnot(i), predicate);
                    predicate += i==bool_query.mustnot_size()-1 ?  ")" : ") AND ";
                }
                predicates.push_back(predicate);
            }
            sql += " ";
            sql += StringUtils::JoinWith(predicates, " AND ");
        }

        static void build_sorters(const std::vector<Sorter>& sorters, std::string& sql) {
            if (sorters.empty()) return;
            std::vector<std::string> sort_strings;
            for(const auto& sorter: sorters) {
                if (sorter.has_field()) {
                    sort_strings.push_back(
                        fmt::format("{} {}", sorter.field().field_name(), sorter.field().order() == DESC ? "DESC" :"ASC")
                    );
                }
            }
            if (!sort_strings.empty()) {
                sql += " ORDER BY ";
                sql += StringUtils::JoinWith(sort_strings, ", ");
            }
        }
    }

    class SQLBuilder {
    public:
        static std::string ToSelectString(
            const std::string& table_name,
            const std::string& column_list,
            const SearchQuery& search_query,
            const std::vector<Sorter>& sorters = {},
            const int offset = -1,
            const int limit = -1) {
            std::string sql = "SELECT ";
            sql += column_list;
            sql += " FROM ";
            sql += table_name;
            sql += " WHERE ";
            details::build_search_query(search_query, sql);
            details::build_sorters(sorters, sql);
            if (limit>0) {
                sql += " LIMIT ";
                sql += std::to_string(limit);
            }
            if (offset>0) {
                sql += " OFFSET ";
                sql += std::to_string(offset);
            }
            return sql;
        }

        static std::string ToDeleteString(
            const std::string& table_name,
            const SearchQuery& search_query) {
            std::string sql = "DELETE FROM ";
            sql += table_name;
            sql += " WHERE ";
            details::build_search_query(search_query, sql);
            return sql;
        }
    };
}


#endif //SQLBUILDER_HPP
