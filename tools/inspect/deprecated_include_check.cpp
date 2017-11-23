//  deprecated_include_check implementation  --------------------------------//

//  Copyright Beman Dawes   2002.
//  Copyright Gennaro Prota 2006.
//  Copyright Hartmut Kaiser 2016.
//  Copyright (c) 2017 Agustin Berge
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config/defines.hpp>

#include <algorithm>

#include "deprecated_include_check.hpp"
#include "function_hyper.hpp"
#include "tokenizer.hpp"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"
#include <vector>

namespace boost { namespace inspect
{
    namespace
    {
        struct include_check
        {
            enum { start, pound, include } state = start;

            bool check_hit(token_match const& t)
            {
                return (t.kind == token::header_name);
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
                    state = (t == "include" ? include : start);
                    break;
                case include:
                    state = start;
                    return check_hit(t);
                }
                return false;
            }
        };

    } // unnamed namespace

    deprecated_includes const names[] =
    {
        { "boost/move/move\\.hpp", "utility" },
        { "boost/atomic/atomic\\.hpp", "boost/atomic.hpp" },
//      { "boost/thread/locks.hpp", "mutex" },
        { "boost/type_traits\\.hpp", "separate type-traits headers" },
        { "boost/unordered_map\\.hpp", "unordered_map" },
        { "boost/unordered_set\\.hpp", "unordered_set" },
        { "boost/utility/enable_if\\.hpp", "type_traits" },
        { "boost/detail/atomic_count\\.hpp", "hpx/util/atomic_count.hpp" },
        { "boost/function\\.hpp", "hpx/util/function.hpp" },
        { "boost/shared_ptr\\.hpp", "memory" },
        { "boost/make_shared\\.hpp", "memory" },
        { "boost/enable_shared_from_this\\.hpp", "memory" },
        { "boost/bind\\.hpp", "hpx/util/bind.hpp" },
        { "boost/(chrono/)?chrono\\.hpp", "chrono" },
        { "boost/(core/)?ref\\.hpp", "functional" },
        { "boost/cstdint\\.hpp", "cstdint" },
        { "boost/thread/barrier\\.hpp", "hpx/compat/barrier.hpp" },
        { "boost/exception_ptr\\.hpp", "exception" },
        { "boost/range/iterator_range\\.hpp", "hpx/util/iterator_range.hpp" },
        { "hpx/hpx_fwd\\.hpp", "nothing (remove unconditionally)" },
        { "boost/preprocessor/cat\\.hpp", "hpx/util/detail/pp/cat.hpp" },
        { "boost/preprocessor/stringize\\.hpp", "hpx/util/detail/pp/stringize.hpp" },
        { "boost/atomic\\.hpp", "atomic" },
        { "boost/nondet_random.hpp", "random" },
        { "boost/random/([^\\s]*)\\.hpp", "random" },
        { "boost/format\\.hpp", "hpx/util/format.hpp" }
    };

    //  deprecated_include_check constructor  -------------------------------//

    deprecated_include_check::deprecated_include_check()
        : m_errors(0)
    {
        // C/C++ source code...
        register_signature(".c");
        register_signature(".cpp");
        register_signature(".cxx");
        register_signature(".h");
        register_signature(".hpp");
        register_signature(".hxx");
        register_signature(".inc");
        register_signature(".ipp");
    }

    //  inspect ( C++ source files )  ---------------------------------------//

    void deprecated_include_check::inspect(
        const string & library_name,
        const path & full_path,      // example: c:/foo/boost/filesystem/path.hpp
        const string & contents)     // contents of file to be inspected
    {
        std::string::size_type p = contents.find("hpxinspect:" "nodeprecatedinclude");
        if (p != string::npos)
        {
            // ignore this directive here (it is handled below) if it is followed
            // by a ':'
            if (p == contents.size() - 30 ||
                (contents.size() > p + 30 && contents[p + 30] != ':'))
            {
                return;
            }
        }

        std::vector<token_match> found_includes;
        include_check check_include;
        tokenize_cpp(
            contents.data() + 0, contents.data() + contents.size(),
            [&](token_match const& t) {
                if (!check_include(t))
                    return;

                found_includes.push_back(t);
            });

        // check for all given includes
        long errors = 0;
        for (deprecated_includes const& d : names)
        {
            boost::regex re(d.include_regex);
            for (auto iter = found_includes.begin();
                iter != found_includes.end(); ++iter)
            {
                token_match const& include = *iter;

                boost::match_results<std::string::const_iterator> m;
                std::string header_name = include.value.str();
                header_name = header_name.substr(1, header_name.size() - 2);
                if (boost::regex_match(header_name, m, re))
                {
                    found_includes.erase(iter);

                    std::string tag("hpxinspect:" "nodeprecatedinclude:" + header_name);
                    if (contents.find(tag) != string::npos)
                        break;

                    char const* it = contents.data();
                    char const* match_it = include.value.first;

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
                    error(library_name, full_path, string(this->name())
                        + " deprecated #include ("
                        + header_name
                        + ") on line "
                        + linelink(full_path, boost::lexical_cast<string>(line_number))
                        + " use " + m.format(d.use_instead) + " instead");

                    break;
                }
            }
        }
    }

}} // namespace boost::inspect

