#ifndef SC2PP_DETAIL_PARSERS_COMMON_HPP
#define SC2PP_DETAIL_PARSERS_COMMON_HPP

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <array>

#include <sc2pp/detail/types.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/types.hpp>

namespace sc2pp { namespace parsers {
template <typename Char, typename OutIter>
OutIter escape_char(Char c, OutIter out)
{
    auto uc = static_cast<unsigned char>(c);
    if (' ' <= uc and uc <= '~' and uc != '\\' and uc != '"') {
        *out++ = static_cast<char>(uc);
    }
    else {
        *out++ = '\\';
        switch(uc) {
        case '"':  *out++ = '"';  break;
        case '\\': *out++ = '\\'; break;
        case '\t': *out++ = 't';  break;
        case '\r': *out++ = 'r';  break;
        case '\n': *out++ = 'n';  break;
        default:
            char const* const hexdig = "0123456789ABCDEF";
            *out++ = 'x';
            *out++ = hexdig[uc >> 4];
            *out++ = hexdig[uc & 0xF];
        }
    }
    return out;
}
}}

namespace boost { namespace spirit { namespace traits {

template <typename Enable>
struct token_printer_debug<unsigned char, Enable>
{
    template <typename Out, typename Char>
    static void print(Out& o, Char c)
    {
        sc2pp::parsers::escape_char(c, std::ostream_iterator<char>(o));
    }
};

}}}

namespace sc2pp { namespace parsers {

typedef sc2pp::detail::hugenum_t hugenum_t;
typedef sc2pp::detail::byte_array byte_array;
typedef sc2pp::detail::byte_map byte_map;
typedef sc2pp::detail::object_type object_type;

template<class OutIter, class InIter>
OutIter write_escaped(InIter begin, const InIter end, OutIter out) {
    *out++ = '"';
    for (InIter i = begin; i != end; ++i) {
        escape_char(*i, out);
    }
    *out++ = '"';
    return out;
}

#define DEBUG_RULE(X) \
    X.name(#X); \
    boost::spirit::qi::debug(X)

#define HANDLE_ERROR(X)                                                         \
    X.name(#X);                                                                 \
    boost::spirit::qi::on_error<boost::spirit::qi::fail>(X, errorhandler<boost::spirit::unused_type, Iterator>()); \
    DEBUG_RULE(X)


template <typename Context, typename Iterator>
struct errorhandler
{
    void operator()(boost::fusion::vector<Iterator, Iterator,
                    Iterator, const boost::spirit::info&> params,
                    Context, boost::spirit::qi::error_handler_result) const
    {
        using boost::phoenix::at_c;
        std::stringstream ss;
        // const int MAX_CONTEXT = 20;
        ss << "Error! Expecting " << at_c<3>(params) << " here: ";
        auto begin = at_c<0>(params), end = at_c<2>(params);
        // if (end - begin > MAX_CONTEXT) begin = end - MAX_CONTEXT;
        write_escaped(begin, end, std::ostream_iterator<char>(ss));
        ss << " >>><<< ";
        begin = at_c<2>(params); end = at_c<1>(params);
        // if (end - begin > MAX_CONTEXT) end = begin + MAX_CONTEXT;
        write_escaped(begin, end, std::ostream_iterator<char>(ss));
        throw parse_error(ss.str());
    }
};

template <typename Context, typename Iterator>
struct errorhandler<Context, bitshift_iterator<Iterator> >
{
    typedef bitshift_iterator<Iterator> iterator_type;
    void operator()(boost::fusion::vector<iterator_type, iterator_type,
                    iterator_type, const boost::spirit::info&> params,
                    Context, boost::spirit::qi::error_handler_result) const
    {
        using boost::phoenix::at_c;
        const int MAX_CONTEXT = 20;
        std::stringstream ss;
        auto error_pos = at_c<2>(params);
        ss << "Error at offset " << std::hex << error_pos.offset()
                  << std::dec;
        if (error_pos.shift())
            ss << " (shifted by " << error_pos.shift() << ")";
        ss << ". Expecting " << at_c<3>(params) << "; got ";
        auto end = at_c<2>(params);
        for (int i = 0; i < MAX_CONTEXT && end != at_c<1>(params); ++i, ++end);
        write_escaped(error_pos, end, std::ostream_iterator<char>(ss));
        throw parse_error(ss.str());
    }
};

struct apply_sign_impl
{
    template <typename Arg>
    struct result
    {
        typedef long type;
    };
    template <typename Arg>
    long operator()(Arg num) const
    {
        return (num / 2) * (1 - 2 * (num & 1));
    }

};

struct apply_huge_sign_impl
{
    template <typename Arg>
    struct result
    {
        typedef hugenum_t type;
    };
    template <typename Arg>
    hugenum_t operator()(Arg num) const
    {
        return (num / 2) * (1 - 2 * (num % 2));
    }

};

struct vector_to_array_impl
{
    template <typename Arg>
    struct result
    {
        typedef std::array<unsigned char, 4> type;
    };
    template <typename Arg>
    typename result<Arg>::type operator()(Arg a) const
    {
        typename result<Arg>::type ret;
        for (size_t i = 0; i < ret.size() && i < a.size(); ++i)
        {
            ret[i] = a[i];
        }
        return ret;
    }
};

}}

#endif // PARSERS_COMMON_HPP
