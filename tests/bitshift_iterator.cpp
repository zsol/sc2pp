#include <array>
#include <vector>
#include <sc2pp/detail/utils.hpp>
#define BOOST_TEST_MODULE bitshift test
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(shift_by_one_test)
{
    const std::array<unsigned char, 4> buf = { 1, 2, 3, 4 };
    auto it = std::begin(buf);
    auto bit = sc2pp::make_bitshift_iterator(it);

    BOOST_CHECK_EQUAL(*bit, *it);
    BOOST_CHECK_EQUAL(bit.shift(1), 1);
    BOOST_CHECK_EQUAL(bit.shift(), 1);
    BOOST_CHECK_EQUAL(*bit, 0);
    BOOST_CHECK_EQUAL(bit.shift(1), 0);
    BOOST_CHECK_EQUAL(bit.shift(), 2);
    BOOST_CHECK_EQUAL(*bit, 2);
    for (size_t i = 0; i < 6; ++i)
    {
        BOOST_CHECK_EQUAL(bit.shift(1), 0);
        BOOST_CHECK_EQUAL(bit.shift(), (i+3)%8);
        BOOST_CHECK_EQUAL(*bit, 2);
    }
    BOOST_CHECK_EQUAL(bit.shift(1), 0);
    BOOST_CHECK_EQUAL(bit.shift(), 1);
    BOOST_CHECK_EQUAL(*bit, 3);
    BOOST_CHECK_EQUAL(bit.shift(1), 1);
    BOOST_CHECK_EQUAL(bit.shift(), 2);
    BOOST_CHECK_EQUAL(*bit, 3);
    for (size_t i = 0; i < 6; ++i)
    {
        BOOST_CHECK_EQUAL(bit.shift(1), 0);
        BOOST_CHECK_EQUAL(bit.shift(), (i+3)%8);
        BOOST_CHECK_EQUAL(*bit, 3);
    }
    BOOST_CHECK_EQUAL(bit.shift(1), 1);
    BOOST_CHECK_EQUAL(bit.shift(), 1);
    BOOST_CHECK_EQUAL(*bit, 2);
    BOOST_CHECK_EQUAL(bit.shift(1), 1);
    BOOST_CHECK_EQUAL(bit.shift(), 2);
    BOOST_CHECK_EQUAL(*bit, 0);
    for (size_t i = 0; i < 6; ++i)
    {
        BOOST_CHECK_EQUAL(bit.shift(1), 0);
        BOOST_CHECK_EQUAL(bit.shift(), (i+3)%8);
        BOOST_CHECK_EQUAL(*bit, 4);
    }
}

BOOST_AUTO_TEST_CASE(no_shift_test)
{
    const std::array<unsigned char, 4> buf = { 1, 2, 3, 4 };
    auto it = std::begin(buf);
    auto bit = sc2pp::make_bitshift_iterator(it);

    for(size_t i = 0; i < buf.size(); ++i, ++bit, ++it)
    {
        BOOST_CHECK_EQUAL(*bit, *it);
        BOOST_CHECK_EQUAL(bit.shift(), 0);
    }
}

struct Fixture
{
    Fixture ()
        : v({0xf0, 0x0f, 0xff}),
          it(std::begin(v)),
          bit(sc2pp::make_bitshift_iterator(it))
    {}

    typedef std::vector<unsigned char> v_t;
    v_t v;
    v_t::const_iterator it;
    sc2pp::bitshift_iterator<v_t::const_iterator> bit;
};

BOOST_FIXTURE_TEST_CASE(read_less_than_8_bits, Fixture)
{
    BOOST_CHECK_EQUAL(bit.shift(4), 0);
    BOOST_CHECK_EQUAL(bit.shift(4), 0xf);
}

BOOST_FIXTURE_TEST_CASE(read_by_two_bits, Fixture)
{
    v_t shift_results = {0x0, 0x0, 0x3, 0x3, 0x3, 0x3, 0x0, 0x0, 0x3, 0x3, 0x3, 0x3};
    v_t deref_results = {0xf0, 0xf3, 0xff, 0xcf, 0x0f, 0x0f, 0x0f, 0x3f, 0xff, 0xfc, 0xf0, 0xc0};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(2);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*2)%8);
        BOOST_REQUIRE_LT(shifted, 4);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_three_bits, Fixture)
{
    v_t shift_results = {0x0, 0x6, 0x7, 0x7, 0x0, 0x6, 0x7, 0x7};
    v_t deref_results = {0xf0, 0xf7, 0xcf, 0x0f, 0x0f, 0x7f}; // undefined, undefined};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        if (i < deref_results.size())
            BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(3);
        BOOST_REQUIRE_LT(shifted, 8);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*3)%8);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_four_bits, Fixture)
{
    v_t shift_results = {0x0, 0xf, 0xf, 0x0, 0xf, 0xf};
    v_t deref_results = {0xf0, 0xff, 0x0f, 0x0f, 0xff};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        if (i < deref_results.size())
            BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(4);
        BOOST_REQUIRE_LT(shifted, 16);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*4)%8);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_five_bits, Fixture)
{
    v_t shift_results = {0x10, 0x1f, 0x03, 0x1e};
    v_t deref_results = {0xf0, 0xef, 0x0f, 0x7f};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        if (i < deref_results.size())
            BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(5);
        BOOST_REQUIRE_LT(shifted, 32);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*5)%8);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_six_bits, Fixture)
{
    v_t shift_results = {0x30, 0x3f, 0x30, 0x3f};
    v_t deref_results = {0xf0, 0xcf, 0x0f};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        if (i < deref_results.size())
            BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(6);
        BOOST_REQUIRE_LT(shifted, 64);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*6)%8);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_seven_bits, Fixture)
{
    v_t shift_results = {0x70, 0x1f, 0x7c};
    v_t deref_results = {0xf0, 0x8f, 0x3f};
    for (size_t i = 0; i < shift_results.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        if (i < deref_results.size())
            BOOST_CHECK_EQUAL(*bit, deref_results[i]);
        unsigned char shifted = bit.shift(7);
        BOOST_REQUIRE_LT(shifted, 128);
        BOOST_REQUIRE_EQUAL(bit.shift(), ((i+1)*7)%8);
        BOOST_CHECK_EQUAL(shifted, shift_results[i]);
    }
}

BOOST_FIXTURE_TEST_CASE(read_by_eight_bits, Fixture)
{
    for (size_t i = 0; i < v.size(); ++i)
    {
        BOOST_TEST_CHECKPOINT("Shifted " << i << " times so far");
        BOOST_CHECK_EQUAL(*bit, *it);
        unsigned char shifted = bit.shift(8);
        BOOST_REQUIRE_LT(shifted, 256);
        BOOST_CHECK_EQUAL(shifted, *it++);
    }
}
BOOST_TEST_DONT_PRINT_LOG_VALUE(sc2pp::bitshift_iterator<std::vector<unsigned char>::const_iterator>);

BOOST_AUTO_TEST_CASE(mixed_reads)
{
    std::vector<unsigned char> v = { 0x0, 0x8f, 0xc7, 0xf3 };
    std::vector<unsigned char>::const_iterator it = std::begin(v), end = std::end(v);
    auto bit = sc2pp::make_bitshift_iterator(it);
    BOOST_REQUIRE_EQUAL(*bit, *it);
    BOOST_CHECK_EQUAL(bit.shift(6), 0x0);
    BOOST_REQUIRE_EQUAL(bit.shift(), 6);
    BOOST_CHECK_EQUAL(*bit, 0xf);
    BOOST_CHECK_EQUAL(bit.shift(6), 0x3c);
    BOOST_REQUIRE_EQUAL(bit.shift(), 4);
    BOOST_CHECK_EQUAL(*bit, 0x87);
    BOOST_CHECK_EQUAL(bit.shift(3), 0x0);
    BOOST_REQUIRE_EQUAL(bit.shift(), 7);
    BOOST_CHECK_EQUAL(*bit, 0xc7);
    BOOST_CHECK_EQUAL(bit.shift(4), 0xf);
    BOOST_REQUIRE_EQUAL(bit.shift(), 3);
    BOOST_CHECK_EQUAL(*bit, 0xc3);
    BOOST_CHECK_EQUAL(bit.shift(3), 0x0);
    BOOST_REQUIRE_EQUAL(bit.shift(), 6);
    BOOST_CHECK_EQUAL(*bit, 0xf3);
    BOOST_CHECK_EQUAL(bit.shift(4), 0xf);
    BOOST_REQUIRE_EQUAL(bit.shift(), 2);
    BOOST_CHECK_EQUAL(bit.shift(1), 0x0);
    BOOST_REQUIRE_EQUAL(bit.shift(), 3);
    BOOST_CHECK_EQUAL(bit.shift(5), 0x1e);
    BOOST_REQUIRE_EQUAL(bit.shift(), 0);
    BOOST_REQUIRE_EQUAL(bit, sc2pp::make_bitshift_iterator(end));
}
