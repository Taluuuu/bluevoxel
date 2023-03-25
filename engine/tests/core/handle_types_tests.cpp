#include "core/handle_types.h"

#include <catch2/catch_test_macros.hpp>

class A {};
class B : public A {};

TEST_CASE("OwningHandle and WeakHandle")
{
    h2o::OwningHandle<A> o1;
    h2o::WeakHandle<A> w1;
    h2o::WeakHandle<A> w2 = w1;

    REQUIRE_FALSE(o1.is_valid());
    REQUIRE_FALSE(w1.is_valid());

    w1 = o1;

    REQUIRE_FALSE(o1.is_valid());
    REQUIRE_FALSE(w1.is_valid());

    o1.reset(new A);

    REQUIRE(o1.is_valid());
    REQUIRE_FALSE(w1.is_valid());

    w1 = o1;

    //REQUIRE(o1.is_valid());
    //REQUIRE(w1.is_valid());
}