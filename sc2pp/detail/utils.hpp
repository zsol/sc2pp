#ifndef SC2PP_UTILS_HPP
#define SC2PP_UTILS_HPP

namespace sc2pp {

constexpr unsigned char LO_MASK[] = { 0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff };
constexpr unsigned char HI_MASK[] = { 0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff };

template <typename Iter>
class bitshift_iterator : public std::iterator<typename std::iterator_traits<Iter>::iterator_category,
                                               typename std::iterator_traits<Iter>::value_type,
                                               typename std::iterator_traits<Iter>::difference_type,
                                               typename std::iterator_traits<Iter>::pointer,
                                               typename std::iterator_traits<Iter>::reference>
{
  typedef std::iterator_traits<Iter> _traits_type;
public:
  typedef Iter iterator_type;
  typedef typename _traits_type::pointer pointer;
  typedef typename _traits_type::reference reference;
  typedef typename _traits_type::value_type value_type;
  typedef typename _traits_type::difference_type difference_type;

  bitshift_iterator() : _offset(0), _shift(0) {}

  explicit bitshift_iterator(const iterator_type& it)
      : _offset(0), _shift(0), _iter(it) {}
  
  reference operator*() const
  {
    if (_shift == 0)
      return *_iter;

    return _buf;
  }

  pointer operator->() const
  {
    return &(operator*());
  }

  bitshift_iterator operator++()
  {
    if (_shift == 0)
       ++_iter, ++_offset;
    else
      advance();

    return *this;
  }

  bitshift_iterator operator++(int)
  {
    bitshift_iterator ret = *this;
    if (_shift == 0)
      _iter++, ++_offset;
    else
      advance();

    return ret;
  }

  bool operator==(bitshift_iterator const & other) const
  {
    return _iter == other._iter && _shift == other._shift;
  }

  bool operator!=(bitshift_iterator const & other) const
  { return not operator==(other); }
  
  difference_type operator-(bitshift_iterator const & other) const
  {
    return _iter - other._iter;
  }

  /*
    _shift == 1: |XXXXXXX-|-------X|
    _shift == 7: |X-------|-XXXXXXX|
   */

  value_type shift(size_t s)
  {
    assert(s <= 8);
    value_type ret = 0;
    if (s + _shift > 8)
    {
      // save the part contained in the first byte
      ret = _buf & HI_MASK[8 - _shift];
      s -= 8 - _shift;
      // make sure _buf == second byte; then read a third one
      _buf &= LO_MASK[_shift];
      _buf |= *_iter & HI_MASK[_shift];
      ++_iter, ++_offset;
      _shift = 0; // we are realigned now, there are 's' remaining bits to shift:
      // s < 8 holds from now on
    }
    const unsigned char mask = LO_MASK[s + _shift] ^ LO_MASK[_shift];
    ret |= _buf & mask >> (8 - s);
    _buf &= 0xff ^ mask;
    _buf |= *_iter & mask;
    _shift -= s;
    _shift %= 8;

    return ret;
  }

  size_t shift() const { return _shift; }
  size_t offset() const { return _offset; }

private:
  size_t _offset;
  size_t _shift;
  iterator_type _iter;
  value_type _buf; // contains the next 8 bits if _shift != 0

  // static constexpr unsigned char LO_MASK[]; 
  // static constexpr unsigned char HI_MASK[]; 

  void advance()
  {
    _buf = *_iter & HI_MASK[8 - _shift];
    ++_iter, ++_offset;
    _buf |= *_iter & LO_MASK[_shift];
  }

};

}

#include <boost/spirit/include/qi.hpp>
#include <boost/fusion/container/vector.hpp>
#include <boost/fusion/include/vector.hpp>

namespace sc2pp {
BOOST_SPIRIT_TERMINAL_EX(bits);

template <typename Size>
struct bits_parser : public boost::spirit::qi::primitive_parser<bits_parser<Size> >
{
    template <typename Context, typename Iterator>
    struct attribute
    {
        typedef typename Iterator::value_type type;
    };

    bits_parser(Size n) : n(n) {}

    template <typename Iterator, typename Context, typename Skipper,
              typename Attribute>
    bool parse(Iterator & first, Iterator const & last, Context &,
               Skipper const & skipper, Attribute & attr) const
    {
        boost::spirit::qi::skip_over(first, last, skipper);

        if (first == last) return false;
        boost::spirit::traits::assign_to(first.shift(n), attr);
        return true;
    }

    template <typename Context>
    boost::spirit::info what(Context &) const
    {
        return boost::spirit::info("bits");
    }

private:
    Size n;
};

template <typename Size, typename Expected>
struct bits_lit_parser : boost::spirit::qi::primitive_parser<bits_lit_parser<Size, Expected> >
{
    template <typename Context, typename Iterator>
    struct attribute
    {
        typedef boost::spirit::unused_type type;
    };

    bits_lit_parser(Size n, Expected e) : n(n), e(e) {}

    template <typename Iterator, typename Context, typename Skipper,
              typename Attribute>
    bool parse(Iterator & first, Iterator const & last, Context &,
               Skipper const & skipper, Attribute & attr) const
    {
        boost::spirit::qi::skip_over(first, last, skipper);
        auto bits = static_cast<typename Iterator::value_type>(e);

        Iterator it = first;
        if (it == last || it.shift(n) != bits)
            return false;

        first = it;
        boost::spirit::traits::assign_to(bits, attr);
        return true;
    }

    template <typename Context>
    boost::spirit::info what(Context &) const
    {
        return boost::spirit::info("bits");
    }

private:
    Size n;
    Expected e;
};

} // namespace sc2pp

namespace boost { namespace spirit {
template <typename A0>
struct use_terminal<qi::domain,
        terminal_ex<sc2pp::tag::bits, fusion::vector1<A0> > >
        : mpl::true_ {};

template <typename A0, typename A1>
struct use_terminal<qi::domain,
        terminal_ex<sc2pp::tag::bits, fusion::vector2<A0, A1> > >
        : mpl::true_ {};

template <>
struct use_lazy_terminal<qi::domain, sc2pp::tag::bits, 1> : mpl::true_ {};

template <>
struct use_lazy_terminal<qi::domain, sc2pp::tag::bits, 2> : mpl::true_ {};

namespace qi {
template <typename Modifiers, typename A0>
struct make_primitive<terminal_ex<sc2pp::tag::bits, fusion::vector1<A0> >,
        Modifiers>
{
    typedef sc2pp::bits_parser<A0> result_type;
    template <typename Terminal>
    result_type operator()(Terminal const & term, unused_type) const
    {
        return result_type(fusion::at_c<0>(term.args));
    }
};

template <typename Modifiers, typename A0, typename A1>
struct make_primitive<terminal_ex<sc2pp::tag::bits, fusion::vector2<A0, A1> >,
        Modifiers>
{
    typedef sc2pp::bits_lit_parser<A0, A1> result_type;
    template <typename Terminal>
    result_type operator()(Terminal const & term, unused_type) const
    {
        return result_type(fusion::at_c<0>(term.args),
                           fusion::at_c<1>(term.args));
    }
};
}
}}

#endif
