#ifndef SC2PP_DETAIL_PARSERS_COMMON_HPP
#define SC2PP_DETAIL_PARSERS_COMMON_HPP

#include <boost/spirit/include/version.hpp>

#if SPIRIT_VERSION < 0x2050
#define USE_SPIRIT_PARSER(PARSER) using boost::spirit::qi::PARSER
#define USE_SPIRIT_PARSER_(PARSER) using boost::spirit::qi::PARSER
#else
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
#define USE_SPIRIT_PARSER(PARSER) boost::spirit::qi::PARSER##_type PARSER
#define USE_SPIRIT_PARSER_(PARSER) boost::spirit::qi::PARSER##type PARSER
#endif

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <array>

#include <sc2pp/detail/types.hpp>
#include <sc2pp/detail/utils.hpp>
#include <sc2pp/types.hpp>

#define DECLARE_DEFAULT_GRAMMAR(GRAMMAR) \
    struct default_##GRAMMAR##_t : GRAMMAR##_t<default_iterator> { default_##GRAMMAR##_t(); }; \
    template <typename Iterator> struct GRAMMAR##_selector { typedef GRAMMAR##_t<Iterator> type; }; \
    template <> struct GRAMMAR##_selector<default_iterator> { typedef default_##GRAMMAR##_t type; }

#define IMPLEMENT_DEFAULT_GRAMMAR(GRAMMAR) \
    default_##GRAMMAR##_t::default_##GRAMMAR##_t() : GRAMMAR##_t() {}

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

typedef bitshift_iterator<const unsigned char*> default_iterator;
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

namespace p = boost::phoenix;

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

template <typename Iterator>
struct big_tbyte_grammar_t
        : boost::spirit::qi::grammar<Iterator, boost::spirit::locals<int>, int()>
{
    big_tbyte_grammar_t() : big_tbyte_grammar_t::base_type(big_tbyte, "Big Endian 24bit binary")
    {
        USE_SPIRIT_PARSER(big_word);
        USE_SPIRIT_PARSER(_a);
        USE_SPIRIT_PARSER(_1);
        USE_SPIRIT_PARSER(_val);
        USE_SPIRIT_PARSER_(byte_);
        using boost::phoenix::static_cast_;

        big_tbyte = big_word[_a = static_cast_<int>(_1) << 8] > byte_[_val = _a | _1];
    }

    boost::spirit::qi::rule<Iterator, boost::spirit::locals<int>, int()> big_tbyte;
};

DECLARE_DEFAULT_GRAMMAR(big_tbyte_grammar);

template <typename Iterator>
struct byteint_grammar_t
        : boost::spirit::qi::grammar<Iterator, int()>
{
    byteint_grammar_t() : byteint_grammar_t::base_type(byteint, "Single-byte int")
    {
        USE_SPIRIT_PARSER(_val);
        USE_SPIRIT_PARSER_(byte_);
        USE_SPIRIT_PARSER(_1);
        using boost::phoenix::static_cast_;

        byteint = byte_[_val = static_cast_<int>(_1)];
    }

    boost::spirit::qi::rule<Iterator, int()> byteint;
};

DECLARE_DEFAULT_GRAMMAR(byteint_grammar);

template <typename Iterator>
struct coordinate_grammar_t
        : boost::spirit::qi::grammar<Iterator, boost::spirit::qi::locals<int, int>, float()>
{
    coordinate_grammar_t() : coordinate_grammar_t::base_type(coordinate, "Coordinate")
    {
        USE_SPIRIT_PARSER_(byte_);
        USE_SPIRIT_PARSER(eps);
        USE_SPIRIT_PARSER(_a);
        USE_SPIRIT_PARSER(_b);
        USE_SPIRIT_PARSER(_1);
        USE_SPIRIT_PARSER(_val);
        bits_type bits;

        coordinate =
                byteint[_a = _1] > byteint[_b = _1 << 4] > bits(4)[_b = _b | _1] >
                eps[_val = make_coordinate(_a, _b)];
    }

    struct make_coordinate_impl
    {
        template <typename A1, typename A2>
        struct result
        {
            typedef float type;
        };

        template <typename A1, typename A2>
        typename result<A1, A2>::type operator()(A1 a1, A2 a2) const
        {
            typedef typename result<A1, A2>::type ret_t;
            ret_t ret = a1;

            for (int i = 11; i >= 0; --i)
                ret += static_cast<ret_t>((a2 >> i) & 1) / static_cast<ret_t>(1 << (12 - i));

            return ret;
        }
    };

    boost::phoenix::function<make_coordinate_impl> make_coordinate;
    byteint_grammar_t<Iterator> byteint;
    boost::spirit::qi::rule<Iterator,
            boost::spirit::qi::locals<int, int>,
            float()> coordinate;
};

DECLARE_DEFAULT_GRAMMAR(coordinate_grammar);

}}

#endif // PARSERS_COMMON_HPP
