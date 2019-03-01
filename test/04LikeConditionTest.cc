/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "BeginsWithCondition.h"
#include "LikeCondition.h"
#include "catch.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

/**
 * @brief TEST_CASE
 */
TEST_CASE ("StringCondition test", "[StateMachine Conditions]")
{
        StringCondition ok ("ala ma kota", StripInput::DONT_STRIP);
        REQUIRE (!ok.checkImpl ("janek ma psa"));
        REQUIRE (ok.checkImpl ("ala ma kota"));
        REQUIRE (!ok.checkImpl (" ala ma kota"));
        REQUIRE (!ok.checkImpl ("ala ma kota "));
        REQUIRE (!ok.checkImpl (" ala ma kota "));
        REQUIRE (!ok.checkImpl ("ala ma kota\r\n"));

        StringCondition con ("ala ma kota", StripInput::STRIP);

        REQUIRE (!con.checkImpl ("janek ma psa"));
        REQUIRE (con.checkImpl ("ala ma kota"));
        REQUIRE (con.checkImpl (" ala ma kota"));
        REQUIRE (con.checkImpl ("ala ma kota "));
        REQUIRE (con.checkImpl (" ala ma kota "));
        REQUIRE (con.checkImpl ("ala ma kota\r\n"));
}

/**
 * @brief TEST_CASE
 */
TEST_CASE ("StringCondition starts", "[StateMachine Conditions]")
{
        BeginsWithCondition ok ("test", StripInput::DONT_STRIP);

        REQUIRE (!ok.checkImpl ("janek ma psa"));
        REQUIRE (ok.checkImpl ("test ma kota"));
        REQUIRE (!ok.checkImpl (" test  ma kota"));
        REQUIRE (ok.checkImpl ("test ma kota "));
        REQUIRE (!ok.checkImpl (" test ma kota "));
        REQUIRE (ok.checkImpl ("test ma kota\r\n"));

        BeginsWithCondition con ("franio", StripInput::STRIP);

        REQUIRE (!con.checkImpl ("janek ma psa"));
        REQUIRE (con.checkImpl ("franio ma kota"));
        REQUIRE (con.checkImpl (" franio ma kota"));
        REQUIRE (con.checkImpl ("franio ma kota "));
        REQUIRE (con.checkImpl (" franio ma kota "));
        REQUIRE (con.checkImpl ("franio ma kota\r\n"));
}

TEST_CASE ("Like condition wo stripping test", "[StateMachine Conditions]")
{
        LikeCondition like1 ("+CSQ: 99:%:66");

        REQUIRE (like1.checkImpl ("+CSQ: 99:0:66")); // select '+CSQ: 99:0:66' like '+CSQ: 99:%:66'; => t
        REQUIRE (!like1.checkImpl ("+CSQ: 99:0:666"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:2345:66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99::66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:JHGHJ:88:66"));
        REQUIRE (like1.checkImpl ("+CSQ: 99:99:66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 99:99#66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98:0:66"));
        REQUIRE (!like1.checkImpl ("CSQ: 99:0:66"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98:0"));
        REQUIRE (!like1.checkImpl ("CSQ: 99:0"));
        REQUIRE (!like1.checkImpl ("+CSQ: 98"));
        REQUIRE (!like1.checkImpl ("CSQ: 99"));
        REQUIRE (!like1.checkImpl (""));

        // TODO ten test ma przechodzić
        // REQUIRE (like1.checkImpl ("+CSQ: 99::66:66")); // select '+CSQ: 99::66:66' like '+CSQ: 99:%:66';

        LikeCondition like2 ("+CSQ: 99:%");

        REQUIRE (like2.checkImpl ("+CSQ: 99:0"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:2345"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:JHGHJ:88"));
        REQUIRE (like2.checkImpl ("+CSQ: 99:99"));
        REQUIRE (!like2.checkImpl ("+CSQ: 98:0"));
        REQUIRE (!like2.checkImpl ("CSQ: 99:0"));

        LikeCondition like3 ("99:%:66|%|77");

        REQUIRE (like3.checkImpl ("99:bb:66|aa|77"));
        REQUIRE (like3.checkImpl ("99:b:66|a|77"));
        REQUIRE (like3.checkImpl ("99::66||77"));
        REQUIRE (like3.checkImpl ("99:bb:77:66|aa|78|77"));
        REQUIRE (!like3.checkImpl ("99:bb:77:66|aa|78|79"));

        //        REQUIRE (like3.checkImpl ("99::66||77"));

        LikeCondition like4 ("+CREG: %,0%");
        REQUIRE (like4.checkImpl ("+CREG: 1,0"));
        REQUIRE (like4.checkImpl ("+CREG: 0,0"));
        REQUIRE (like4.checkImpl ("+CREG: 0,0\r\n"));

        // TODO zaimplementować _
        //        LikeCondition like3 ("+CSQ:_99");

        LikeCondition like5 ("%41 0C%", StripInput::STRIP, InputRetention::RETAIN_INPUT);

        REQUIRE (like5.checkImpl (">41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r>41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 40 "));
        REQUIRE (like5.checkImpl (">41 0C 00 40\r"));
        REQUIRE (like5.checkImpl (">41 0C 00 40\r\n"));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 40\r\n"));

        REQUIRE (like5.checkImpl ("41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 40 "));
        REQUIRE (like5.checkImpl ("41 0C 00 40\r"));
        REQUIRE (like5.checkImpl ("41 0C 00 40\r\n"));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 40\r\n"));

        REQUIRE (like5.checkImpl (">41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r>41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 00 "));
        REQUIRE (like5.checkImpl (">41 0C 00 00\r"));
        REQUIRE (like5.checkImpl (">41 0C 00 00\r\n"));
        REQUIRE (like5.checkImpl ("\r\n>41 0C 00 00\r\n"));

        REQUIRE (like5.checkImpl ("41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 00 "));
        REQUIRE (like5.checkImpl ("41 0C 00 00\r"));
        REQUIRE (like5.checkImpl ("41 0C 00 00\r\n"));
        REQUIRE (like5.checkImpl ("\r\n41 0C 00 00\r\n"));
}

TEST_CASE ("Like condition all", "[StateMachine Conditions]")
{
        LikeCondition like1 ("%");

        REQUIRE (like1.checkImpl ("a"));
        REQUIRE (like1.checkImpl ("ajkds ds sd dsf dfkjldskjldf"));
        REQUIRE (like1.checkImpl ("")); // Data base (which DB) like operator catches even empty strings, so mine behaves the same.
}
