//  assert_macro_check implementation  ------------------------------------------------//

//  Copyright Eric Niebler 2010.
//  Based on the assert_macro_check checker by Marshall Clow
//  Copyright (c) 2017 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config/defines.hpp>

#include "assert_macro_check.hpp"
#include "function_hyper.hpp"
#include "tokenizer.hpp"
#include <functional>
#include <string>
#include "boost/lexical_cast.hpp"
#include "boost/filesystem/operations.hpp"

namespace fs = boost::filesystem;

namespace boost { namespace inspect
{
    namespace
    {
        struct undef_check
        {
            enum { start, pound, undef } state = start;

            bool check_hit(token_match const& t)
            {
                return (t == "assert");
            }

            bool operator()(token_match const& t)
            {
                if (t.kind == token::comment || t.kind == token::white_space)
                    return false;

                switch (state)
                {
                case start:
                    state = (t == "#" ? pound : start);
                    break;
                case pound:
                    state = (t == "undef" ? undef : start);
                    break;
                case undef:
                    state = start;
                    return check_hit(t);
                }
                return false;
            }
        };

        struct identifier_check
        {
            enum { start, identifier } state = start;

            bool check_hit(token_match const& t)
            {
                return (t == "assert");
            }

            bool operator()(token_match const& t)
            {
                if (t.kind == token::comment || t.kind == token::white_space)
                    return false;

                switch (state)
                {
                case start:
                    state = (check_hit(t) ? identifier : start);
                    break;
                case identifier:
                    state = start;
                    return t == "(";
                }
                return false;
            }
        };

    } // unnamed namespace

    assert_macro_check::assert_macro_check() : m_files_with_errors(0)
    {
        register_signature(".c");
        register_signature(".cpp");
        register_signature(".cxx");
        register_signature(".h");
        register_signature(".hpp");
        register_signature(".hxx");
        register_signature(".ipp");
    }

    void assert_macro_check::inspect(
        const string & library_name,
        const path & full_path,   // example: c:/foo/boost/filesystem/path.hpp
        const string & contents)     // contents of file to be inspected
    {
        if (contents.find("hpxinspect:" "noassert_macro") != string::npos)
            return;

        long errors = 0;
        undef_check check_undef;
        identifier_check check_identifier;
        tokenize_cpp(
            contents.data() + 0, contents.data() + contents.size(),
            [&](token_match const& t) {
                if (!check_undef(t) && !check_identifier(t))
                    return;

                char const* it = contents.data();
                char const* match_it = t.value.first;

                std::size_t line_number = 1;
                char const* line_start = it;
                for (; it != match_it; ++it) {
                    if (*it == '\n') {
                        ++line_number;
                        line_start = it + 1; // could be end()
                    }
                }

                std::string lineloc = linelink(full_path,
                    boost::lexical_cast<string>(line_number));
                ++errors;
                error( library_name, full_path,
                    "C-style assert macro on line " + lineloc);
            });

        if (errors > 0)
            ++m_files_with_errors;
    }

}} // namespace boost::inspect
