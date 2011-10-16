#include <boost/spirit/include/phoenix.hpp>

#include <sc2pp/parsers.h>

using namespace boost::spirit::qi;
using namespace boost::phoenix;

namespace
{
    template<class OutIter>
    OutIter write_escaped(const unsigned char* begin, const unsigned char* end, OutIter out) {
        *out++ = '"';
        for (const unsigned char* i = begin; i != end; ++i) {
            unsigned char c = *i;
            if (' ' <= c and c <= '~' and c != '\\' and c != '"') {
                *out++ = (char)c;
            }
            else {
                *out++ = '\\';
                switch(c) {
                case '"':  *out++ = '"';  break;
                case '\\': *out++ = '\\'; break;
                case '\t': *out++ = 't';  break;
                case '\r': *out++ = 'r';  break;
                case '\n': *out++ = 'n';  break;
                default:
                    char const* const hexdig = "0123456789ABCDEF";
                    *out++ = 'x';
                    *out++ = hexdig[c >> 4];
                    *out++ = hexdig[c & 0xF];
                }
            }
        }
        *out++ = '"';
        return out;
    }
    
    template <typename Context>
    void errorhandler(boost::fusion::vector<const unsigned char*, const unsigned char*, 
                                            const unsigned char*, const info&> params, 
                      Context, error_handler_result)
    {
        std::cerr << "Error! Expecting " << at_c<3>(params) << " here: ";
        write_escaped(at_c<0>(params), at_c<2>(params), std::ostream_iterator<char>(std::cerr));
        std::cerr << " >>><<< ";
        write_escaped(at_c<2>(params), at_c<1>(params), std::ostream_iterator<char>(std::cerr));
        std::cerr << std::endl;
    }
    
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
    
    function<apply_sign_impl> apply_sign;
}

namespace sc2pp { 
    namespace parsers {
        
        byte_string_rule_type byte_string;
        single_byte_integer_rule_type single_byte_integer;
        single_byte_integer_rule_type single_byte_integer_; // just an integer without 'type' byte
        four_byte_integer_rule_type four_byte_integer;
        variable_length_integer_rule_type variable_length_integer;
        array_rule_type array;
        map_rule_type map;
        object_rule_type object;

        Initializer::Initializer()
        {
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
                                                  >> byte_[_a += (_1 & 0x7f) << (_b * 7)] 
                                                  >> eps[_b += 1]]
                > byte_[_a += (_1 & 0xff) << (_b * 7)] 
                > eps[_val = apply_sign(_a)];

            array %=
                omit[byte_(0x4) > byte_(0x1) > byte_(0x0) > single_byte_integer_[_a = _1]]
                >> repeat(_a)[object];

            map =
                omit[byte_(0x5) > single_byte_integer_[_a = _1]]
                >> repeat(_a)[single_byte_integer_ > object];

            object %=  
                byte_string | single_byte_integer | four_byte_integer | variable_length_integer
                | array | map;
          
#define HANDLE_ERROR(X)                                                 \
            X.name(#X);                                                 \
            on_error<fail>(X, ::errorhandler<X##_rule_type::context_type>)

            HANDLE_ERROR(byte_string);
            HANDLE_ERROR(single_byte_integer);
            HANDLE_ERROR(four_byte_integer);
            HANDLE_ERROR(variable_length_integer);
            HANDLE_ERROR(array);
            HANDLE_ERROR(map);
            HANDLE_ERROR(object);
        }

        static Initializer __initializer;
    }
}

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

