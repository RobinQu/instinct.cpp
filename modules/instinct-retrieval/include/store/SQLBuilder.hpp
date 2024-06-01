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
        static void build_int_range_query(const IntRangeQuery& range_query, std::string& sql);
        static void build_double_range_query(const DoubleRangeQuery& range_query, std::string& sql);

        static void build_int_range_query(const IntRangeQuery& range_query, std::string& sql) {
            std::vector<std::string> ranges;
            if (range_query.has_from()) {
                std::string range;
                range += range_query.name();
                range += range_query.inclusive_start() ? " >= " : " > ";
                range += std::to_string(range_query.from());
                ranges.push_back(range);
            }
            if (range_query.has_to()) {
                std::string range;
                range += range_query.name();
                range += range_query.inclusive_end() ? " <= " : " < ";
                range += std::to_string(range_query.to());
                ranges.push_back(range);
            }
            if (!ranges.empty()) {
                if (ranges.size()>1) {
                    sql += "(";
                    sql += StringUtils::JoinWith(ranges, " AND ");
                    sql += ")";
                } else {
                    sql += ranges[0];
                }
            }
        }

        static void build_double_range_query(const DoubleRangeQuery& range_query, std::string& sql) {
            std::vector<std::string> ranges;
            if (range_query.has_from()) {
                std::string range;
                range += range_query.name();
                range += range_query.inclusive_start() ? " >= " : " > ";
                range += fmt::format("{}", range_query.from());
                ranges.push_back(range);
            }
            if (range_query.has_to()) {
                std::string range;
                range += range_query.name();
                range += range_query.inclusive_end() ? " <= " : " < ";
                range += fmt::format("{}", range_query.to());
                ranges.push_back(range);
            }
            if (!ranges.empty()) {
                if (ranges.size()>1) {
                    sql += "(";
                    sql += StringUtils::JoinWith(ranges, " AND ");
                    sql += ")";
                } else {
                    sql += ranges[0];
                }
            }
        }

        static void build_search_query(const SearchQuery& search_query, std::string& sql) {
            if (search_query.has_bool_()) {
                build_bool_query(search_query.bool_(), sql);
            } else if (search_query.has_term()) {
                build_term_query(search_query.term(), sql);
            } else if(search_query.has_terms()) {
                build_terms_query(search_query.terms(), sql);
            } else if(search_query.has_double_range()) {
                build_double_range_query(search_query.double_range(), sql);
            } else if(search_query.has_int_range()) {
                build_int_range_query(search_query.int_range(), sql);
            }
        }

        static void build_term_value(const google::protobuf::Value& value, std::string& sql) {
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
            const auto term_values = terms_query.terms() | std::views::transform([&](const google::protobuf::Value& term) {
                std::string v;
                build_term_value(term,v);
                return v;
            });
            sql += StringUtils::JoinWith(term_values, ", ");
            sql += ")";
        }

        static void build_term_query(const TermQuery& term_query, std::string& sql) {
            sql += term_query.name();
            sql += " = ";
            build_term_value(term_query.term(), sql);

        }

        static void build_bool_query(const BoolQuery& bool_query, std::string& sql) {
            std::vector<std::string> predicates;
            if (bool_query.must_size()>0) {
                std::string predicate;
                if (const auto n = bool_query.must_size(); n>1) {
                    predicate+= "(";
                    for(int i=0; i< n;++i) {
                        build_search_query(bool_query.must(i), predicate);
                        predicate += i==n-1 ?  "" : " AND ";
                    }
                    predicate += ")";
                } else {
                    build_search_query(bool_query.must(0), predicate);
                }
                predicates.push_back(predicate);
            }
            if (bool_query.should_size()>0) {
                std::string predicate;
                if (const auto n = bool_query.should_size(); n>1) {
                    predicate+= "(";
                    for(int i=0; i< bool_query.should_size();++i) {
                        build_search_query(bool_query.should(i), predicate);
                        predicate += i==n-1 ?  "" : " OR ";
                    }
                    predicate += ")";
                } else {
                    build_search_query(bool_query.should(0), predicate);
                }
                predicates.push_back(predicate);
            }
            if (bool_query.mustnot_size()>0) {
                std::string predicate;
                if(const auto n = bool_query.mustnot_size(); n>1) {
                    predicate+= "(";
                    for(int i=0; i< bool_query.mustnot_size();++i) {
                        predicate+= "NOT ";
                        build_search_query(bool_query.mustnot(i), predicate);
                        predicate += i==n-1 ?  "" : " AND ";
                    }
                    predicate += ")";
                } else {
                    predicate += "NOT ";
                    build_search_query(bool_query.mustnot(0), predicate);
                }
                predicates.push_back(predicate);
            }
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
                sql += "ORDER BY ";
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
            std::vector<std::string> parts = {"SELECT", column_list, "FROM", table_name};
            if (search_query.query_case() != SearchQuery::QUERY_NOT_SET) {
                parts.emplace_back("WHERE");
                std::string sql;
                details::build_search_query(search_query, sql);
                parts.push_back(sql);
            }
            if (!sorters.empty()) {
                std::string sql;
                details::build_sorters(sorters, sql);
                parts.push_back(sql);
            }

            if (limit>0) {
                parts.emplace_back("LIMIT");
                parts.emplace_back(std::to_string(limit));
            }
            if (offset>0) {
                parts.emplace_back("OFFSET");
                parts.push_back(std::to_string(offset));
            }
            return StringUtils::JoinWith(parts, " ") + ";";
        }

        static std::string ToDeleteString(
            const std::string& table_name,
            const SearchQuery& search_query) {
            std::vector<std::string> parts = {"DELETE", "FROM",  table_name};
            if (search_query.query_case() != SearchQuery::QUERY_NOT_SET) {
                parts.emplace_back("WHERE");
                std::string sql;
                details::build_search_query(search_query, sql);
                parts.push_back(sql);
            }
            return StringUtils::JoinWith(parts, " ") + ";";
        }
    };
}


#endif //SQLBUILDER_HPP
