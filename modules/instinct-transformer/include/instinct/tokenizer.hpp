//
// Created by root on 5/24/24.
//

#ifndef CXX_TEST_TOKENIZER_HPP
#define CXX_TEST_TOKENIZER_HPP
#include <vector>
#include <set>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include <regex>
#include <cstring>
#include <limits>

#include <instinct/./config.hpp>


namespace INSTINCT_TRANSFORMER_NS::tokenizer {

    enum token_type {
        UNDEFINED = 0,
        NORMAL = 1,
        UNKNOWN = 2,
        CONTROL = 3,
        USER_DEFINED = 4,
        UNUSED = 5,
        BYTE = 6,
    };

    static size_t utf8_len(char src) {
        const size_t lookup[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 3, 4};
        uint8_t highbits = static_cast<uint8_t>(src) >> 4;
        return lookup[highbits];
    }


    struct _vocab {
        using id = int32_t;
        using token = std::string;

        struct token_score {
            token tok{};
            float score{};
            token_type type{};
        };

        std::unordered_map<token, id> token_to_id;
        std::vector<token_score> id_to_token;

        std::unordered_map<id, token> special_tokens_cache;
        std::map<std::pair<std::string, std::string>, int> bpe_ranks;

        int find_bpe_rank(const std::string &token_left, const std::string &token_right) const {
            auto it = bpe_ranks.find(std::make_pair(token_left, token_right));
            if (it == bpe_ranks.end()) {
                return -1;
            }

            return it->second;
        }

        bool is_token_of_type(id id, token_type t) const {
            if ((id < 0) || (id >= (int) id_to_token.size())) return false;
            return t == id_to_token[id].type;
        }

        bool is_normal_token(id id) const {
            return is_token_of_type(id, token_type::NORMAL);
        }

        bool is_control_token(id id) const {
            return is_token_of_type(id, token_type::CONTROL);
        }
    };


    class TextPreprocessor {
    public:
        virtual ~TextPreprocessor() = default;

        virtual std::string transform(const std::string &s) = 0;
    };


    class Processor {
    public:
        virtual ~Processor() = default;

        struct TokenId {
            std::string token;
            int id;
        };

        Processor() : piece_size(0), id_unk_token(-1), token_unk_id("<?>"), ret_special_token(false) {}

        virtual size_t Load(const char *buffer, int n_vocab) = 0;

        virtual int PieceToId(std::string_view piece) const {
            auto r = vocab_.token_to_id.find(std::string(piece));
            return r != vocab_.token_to_id.end() ? r->second : id_unk_token;
        }

        virtual std::string IdToPiece(int id) const {
            if (token_override.contains(id))
                return token_override.find(id)->second;

            if (id < 0) return token_unk_id;
            return id < (int) vocab_.id_to_token.size() ? vocab_.id_to_token[id].tok : token_unk_id;
        }

        virtual int Encode(const std::string &input,
                           std::vector<std::string> *pieces) const {
            std::vector<int> ids;
            Encode(input, &ids);
            for (auto id: ids)
                pieces->emplace_back(IdToPiece(id));
            return 0;
        }

        // Given a UTF8 input, encodes it into a sequence of ids.
        virtual int Encode(const std::string &input,
                           std::vector<int> *ids) const {
            std::string s = input;
            for (auto &p: pp)
                s = p->transform(s);

            while (true) {
                size_t special_pos = std::string::npos;
                int special_id = -1;
                size_t special_tok_len = 0;

                for (auto &tok: added_tokens) {
                    size_t pos = s.find(tok.token);

                    if (pos < special_pos) {
                        special_pos = pos;
                        special_id = tok.id;
                        special_tok_len = tok.token.size();
                    }
                }

                if (std::string::npos == special_pos)
                    break;

                std::string part = s.substr(0, special_pos);
                DoEncode(part, ids);

                s = s.substr(special_pos + special_tok_len);
                ids->push_back(special_id);
            }

            DoEncode(s, ids);

            return 0;
        }

        // Given a sequence of ids, decodes it into a detokenized output.
        virtual int Decode(const std::vector<int> &ids,
                           std::string *detokenized) const {
            std::string_view view;
            for (auto id: ids)
                detokenized->append(IdToPiece(id));
            return 0;
        }

        int GetPieceSize() const { return piece_size; }

        [[maybe_unused]] void SetIdUnknownToken(int id) { id_unk_token = id; }

        void SetTokenUnknownId(const std::string &s) { token_unk_id = s; }

        void EnableReturnSpecialToken(bool en) { ret_special_token = en; }

        void RegisterPreprocessor(TextPreprocessor *prep) {
            pp.push_back(std::unique_ptr<TextPreprocessor>(prep));
        }

        void OverrideTokenDecoding(int id, const std::string &tok) {
            token_override.emplace(id, tok);
        }

        void AddAddedToken(const std::string &tok, int id) {
            added_tokens.emplace_back(TokenId{tok, id});
        }

    protected:
        virtual int DoEncode(const std::string &input, std::vector<int> *ids) const = 0;

    protected:
        _vocab vocab_{};
        int piece_size;
        int id_unk_token;
        std::string token_unk_id;
        bool ret_special_token;
        std::vector<std::unique_ptr<TextPreprocessor>> pp{};
        std::map<int, std::string> token_override{};
        std::vector<TokenId> added_tokens{};
    };

    class BaseTokenizer {
    public:
        explicit BaseTokenizer(const BaseConfig &config) :
                bos_token_id(config.bos_token_id),
                eos_token_id(config.eos_token_id),
                pad_token_id(config.pad_token_id),
                sep_token_id(config.sep_token_id),
                tp(nullptr),
                max_length(config.max_length),
                auto_add_bos(true) {}

        virtual ~BaseTokenizer() = default;

        virtual size_t load(const char *buffer, int n_vocab) = 0;

        virtual void encode(const std::string &text, std::vector<int> &ids) const {
            std::string input = preprocess(text);
            tp->Encode(input, &ids);
        }

        [[nodiscard]] virtual std::vector<int> encode(const std::string &text) const {
            std::vector<int> ids;
            encode(text, ids);
            return ids;
        }

        virtual void encode_qa(const std::string &q, const std::string &a, std::vector<int> &ids) const {
            std::string input = preprocess(q);
            tp->Encode(input, &ids);

            input = preprocess(a);
            tp->Encode(input, &ids);
        }

        [[nodiscard]] virtual std::string decode(const std::vector<int> &ids) const {
            // filter out special tokens
            std::vector<int> normal_ids(ids);
            normal_ids.erase(std::remove_if(normal_ids.begin(), normal_ids.end(),
                                            [this](int id) { return is_special_id(id); }),
                             normal_ids.end());
            std::string text;
            tp->Decode(normal_ids, &text);
            text = postprocess(text);
            return text;
        }

        virtual void set_additional_args(const std::map<std::string, std::string> &args) {}

        [[nodiscard]] virtual bool is_terminate_token_id(int id) const {
            if (terminate_ids.empty())
                return id == eos_token_id;

            return terminate_ids.find(id) != terminate_ids.end();
        }

        [[nodiscard]] virtual bool is_special_id(int id) const { return false; }


        int bos_token_id;
        int eos_token_id;
        int pad_token_id;
        int sep_token_id;

    protected:

        virtual std::string preprocess(const std::string &text) const {
            return text;
        }

        virtual std::string postprocess(const std::string &text) const {
            return text;
        }

    public:
        Processor *tp;
    protected:
        const int max_length;
        bool auto_add_bos;
        std::set<int> terminate_ids;
    };
    using TokenizerPtr = std::shared_ptr<BaseTokenizer>;

    class Reader {
    public:
        explicit Reader(const char *buffer)
                : buffer(buffer), offset(0) {}

        void read_raw(void *r, size_t size) {
            memcpy(r, buffer + offset, size);
            offset += size;
        }

        uint32_t read_u32(void) {
            uint32_t r;
            read_raw(&r, sizeof(r));
            return r;
        }

        int32_t read_i32(void) {
            int32_t r;
            read_raw(&r, sizeof(r));
            return r;
        }

        std::string read_string(int len) {
            std::vector<char> chars(len);
            read_raw(chars.data(), len);
            return std::string(chars.data(), len);
        }

        [[nodiscard]] size_t get_total_size() const { return offset; }

    private:
        const char *buffer;
        size_t offset;
    };


    static int load_vocab_list(_vocab &vocab, Reader &reader, bool has_score, bool has_type, int start_piece_id) {
        int count = 0;
        while (true) {
            int len = reader.read_i32();

            if (len < 0) {
                break;
            }

            int id = start_piece_id + count;
//        CHATLLM_CHECK((size_t)(id) < vocab.id_to_token.size()) << "too many extra tokens";
            GGML_ASSERT((size_t) (id) < vocab.id_to_token.size());

            std::string word = reader.read_string(len);

            float score = 0.0f;
            uint8_t type = token_type::NORMAL;
            if (has_score)
                reader.read_raw(&score, sizeof(score));
            if (has_type)
                reader.read_raw(&type, sizeof(type));

            vocab.token_to_id[word] = id;

            auto &tok_score = vocab.id_to_token[id];
            tok_score.tok = std::move(word);
            tok_score.score = score;
            tok_score.type = (token_type) type;

            count++;
        }

        return count;
    }


    struct unigram_tokenizer {
        using index = int;

        struct best {
            best(int prev, float score, int tok_id = 0) : prev(prev), score(score), tok_id(tok_id) {}

            index prev;
            float score;
            index tok_id;
        };

        struct unigram {
            unigram(const char *text, int pos, int n) : text(text), start(pos), end(pos + n) {}

            const char *text;
            index start, end;
        };

        unigram_tokenizer(const _vocab &vocab, int tok_max_len, int unk_id) : vocab_(vocab), tok_max_len(tok_max_len),
                                                                              unk_id(unk_id) {}

        void tokenize(const std::string &text, std::vector<_vocab::id> &output) {
            size_t offs = 0;

            symbols_.emplace_back(text.c_str(), 0, 0);
            trace.emplace_back(0, 0.0f);

            while (offs < text.size()) {
                size_t char_len = std::min(text.size() - offs, utf8_len(text[offs]));
                symbols_.emplace_back(text.c_str() + offs, (int) offs, (int) char_len);
                offs += char_len;
            }

            if (symbols_.size() <= 1)
                return;

            // Viterbi algorithm
            for (int i = 1; i < (int) symbols_.size(); i++) {
                find_best(i);
            }

            // backtrace
            int prev = (int) (trace.size()) - 1;
            std::vector<int> temp;
            while (prev != 0) {
                auto &b = trace[prev];
                temp.push_back(b.tok_id);
                prev = b.prev;
            }
            for (int i = (int) temp.size() - 1; i >= 0; i--)
                output.push_back(temp[i]);
        }

    private:
        void find_best(int pos) {
            int start = pos - tok_max_len;
            if (start < 0) start = 0;

            float max_prop = std::numeric_limits<float>::lowest();
            int prev = -1;
            int tok_id = -1;

            for (int i = start; i < pos; i++) {
                auto &b = trace[i];
                auto &sym = symbols_[i];

                const std::string text = std::string(sym.text + sym.end - sym.start, symbols_[pos].end - sym.end);

                auto token = vocab_.token_to_id.find(text);
                if (token == vocab_.token_to_id.end())
                    continue;

                auto &tok = vocab_.id_to_token[token->second];

                if (b.score + tok.score > max_prop) {
                    prev = i;
                    max_prop = b.score + tok.score;
                    tok_id = token->second;
                }
            }

            if (prev < 0) {
                auto i = pos - 1;
                auto &b = trace[i];
                auto &tok = vocab_.id_to_token[unk_id];

                prev = i;
                max_prop = b.score + tok.score;
                tok_id = unk_id;
            }

            trace.emplace_back(prev, max_prop, tok_id);
        }

        const _vocab &vocab_;
        std::vector<best> trace;
        std::vector<unigram> symbols_;
        int tok_max_len;
        int unk_id;
    };

    class UnigramProcessor final : public Processor {
    public:
        explicit UnigramProcessor(int unk_tok_id) :
                Processor::Processor(), unk_tok_id(unk_tok_id), tok_max_len(0) {};

        size_t Load(const char *buffer, int n_vocab) override {
            Reader reader(buffer);

            vocab_.token_to_id.clear();
            vocab_.id_to_token.resize((size_t) n_vocab + 100);

            piece_size = load_vocab_list(vocab_, reader, true, false, 0);
            vocab_.id_to_token.resize(piece_size);

            // TODO: tok_max_len should be number of characters, not bytes
            for (auto &tok: vocab_.id_to_token) {
                if (tok.tok.size() > tok_max_len)
                    tok_max_len = tok.tok.size();
            }

            return reader.get_total_size();
        }

        int unk_tok_id;

    private:
        int DoEncode(const std::string &input,
                     std::vector<int> *ids) const override {
            unigram_tokenizer tokenizer(vocab_, (int) tok_max_len, unk_tok_id);
            tokenizer.tokenize(input, *ids);
            return 0;
        }

    private:
        size_t tok_max_len;
    };


    class TextPrepNewlineToSpaces : public TextPreprocessor {
    public:
        std::string transform(const std::string &s) override {
            const static std::regex r(R""([\r\n]+)"");
            return std::regex_replace(s, r, " ");
        }
    };


    class TextPrepDeleteMultiSpaces : public TextPreprocessor {
    public:
        std::string transform(const std::string &s) override {
            const static std::regex r(R""( {2,})"");
            return std::regex_replace(s, r, " ");
        }
    };


    class TextPrepAddLeadingSpace : public TextPreprocessor {
    public:
        std::string transform(const std::string &s) override {
            if (s.size() < 1) return " ";
            return s[0] == ' ' ? s : " " + s;
        }
    };



}

#endif //CXX_TEST_TOKENIZER_HPP
