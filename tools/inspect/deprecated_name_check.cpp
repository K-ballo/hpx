//  deprecated_name_check implementation  -----------------------------------//

//  Copyright Beman Dawes   2002.
//  Copyright Gennaro Prota 2006.
//  Copyright Hartmut Kaiser 2016.
//
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config/defines.hpp>

#include <algorithm>

#include "deprecated_name_check.hpp"
#include "function_hyper.hpp"
#include "tokenizer.hpp"
#include "boost/regex.hpp"
#include "boost/lexical_cast.hpp"

#include <set>
#include <string>
#include <vector>

namespace boost { namespace inspect
{
    namespace
    {
        struct name_check
        {
            token_match hit;
            enum { start, identifier, scope } state = start;

            bool check_hit(token_match const& t)
            {
                if (t.kind == token::identifier || t == "::")
                {
                    if (hit.kind != token::identifier)
                        hit = t;
                    else
                        hit.value.second = t.value.second;
                    hit.kind = token::identifier;
                    return true;
                }
                return false;
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
                    state = (check_hit(t) ? scope : start);
                    return state != scope;
                case scope:
                    state = (check_hit(t) ? identifier : start);
                    return state != identifier;
                }
                return false;
            }
        };

    } // unnamed namespace

    deprecated_names const names[] =
    {
        // boost::xyz
        { "(\\bboost\\s*::\\s*move\\b)", "std::move" },
        { "(\\bboost\\s*::\\s*forward\\b)", "std::forward" },
        { "(\\bboost\\s*::\\s*noncopyable\\b)", "HPX_NON_COPYABLE" },
        { "(\\bboost\\s*::\\s*result_of\\b)", "std::result_of" },
        { "(\\bboost\\s*::\\s*decay\\b)", "std::decay" },
        { "(\\bboost\\s*::\\s*enable_if\\b)", "std::enable_if" },
        { "(\\bboost\\s*::\\s*disable_if\\b)", "std::enable_if" },
        { "(\\bboost\\s*::\\s*enable_if_c\\b)", "std::enable_if" },
        { "(\\bboost\\s*::\\s*disable_if_c\\b)", "std::enable_if" },
        { "(\\bboost\\s*::\\s*lazy_enable_if\\b)", "hpx::util::lazy_enable_if" },
        { "(\\bboost\\s*::\\s*lazy_disable_if\\b)", "hpx::util::lazy_enable_if" },
        { "(\\bboost\\s*::\\s*lazy_enable_if_c\\b)", "hpx::util::lazy_enable_if" },
        { "(\\bboost\\s*::\\s*lazy_disable_if_c\\b)", "hpx::util::lazy_enable_if" },
        { "(\\bboost\\s*::\\s*mpl\\b)", "no specific replacement" },
        { "(\\bboost\\s*::\\s*(is_[^\\s]*?\\b))", "std::\\2" },
        { "(\\bboost\\s*::\\s*(add_[^\\s]*?\\b))", "std::\\2" },
        { "(\\bboost\\s*::\\s*(remove_[^\\s]*?\\b))", "std::\\2" },
        { "(\\bboost\\s*::\\s*(((false)|(true))_type\\b))", "std::\\2" },
        { "(\\bboost\\s*::\\s*lock_guard\\b)", "std::lock_guard" },
        { "(\\bboost\\s*::\\s*unordered_map\\b)", "std::unordered_map" },
        { "(\\bboost\\s*::\\s*unordered_multimap\\b)", "std::unordered_multimap" },
        { "(\\bboost\\s*::\\s*unordered_set\\b)", "std::unordered_set" },
        { "(\\bboost\\s*::\\s*unordered_multiset\\b)", "std::unordered_multiset" },
        { "(\\bboost\\s*::\\s*detail\\s*::\\s*atomic_count\\b)",
          "hpx::util::atomic_count" },
        { "(\\bboost\\s*::\\s*function\\b)", "hpx::util::function_nonser" },
        { "(\\bboost\\s*::\\s*shared_ptr\\b)", "std::shared_ptr" },
        { "(\\bboost\\s*::\\s*make_shared\\b)", "std::make_shared" },
        { "(\\bboost\\s*::\\s*enable_shared_from_this\\b)",
          "std::enable_shared_from_this" },
        { "(\\bboost\\s*::\\s*bind\\b)", "hpx::util::bind" },
        { "(\\bboost\\s*::\\s*unique_lock\\b)", "std::unique_lock" },
        { "(\\bboost\\s*::\\s*chrono\\b)", "std::chrono" },
        { "(\\bboost\\s*::\\s*reference_wrapper\\b)", "std::reference_wrapper" },
        { "(\\bboost\\s*::\\s*(c?ref)\\b)", "std::\\2" },
        { "(\\bboost\\s*::\\s*(u?int[0-9]+_t)\\b)", "std::\\2" },
        { "(\\bboost\\s*::\\s*thread\\b)", "hpx::compat::thread" },
        { "(\\bboost\\s*::\\s*this_thread::\\s*get_id\\b)",
          "hpx::compat::this_thread::get_id" },
        { "(\\bboost\\s*::\\s*this_thread::\\s*yield\\b)",
          "hpx::compat::this_thread::yield" },
        { "(\\bboost\\s*::\\s*this_thread::\\s*sleep_until\\b)",
          "hpx::compat::this_thread::sleep_until" },
        { "(\\bboost\\s*::\\s*this_thread::\\s*sleep_for\\b)",
          "hpx::compat::this_thread::sleep_for" },
        { "(\\bboost\\s*::\\s*mutex\\b)", "hpx::compat::mutex" },
        { "(\\bboost\\s*::\\s*recursive_mutex\\b)", "hpx::compat::recursive_mutex" },
        { "(\\bboost\\s*::\\s*once_flag\\b)", "hpx::compat::once_flag" },
        { "(\\bboost\\s*::\\s*call_once\\b)", "hpx::compat::call_once" },
        { "(\\bboost\\s*::\\s*cv_status\\b)", "hpx::compat::cv_status" },
        { "(\\bboost\\s*::\\s*condition_variable\\b)",
          "hpx::compat::condition_variable" },
        { "(\\bboost\\s*::\\s*barrier\\b)", "hpx::compat::barrier" },
        { "(\\bboost\\s*::\\s*exception_ptr\\b)", "std::exception_ptr" },
        { "(\\bboost\\s*::\\s*copy_exception\\b)", "std::make_exception_ptr" },
        { "(\\bboost\\s*::\\s*current_exception\\b)", "std::current_exception" },
        { "(\\bboost\\s*::\\s*rethrow_exception\\b)", "std::rethrow_exception" },
        { "(\\bboost\\s*::\\s*enable_error_info\\b)", "hpx::throw_with_info" },
        { "(\\bboost\\s*::\\s*iterator_range\\b)", "hpx::util::iterator_range" },
        { "(\\bboost\\s*::\\s*make_iterator_range\\b)",
          "hpx::util::make_iterator_range" },
        { "(\\bboost\\s*::\\s*atomic_flag\\b)", "std::atomic_flag" },
        { "(\\bboost\\s*::\\s*atomic\\b)", "std::atomic" },
        { "(\\bboost\\s*::\\s*memory_order_((relaxed)|(acquire)|(release)|"
          "(acq_rel)|(seq_cst))\\b)", "std::memory_order_\\2" },
        { "(\\bboost\\s*::\\s*random\\s*::\\s*([^\\s]*)\\b)", "std::\\2" },
        { "(\\bboost\\s*::\\s*format\\b)", "hpx::util::format[_to]" },
        /////////////////////////////////////////////////////////////////////////
        { "((\\bhpx::\\b)?\\btraits\\s*::\\bis_callable\\b)",
          "\\2traits::is_invocable[_r]" },
        { "((\\bhpx::\\b)?\\butil\\s*::\\bresult_of\\b)", "\\2util::invoke_result" },
        { "(\\bNULL\\b)", "nullptr" },
        // Boost preprocessor macros
        { "\\b(BOOST_PP_CAT)\\b", "HPX_PP_CAT" },
        { "\\b(BOOST_PP_STRINGIZE)\\b", "HPX_PP_STRINGIZE" },
        { "\\b(BOOST_STRINGIZE)\\b", "HPX_PP_STRINGIZE(HPX_PP_EXPAND())" },
        { "\\b(BOOST_ASSERT)\\b", "HPX_ASSERT" }
    };

    //  deprecated_name_check constructor  --------------------------------- //

    deprecated_name_check::deprecated_name_check()
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

    void deprecated_name_check::inspect(
        const string & library_name,
        const path & full_path,      // example: c:/foo/boost/filesystem/path.hpp
        const string & contents)     // contents of file to be inspected
    {
        std::string::size_type p = contents.find("hpxinspect:" "nodeprecatedname");
        if (p != string::npos)
        {
            // ignore this directive here (it is handled below) if it is followed
            // by a ':'
            if (p == contents.size() - 27 ||
                (contents.size() > p + 27 && contents[p + 27] != ':'))
            {
                return;
            }
        }

        std::vector<token_match> found_names;
        name_check check_name;
        tokenize_cpp(
            contents.data() + 0, contents.data() + contents.size(),
            [&](token_match const& t) {
                if (!check_name(t))
                    return;

                found_names.push_back(check_name.hit);
                check_name.hit = token_match();
            });
        if (check_name.hit.kind == token::identifier)
            found_names.push_back(check_name.hit);

        // for all given names, check whether any is used
        long errors = 0;
        for (deprecated_names const& d : names)
        {
            boost::regex re(d.name_regex);
            for (auto iter = found_names.begin();
                iter != found_names.end(); ++iter)
            {
                token_match const& name = *iter;

                boost::match_results<std::string::const_iterator> m;
                std::string found_name = name.value.str();
                if (boost::regex_match(found_name, m, re))
                {
                    found_names.erase(iter);

                    std::string tag("hpxinspect:" "nodeprecatedinclude:" + found_name);
                    if (contents.find(tag) != string::npos)
                        break;

                    char const* it = contents.data();
                    char const* match_it = name.value.first;

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
                        + " deprecated #name ("
                        + found_name
                        + ") on line "
                        + linelink(full_path, boost::lexical_cast<string>(line_number))
                        + " use " + m.format(d.use_instead) + " instead");

                    break;
                }
            }
        }
    }

}} // namespace boost::inspect

