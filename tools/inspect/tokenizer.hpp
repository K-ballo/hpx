//  Copyright (c) 2017 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_TOKENIZER_HPP
#define BOOST_TOKENIZER_HPP

///////////////////////////////////////////////////////////////////////////////
#include <cstddef>
#include <cstring>
#include <string>
#include <utility>

namespace boost { namespace inspect
{
    enum class token
    {
        invalid,
        header_name,
        identifier,
        pp_number,
        op_or_punc,
        comment,
        white_space
    };

    struct lexeme
      : std::pair<char const*, char const*>
    {
        lexeme()
          : pair(nullptr, nullptr)
        {}

        lexeme(char const* cstr)
          : pair(cstr, cstr + std::strlen(cstr))
        {}

        lexeme(char const* first, char const* last)
          : pair(first, last)
        {}

        std::string str() const
        {
            return std::string(first, second - first);
        }

        char const* data() const noexcept
        {
            return first;
        }

        std::size_t size() const noexcept
        {
            return second - first;
        }
    };

    inline bool operator==(lexeme const& lhs, lexeme const& rhs)
    {
        return lhs.size() == rhs.size() &&
            std::strncmp(lhs.data(), rhs.data(), lhs.size()) == 0;
    }

    inline bool operator!=(lexeme const& lhs, lexeme const& rhs)
    {
        return !(lhs == rhs);
    }

    inline bool operator==(lexeme const& lhs, char const* rhs)
    {
        return std::strlen(rhs) == lhs.size() &&
            std::strncmp(lhs.data(), rhs, lhs.size()) == 0;
    }

    inline bool operator==(lexeme const& lhs, std::string const& rhs)
    {
        return lhs.size() == rhs.size() &&
            std::strncmp(lhs.data(), rhs.data(), lhs.size()) == 0;
    }

    struct token_match
    {
        token kind;
        lexeme value;
    };

    inline bool operator==(token_match const& lhs, token_match const& rhs)
    {
        return lhs.kind == rhs.kind && lhs.value == rhs.value;
    }

    inline bool operator!=(token_match const& lhs, token_match const& rhs)
    {
        return !(lhs == rhs);
    }

    inline bool operator==(token_match const& lhs, char const* rhs)
    {
        return lhs.value == rhs;
    }

    inline bool operator==(token_match const& lhs, std::string const& rhs)
    {
        return lhs.value == rhs;
    }
}}

///////////////////////////////////////////////////////////////////////////////
#include <algorithm>
#include <functional>
#include <iterator>
#include <regex>
#include <string>
#include <vector>

namespace boost { namespace inspect
{
    namespace detail
    {
        class regex_token
        {
            std::regex _re;

        public:
            regex_token(std::string const& pattern)
              : _re(pattern)
            {}

            template <typename BidirIterator>
            BidirIterator operator()(BidirIterator first, BidirIterator last) const
            {
                std::match_results<BidirIterator> results;
                bool const matched = std::regex_search(
                    first, last,
                    results, _re,
                    std::regex_constants::match_continuous
                  | std::regex_constants::match_not_null);

                return matched ? results[0].second : first;
            }
        };
    }

    //! header-name:
    //!     <h-char-sequence>
    //!     "q-char-sequence"
    class header_name
      : public detail::regex_token
    {
    public:
        static token const kind = token::header_name;

    public:
        header_name()
          : detail::regex_token("[<\"](.*)[>\"]")
        {}
    };

    //! identifier:
    //!     identifier-nondigit
    //!     identifier identifier-nondigit
    //!     identifier digit
    //!     identifier-nondigit:
    //!     nondigit
    //!     universal-character-name
    //!     other implementation-defined characters
    //!     nondigit: one of
    //!     a b c d e f g h i j k l m
    //!     n o p q r s t u v w x y z
    //!     A B C D E F G H I J K L M
    //!     N O P Q R S T U V W X Y Z _
    //!     digit: one of
    //!     0 1 2 3 4 5 6 7 8 9
    class identifier
      : public detail::regex_token
    {
    public:
        static token const kind = token::identifier;

    public:
        identifier()
          : detail::regex_token("[a-zA-Z_][a-zA-Z0-9_]*")
        {}
    };

    //! pp-number:
    //!     digit
    //!     . digit
    //!     pp-number digit
    //!     pp-number identifier-nondigit
    //!     pp-number e sign
    //!     pp-number E sign
    //!     pp-number .
    class pp_number
      : public detail::regex_token
    {
    public:
        static token const kind = token::pp_number;

    public:
        pp_number()
          : detail::regex_token("[0-9.][0-9.a-zA-Z+-]*")
        {}
    };

    //! op-or-punc: one of
    //!     { } [ ] # ## ( )
    //!     <: :> <% %> %: %:%: ; : ...
    //!     new delete ? :: . .*
    //!     + - * / % ^ & | ~
    //!     ! = < > += -= *= /= %=
    //!     ^= &= |= << >> >>= <<= == !=
    //!     <= >= && || ++ -- , ->* ->
    //!     and and_eq bitand bitor compl not not_eq
    //!     or or_eq xor xor_eq
    class op_or_punc
    {
    public:
        static token const kind = token::op_or_punc;

    private:
        static std::vector<std::string> make_items()
        {
            std::vector<std::string> items{
                "{", "}", "[", "]", "#", "##", "(", ")",
                "<:", ":>", "<%", "%>", "%:", "%:%:", ";", ":", "...",
                "new", "delete", "?", "::", ".", ".*",
                "+", "-", "*", "/", "%", "^", "&", "|", "~",
                "!", "=", "<", ">", "+=", "-=", "*=", "/=", "%=",
                "^=", "&=", "|=", "<<", ">>", ">>=", "<<=", "==", "!=",
                "<=", ">=", "&&", "||", "++", "--", ",", "->*", "->",
                "and", "and_eq", "bitand", "bitor", "compl", "not", "not_eq",
                "or", "or_eq", "xor", "xor_eq" };
            std::sort(items.begin(), items.end(), std::greater<std::string>());
            return items;
        }

    public:
        template <typename Iterator>
        Iterator operator()(Iterator first, Iterator last) const
        {
            static std::vector<std::string> const items = make_items();

            for (std::string const& item : items)
            {
                auto miss = std::mismatch(item.begin(), item.end(), first, last);
                if (miss.first == item.end())
                    return miss.second;
            }

            return first;
        }
    };

    //! comment:
    class comment
    {
    public:
        static token const kind = token::comment;

    public:
        template <typename Iterator>
        Iterator operator()(Iterator first, Iterator last) const
        {
            Iterator iter = first;
            if (*iter++ == '/' && iter != last)
            {
                if (*iter == '/' && ++iter != last) // single line
                {
                    while (iter != last && *iter != '\r' && *iter != '\n')
                        ++iter;
                    return iter;
                } else if (*iter == '*' && ++iter != last) { // multi line
                    do {
                        while (iter != last && *iter != '*')
                            ++iter;
                        if (++iter != last && *iter == '/')
                            return iter;
                    } while (iter != last);
                }
            }
            return first;
        }
    };

    //! white-space:
    //!     white-space-character
    //!     white-space white-space-character
    //!     white-space-character: one of
    //!     space horizontal-tab new-line vertical-tab form-feed
    class white_space
    {
    public:
        static token const kind = token::white_space;

    public:
        template <typename Iterator>
        Iterator operator()(Iterator first, Iterator last) const
        {
            static std::string const items = { ' ', '\t', '\n', '\v', '\r' };

            while (first != last && items.find_first_of(*first) != -1)
                ++first;
            return first;
        }
    };

    //! invalid:
    class invalid
    {
    public:
        static token const kind = token::invalid;

    public:
        template <typename Iterator>
        Iterator operator()(Iterator first, Iterator last) const
        {
            return ++first;
        }
    };
}}

///////////////////////////////////////////////////////////////////////////////
#include <cassert>
#include <cstddef>
#include <utility>

namespace boost { namespace inspect
{
    namespace detail
    {
        template <typename Token>
        void tokenize_one(
            char const* first, char const* last,
            Token const& token, token_match& match)
        {
            char const* mark = token(first, last);

            lexeme const value(first, mark);
            if (value.size() > match.value.size())
            {
                match.kind = token.kind;
                match.value = value;
            }
        }

        template <typename ...Tokens>
        token_match tokenize(
            char const* first, char const* last,
            Tokens const&... tokens)
        {
            token_match match;
            match.kind = static_cast<token>(-1);
            match.value = lexeme();

            int _sequencer[] = {(tokenize_one(first, last, tokens, match), 0)...};
            (void)_sequencer;
            return match;
        }
    }

    template <typename UnaryFunction, typename ...Tokens>
    bool tokenize(
        char const* first, char const* last, UnaryFunction out,
        Tokens const&... tokens)
    {
        while (first != last)
        {
            token_match match = detail::tokenize(first, last, tokens...);
            if (match.kind == static_cast<token>(-1))
                break;

            assert(match.value.size() > 0 && "lexeme cannot be empty");
            out(match);

            first += match.value.size();
        }
        return first == last;
    }

    template <typename UnaryFunction>
    bool tokenize_cpp(
        char const* first, char const* last, UnaryFunction out)
    {
        return tokenize(first, last, std::move(out),
            header_name{}, identifier{}, pp_number{}, op_or_punc{},
            comment{}, white_space{}, invalid{});
    }
}}

#endif // BOOST_TOKENIZER_HPP
