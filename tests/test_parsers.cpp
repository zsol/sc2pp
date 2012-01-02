#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <iterator>

#include <boost/spirit/include/phoenix.hpp>

#include <sc2pp/detail/parsers_object.hpp>

using namespace sc2pp::parsers;
using namespace sc2pp;
using namespace boost::phoenix;
using namespace boost::spirit::qi;

template <class Result, class Container, class Rule>
bool test(Container const & arr, Rule const & rule, Result const & result)
{
  Result tmp;
  const uint8_t* beg, *end;
  if (not parse(beg = arr.begin(), end = arr.end(), rule, tmp)) {
    std::cerr << "Failed to parse..." << std::endl;
    return false;
  }
  if (not (tmp == result))
    std::cerr << "Expected: " << result << ", got: " << tmp << "." << std::endl;
  return tmp == result;
}

int main(int argc, char** argv)
{
  std::array<const uint8_t, 7> bytestring = {0x02, 0x0a, 0x50, 0x69, 0x6c, 0x6c, 0x65}; 
  std::array<const uint8_t, 2> singlebyteinteger = {0x06, 0x4c}; 
  std::array<const uint8_t, 5> fourbyteinteger = {0x07, 0x00, 0x00, 0x53, 0x32};
  std::array<const uint8_t, 4> vlinteger = {0x09, 0xf2, 0xbf, 0x50};
  std::array<const uint8_t, 11> hugevlinteger = {0x09, 0x80, 0x80, 0x80, 0x80, 0x80, 
                                                 0x80, 0x80, 0x80, 0x80, 0x02};
  std::array<const uint8_t, 17> bytearray = {0x04, 0x01, 0x00, 0x08, 0x02 ,0x0a, 0x50, 
                                             0x69, 0x6c, 0x6c, 0x65, 0x06, 0x2a, 0x06, 
                                             0xa6, 0x06, 0x8d};
  std::array<const uint8_t, 19> bytemap = {0x05, 0x08, 0x00, 0x09, 0x04, 0x02, 0x07, 
                                           0x00, 0x00, 0x53, 0x32, 0x04, 0x09, 0x02,
                                           0x08, 0x09, 0xf2, 0xbf, 0x50};

  object_grammar_t<const uint8_t*> g;
  object_type obj;
  assert(test(bytestring, g.byte_string, obj = std::string("Pille")));
  assert(test(singlebyteinteger, g.single_byte_integer, obj = 38));
  assert(test(fourbyteinteger, g.four_byte_integer, obj = 10649));
  assert(test(vlinteger, g.variable_length_integer, obj = mpz_class(659449)));
  assert(test(hugevlinteger, g.variable_length_integer, obj = (mpz_class(1) << 63)));
  assert(test(bytearray, g.array, obj = byte_array(std::vector<object_type>({"Pille", 0x2a >> 1, 0xa6 >> 1, 0x46 * -1}))));
  byte_map bytemap_result;
  bytemap_result.map[0] = mpz_class(2);
  bytemap_result.map[1] = 10649;
  bytemap_result.map[2] = mpz_class(1);
  bytemap_result.map[4] = mpz_class(659449);
  assert(test(bytemap, g.map, obj = bytemap_result));

  return 0;
}
