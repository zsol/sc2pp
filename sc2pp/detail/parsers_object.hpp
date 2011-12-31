#ifndef SC2PP_DETAIL_PARSERS_OBJECT_HPP
#define SC2PP_DETAIL_PARSERS_OBJECT_HPP

#include <sc2pp/detail/parsers_common.hpp>

namespace sc2pp { namespace parsers {

template <typename Iterator>
struct object_grammar_t : public boost::spirit::qi::grammar<Iterator, object_type()>
{
    object_grammar_t() : object_grammar_t::base_type(object, "Blizzard Object")
    {
        using boost::spirit::byte_;
        using boost::spirit::big_dword;
        using boost::spirit::repeat;
        using boost::spirit::inf;
        using boost::spirit::eps;
        using boost::spirit::_val;
        using boost::spirit::omit;
        using boost::spirit::_1;
        using boost::spirit::_a;
        using boost::spirit::_b;
        using boost::spirit::_pass;
        using boost::phoenix::static_cast_;
        using boost::phoenix::if_;


        byte_string %=
                omit[byte_(0x2)] > omit[byte_[_a = _1/2]] > repeat(_a)[byte_];

        single_byte_integer_ =
                byte_[_val = apply_sign(_1)];

        single_byte_integer %=
                omit[byte_(0x6)] > single_byte_integer_;

        four_byte_integer =
                omit[byte_(0x7)] > big_dword[_val = apply_sign(_1)];

        variable_length_integer =
                omit[byte_(0x9)] > repeat(0, inf)[&byte_[if_((_1 & 0x80) == 0)[_pass = false]]
                >> byte_[_a += static_cast_<hugenum_t>(_1 & 0x7f) << (_b * 7)]
                                                                  >> eps[_b += 1]]
                > byte_[_a += static_cast_<hugenum_t>(_1 & 0xff) << (_b * 7)]
                                                                    > eps[_val = apply_huge_sign(_a)];

        array %=
                omit[byte_(0x4) > byte_(0x1) > byte_(0x0) > single_byte_integer_[_a = _1]]
                >> repeat(_a)[object];

        map =
                omit[byte_(0x5) > single_byte_integer_[_a = _1]]
                >> repeat(_a)[single_byte_integer_ > object];

        object %=
                byte_string | single_byte_integer | four_byte_integer |
                variable_length_integer | array | map;


        HANDLE_ERROR(byte_string);
        HANDLE_ERROR(single_byte_integer);
        HANDLE_ERROR(four_byte_integer);
        HANDLE_ERROR(variable_length_integer);
        HANDLE_ERROR(array);
        HANDLE_ERROR(map);
        HANDLE_ERROR(object);
    }

    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int>,
                            std::string() > byte_string;
    boost::spirit::qi::rule<Iterator,
                            int()> single_byte_integer;
    boost::spirit::qi::rule<Iterator,
                            int()> single_byte_integer_; // an integer without 'type' byte
    boost::spirit::qi::rule<Iterator,
                            num_t()> four_byte_integer;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<hugenum_t, int>,
                            hugenum_t()> variable_length_integer;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int>,
                            byte_array()> array;
    boost::spirit::qi::rule<Iterator,
                            boost::spirit::qi::locals<int, int>,
                            byte_map()> map;
    boost::spirit::qi::rule<Iterator,
                            object_type()> object;

    boost::phoenix::function<apply_sign_impl> apply_sign;
    boost::phoenix::function<apply_huge_sign_impl> apply_huge_sign;

};

}}

#endif // SC2PP_DETAIL_PARSERS_OBJECT_HPP
