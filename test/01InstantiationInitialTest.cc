#include "catch.hpp"
#include <etl/cstring.h>

using string = etl::string<64>;


template <typename EventT>
class StateMachine {
public:
    using EventType = EventT;



};

/**
 * @brief TEST_CASE
 */
TEST_CASE ("First instantiation", "[Instantiation]") {



    REQUIRE (true);

    StateMachine <string> machine;


}
