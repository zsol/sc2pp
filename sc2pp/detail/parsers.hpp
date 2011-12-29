#ifndef SC2PP_DETAIL_PARSERS_H
#define SC2PP_DETAIL_PARSERS_H

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <array>

#include <sc2pp/detail/types.hpp>
#include <sc2pp/types.hpp>

namespace sc2pp { namespace parsers {
        typedef sc2pp::detail::hugenum_t hugenum_t;
        typedef sc2pp::detail::byte_array byte_array;
        typedef sc2pp::detail::byte_map byte_map;
        typedef sc2pp::detail::object_type object_type;

        template<class OutIter, class InIter>
        OutIter write_escaped(InIter begin, const InIter end, OutIter out) {
            *out++ = '"';
            for (InIter i = begin; i != end; ++i) {
                auto c = static_cast<unsigned char>(*i);
                if (' ' <= c and c <= '~' and c != '\\' and c != '"') {
                    *out++ = static_cast<char>(c);
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

#define HANDLE_ERROR(X)                                                 \
        X.name(#X);                                                     \
        boost::spirit::qi::on_error<boost::spirit::qi::fail>(X, errorhandler<boost::spirit::unused_type, Iterator>)

        template <typename Context, typename Iterator>
        void errorhandler(boost::fusion::vector<Iterator, Iterator, 
                                                Iterator, const boost::spirit::info&> params, 
                          Context, boost::spirit::qi::error_handler_result)
        {
            using boost::phoenix::at_c;
            // const int MAX_CONTEXT = 20;
            std::cerr << "Error! Expecting " << at_c<3>(params) << " here: ";
            auto begin = at_c<0>(params), end = at_c<2>(params);
            // if (end - begin > MAX_CONTEXT) begin = end - MAX_CONTEXT;
            write_escaped(begin, end, std::ostream_iterator<char>(std::cerr));
            std::cerr << " >>><<< ";
            begin = at_c<2>(params); end = at_c<1>(params);
            // if (end - begin > MAX_CONTEXT) end = begin + MAX_CONTEXT;
            write_escaped(begin, end, std::ostream_iterator<char>(std::cerr));
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
                    byte_string | single_byte_integer | four_byte_integer | variable_length_integer
                    | array | map;


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

        template <typename Iterator>
        struct timestamp_grammar_t
            : public boost::spirit::qi::grammar<Iterator, 
                                                boost::spirit::qi::locals<int, num_t>,
                                                num_t()>
        {
            timestamp_grammar_t() : timestamp_grammar_t::base_type(timestamp, "Timestamp")
            {
                using boost::spirit::byte_;
                using boost::spirit::repeat;
                using boost::spirit::eps;
                using boost::spirit::_val;
                using boost::spirit::omit;
                using boost::spirit::_1;
                using boost::spirit::_a;
                using boost::spirit::_b;
                using boost::phoenix::static_cast_;

                
                timestamp =
                    &byte_[_b = _1 & 0x3] >> byte_[_a = static_cast_<int>(_1) >> 2]
                                          >> repeat(_b)[eps[_a <<= 8] >> byte_[_a += _1]]
                                          >> eps[_val = _a];

                HANDLE_ERROR(timestamp);
            }

            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<int, num_t>,
                                    num_t()> timestamp;

        };
        

        template <typename Iterator>
        struct message_grammar_t 
            : public boost::spirit::qi::grammar<Iterator, 
                                                boost::spirit::locals<num_t, int>, 
                                                message_event_ptr()>
        {
            message_grammar_t() : message_grammar_t::base_type(message_event, "Message")
            {
                using boost::spirit::byte_;
                using boost::spirit::little_dword;
                using boost::spirit::repeat;
                using boost::spirit::inf;
                using boost::spirit::eps;
                using boost::spirit::_val;
                using boost::spirit::omit;
                using boost::spirit::_1;
                using boost::spirit::_a;
                using boost::spirit::_b;
                using boost::spirit::_r1;
                using boost::spirit::_r2;
                using boost::spirit::_pass;
                using boost::spirit::as_string;
                using boost::phoenix::static_cast_;
                using boost::phoenix::if_;
                using boost::phoenix::bind;
                            
                
                ping_event =
                    byte_(0x83) > little_dword[_a = _1] > little_dword[_b = _1] 
                    >> eps[_val = bind(ping_event_t::make, _r1, _r2, _a, _b)];
            
                message = 
                    &byte_[if_((_1 & 0x80) != 0)[_pass = false]]
                    > byte_[_b = static_cast_<message_t::target_t>(_1 & 0x3), _a = (_1 & 0x18) << 3] >> byte_[_a += _1]
                                                                                               >> as_string[repeat(_a)[byte_]][_val = bind(message_t::make, _r1, _r2, _b, _1)];

                unknown_message =
                    byte_(0x80) > repeat(4)[byte_][_val = boost::phoenix::bind(unknown_message_t::make, _r1, _r2, vector_to_array(_1))];

                message_event %= omit[timestamp[_a = _1] >> byte_[_b = _1 & 0xF]]
                    >> (ping_event(_a, _b) | message(_a, _b) | unknown_message(_a, _b));


                HANDLE_ERROR(message_event);
                HANDLE_ERROR(ping_event);
                HANDLE_ERROR(message);
                HANDLE_ERROR(unknown_message);
                // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);
            }

            timestamp_grammar_t<Iterator> timestamp;
            
            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<num_t, int>,
                                    message_event_ptr()> message_event;
            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<int, int>,
                                    message_event_ptr(num_t, int)> ping_event;
            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<int, message_t::target_t>,
                                    message_event_ptr(num_t, int)> message;
            boost::spirit::qi::rule<Iterator,
                                    message_event_ptr(num_t, int)> unknown_message;

            boost::phoenix::function<vector_to_array_impl> vector_to_array;
        };


        template <typename Iterator>
        struct game_event_grammar_t
            : public boost::spirit::qi::grammar<Iterator, 
                                                boost::spirit::qi::locals<num_t, int>,
                                                game_event_ptr()>
        {
            game_event_grammar_t() : game_event_grammar_t::base_type(game_event, "Game Event")
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
                using boost::spirit::_r1;
                using boost::spirit::_r2;
                using boost::spirit::_pass;
                using boost::spirit::as_string;
                using boost::phoenix::static_cast_;
                using boost::phoenix::if_;
                using boost::phoenix::bind;

                
                game_event %=
                        omit[timestamp[_a = _1] >> &byte_[_b = (static_cast_<int>(_1) >> 3) & 0x1f]] >
                        (
                            initial_event(_a, _b) |
                            action_event(_a, _b) |
                            unknown_event(_a, _b)
                        );

                unknown_event = 
                    byte_[_a = _1 & 0x7] 
                    >> byte_[_val = bind(unknown_event_t::make, _r1, _r2, _a, _1)];

                initial_event %=
                        omit[byte_[if_((_1 & 0x7) != 0)[_pass = false]]] >
                        (
                            player_joined_event(_r1, _r2) |
                            game_started_event(_r1, _r2)
                        );

                player_joined_event = 
                    (byte_(0xB) | byte_(0xC) | byte_(0x2C))
                    >> eps[_val = bind(player_joined_event_t::make, _r1, _r2)];

                game_started_event =
                    byte_(0x5)
                    >> eps[_val = bind(game_started_event_t::make, _r1, _r2)];

                action_event %=
                        omit[byte_[if_((_1 & 0x7) != 1)[_pass = false]]] >
                        (
                            player_left_event(_r1, _r2) |
                            resource_transfer_event(_r1, _r2)
                        );

                player_left_event =
                    byte_(0x9)
                    >> eps[_val = bind(player_left_event_t::make, _r1, _r2)];
                
                resource_transfer_event =
                    &byte_[if_((_1 & 0xf) != 0xf)[_pass = false]] >> byte_[_a = static_cast_<int>(_1) >> 4] >> 
                    byte_(0x84) >>
                    repeat(4)[resource][_val = bind(resource_transfer_event_t::make, _r1, _r2, _a, _1)];
                                                             
                resource = 
                    big_dword[_val = (static_cast_<num_t>(_1) >> 8) * (static_cast_<num_t>(_1) & 0xf0) + (static_cast_<num_t>(_1) & 0x0f)];


                // camera_event %=
                //     omit[byte_[if_((_1 & 0x7) != 3)[_pass = false]]];


                HANDLE_ERROR(game_event);
                HANDLE_ERROR(unknown_event);
                HANDLE_ERROR(player_joined_event);
                HANDLE_ERROR(game_started_event);
                HANDLE_ERROR(initial_event);
                HANDLE_ERROR(action_event);
                HANDLE_ERROR(player_left_event);
                HANDLE_ERROR(resource_transfer_event);
                // boost::spirit::qi::on_error<boost::spirit::qi::fail>(*this, errorhandler<boost::spirit::unused_type, Iterator>);

            }

            timestamp_grammar_t<Iterator> timestamp;

            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<num_t, int>,
                                    game_event_ptr()> game_event;
            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<int>,
                                    game_event_ptr(num_t, int)> unknown_event;
            boost::spirit::qi::rule<Iterator,
                                    game_event_ptr(num_t, int)> initial_event;
            boost::spirit::qi::rule<Iterator,
                                    game_event_ptr(num_t, int)> player_joined_event;
            boost::spirit::qi::rule<Iterator,
                                    game_event_ptr(num_t, int)> game_started_event;
            boost::spirit::qi::rule<Iterator,
                                    game_event_ptr(num_t, int)> action_event;
            boost::spirit::qi::rule<Iterator,
                                    game_event_ptr(num_t, int)> player_left_event;
            boost::spirit::qi::rule<Iterator,
                                    boost::spirit::qi::locals<int, std::vector<num_t> >,
                                    game_event_ptr(num_t, int)> resource_transfer_event;
            boost::spirit::qi::rule<Iterator,
                                    num_t() > resource;
            
        };

    } 
}

#endif

// Local Variables:
// mode:c++
// c-file-style: "stroustrup"
// end:

