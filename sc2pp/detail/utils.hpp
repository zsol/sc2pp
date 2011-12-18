#ifndef SC2PP_UTILS_HPP
#define SC2PP_UTILS_HPP

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

  bitshift_iterator() : _shift(0) {}

  explicit bitshift_iterator(const iterator_type& it) : _shift(0), _iter(it) {}
  
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
       ++_iter;
    else
      advance();

    return *this;
  }

  bitshift_iterator operator++(int)
  {
    bitshift_iterator ret = *this;
    if (_shift == 0)
      _iter++;
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
      ++_iter;
      _shift = 0; // we are realigned now, there are 's' remaining bits to shift:
      // s < 8 holds from now on
    }
    const unsigned char mask = LO_MASK[s + _shift] ^ LO_MASK[_shift];
    ret |= _buf & mask >> 8 - s;
    _buf &= 0xff ^ mask;
    _buf |= *_iter & mask;
    _shift -= s;
    _shift %= 8;

    return ret;
  }

private:
  size_t _shift;
  iterator_type _iter;
  value_type _buf; // contains the next 8 bits if _shift != 0

  // static constexpr unsigned char LO_MASK[]; 
  // static constexpr unsigned char HI_MASK[]; 

  void advance()
  {
    _buf = *_iter & HI_MASK[8 - _shift];
    ++_iter;
    _buf |= *_iter & LO_MASK[_shift];
  }
};
 

#endif
