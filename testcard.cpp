module;

#include <iostream>
import cards;

export module testcard;

export namespace TestCards {

int TestStructCard() {
    int testsFailed = 0;
    int testNumber = 0;

    auto Test = [&testsFailed, &testNumber](const bool result, const std::string &description,
                                            const bool UpdateNumber = true) {
        if (!result) {
            std::cout << "Test failed " << testNumber << " " << description << "\n";
            ++testsFailed;
        }

        if (UpdateNumber)
            ++testNumber;

        return result;
    };

    auto RunTestCardIntegrity = [&testsFailed, &testNumber,
                                 &Test](const int intVal, bool isValid,
                                        const std::string &&testVal = "") {
        cards::Card cd;
        cd.crd = intVal;
        Test(isValid == cd.IsValid(), " Card Test Integrity Test number ", false) &&
            Test((testVal.length() == 0) || (testVal == cd.to_string()),
                 "Card Test Integrity incorrect description ");
    };

    RunTestCardIntegrity(0, true, "2C");
    RunTestCardIntegrity(51, true, "AS");
    RunTestCardIntegrity(13, true, "2D");
    RunTestCardIntegrity(53, false, "");
    RunTestCardIntegrity(14 | 128, true, "3C");
    RunTestCardIntegrity(53 | 128, false, "");

    Test(!bool(cards::CharToSuit('F')), "Test optional");
    Test(!bool(cards::SuitToNumber("invalid")), "Test optional");
    Test(!bool(cards::CardToNumber("invalid")), "Test optional");

    {
        cards::Card cd;
        cd.FromString("7H");
        RunTestCardIntegrity(cd.crd, true, "7H");
    }
    {
        cards::Card cd;
        cd.FromString("10S");
        RunTestCardIntegrity(cd.crd, true, "10S");
    }
    {
        cards::Card cd;
        cd.FromString("9D");
        Test(cd.Suit() == cards::suit::diamonds, "suit returned is incorrect");
    }

    {
        cards::Card cd;
        cd.FromString("JC");
        Test(cd.PointCount() == 1, "Jack is 1 point");
        cd.FromString("QS");
        Test(cd.PointCount() == 2, "Queen is 2 points");
        cd.FromString("AH");
        Test(cd.PointCount() == 4, "Ace is 4 points");
    }

    Test(cards::MakeCard("3S").to_link() == "S3", "test to_link");

    {
        cards::Card cd;
        cd.FromString("10H");
        cd.SetPlayed();
        Test(cd.CardHasPlayed(), "Is played");
        cd.SetPlayed(false);
        Test(!cd.CardHasPlayed(), "Is not played");
        Test(cd.to_string() == "10H", "Setting Played doesnt change value");
    }

    {
        cards::Card c1, c2;
        c1.FromString("7S");
        c2.FromString("8S");
        Test(c1 < c2, "comparison same suit");
        c2.FromString("7S");
        c2.CardHasPlayed();
        Test(c1 == c2, "comparison doesnt care if you played the card");
    }

    {
        cards::Card c1, c2;
        c1.FromString("JD");
        c2 = c1;
        Test(c2.to_string() == "JD", "check assignement");
    }

    {
        using enum cards::vulnerability;
        Test(cards::vulnerabilitytoLink(neither) == "", "vul n");
        Test(cards::vulnerabilitytoLink(eastwest) == "e", "vul n");
        Test(cards::vulnerabilitytoLink(northsouth) == "e", "vul n");
        Test(cards::vulnerabilitytoLink(both) == "b", "vul n");
    }

    {
        const auto postext = "pos offset test";
        const auto OffsetPosText = "pos diff test";
        Test(cards::positiondiff(cards::position::south, cards::position::south) == 0, postext);
        Test(cards::positiondiff(cards::position::south, cards::position::west) == 1, postext);
        Test(cards::positiondiff(cards::position::east, cards::position::west) == 2, postext);
        Test(cards::positiondiff(cards::position::west, cards::position::east) == 2, postext);
        Test(cards::positiondiff(cards::position::west, cards::position::south) == 3, postext);
        Test(cards::positiondiff(cards::position::south, cards::position::east) == 3, postext);
        Test(cards::OffsetPosition(cards::position::west, 2) == cards::position::east,
             OffsetPosText);
        Test(cards::OffsetPosition(cards::position::north, 3) == cards::position::west,
             OffsetPosText);

        return testsFailed;
    }
}

int TestBid() {
    int testsFailed = 0;
    int testNumber = 0;

    auto Test = [&testsFailed, &testNumber](bool res, const std::string &description) {
        if (!res) {
            std::cout << "Bid Test " << testNumber << " " << description << "\n";
            ++testsFailed;
        }
        ++testNumber;
    };

    cards::bid c1;
    c1.SetNoTrumps(3);
    c1.SetBidder(cards::position::north);
    Test(c1.IsNoTrumps(), " set no trumps");

    auto optsuit = c1.bidSuit();
    Test(!optsuit, "should not return a suit as no trumps");
    Test(*c1.bidSize() == 3, " should be 3 ");

    c1.SetSuit(cards::suit::spades, 6);
    Test(!c1.IsNoTrumps(), " suit contract not nt");

    optsuit = c1.bidSuit();

    Test(*optsuit == cards::suit::spades, "should be spades");
    cards::bid c2 = c1;
    Test(c1 == c2, " are they the same");

    c1.SetSuit(cards::suit::spades, 3);
    c2.SetSuit(cards::suit::hearts, 3);
    Test(c2 < c1, " is less than");
    Test(c1 > c2, " is more than");

    c2.SetNoTrumps(3);
    Test(c2 > c1, "no trumps is higher than spades");

    c2.SetPass();
    Test(c2.IsPass(), "Yes its a pass");

    c2.SetDouble();
    Test(c2.IsDouble(), "Yes its a double");

    Test(!c1.IsReDouble(), "No its not a redouble");

    {
        cards::bid s1;
        Test(!s1.FromString("3"), "not a valid bid");
    }

    {
        cards::bid unInit1;
        Test(!bool(unInit1.bidSize()), "optional returns nothing");
    }

    {
        cards::bid t1, t2;
        t1.FromString("3S");
        t2 = t1;
        t2.SetBidder(cards::position::east);
        Test((t2 < t1) || (t1 > t2), "check spaceship");
    }

    cards::bid c3;
    Test(!c3.IsValid(), "No bid set");

    cards::bid c4;
    c4.FromString("3NT");
    Test(c4.IsNoTrumps(), "is nt test");

    Test(cards::IsOpponent(cards::position::north, cards::position::east), "who is your opponent");
    Test(cards::IsOpponent(cards::position::south, cards::position::west), "who is your opponent1");
    Test(!cards::IsOpponent(cards::position::west, cards::position::east), "who is your partner");
    Test(!cards::IsOpponent(cards::position::north, cards::position::south),
         "who is your partner1");

    cards::bid ptest;
    ptest.SetSuit(cards::suit::spades, 6);
    Test(ptest.to_string() == "6S ", "Invalid print 6S - " + ptest.DumpRaw());
    ptest.SetPass();
    Test(ptest.to_string() == "P  ", "invalid print for pass");
    ptest.SetNoTrumps(3);
    Test(ptest.to_string() == "3NT", "invalid print for 3NT");

    return testsFailed;
}

int TestContract() {
    int testsFailed = 0;
    int testNumber = 0;
    auto Test = [&testsFailed, &testNumber](bool res, const std::string &description) {
        if (!res) {
            std::cout << "Contract Test " << testNumber << " " << description << "\n";
            ++testsFailed;
        }
        ++testNumber;
    };
    {
        cards::deal d(cards::position::east);

        Test(d.contrct.to_string() == "No bids yet\n", "to string of no bids");
        cards::bid b;
        b.SetPass();
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.AddBid(b), "add another pass");
        b.SetSuit(cards::suit::spades, 1);
        Test(d.contrct.AddBid(b), "add a bid of a spade");

        b.SetSuit(cards::suit::hearts, 1);
        Test(!d.contrct.NextBidValid(b), "insufficient bid should be invalid");

        b.SetDouble();
        Test(d.contrct.AddBid(b), "add a double");
        b.SetReDouble();
        Test(d.contrct.AddBid(b), "add a redouble");
        b.SetNoTrumps(1);
        Test(d.contrct.AddBid(b), "add a notrump");
    }
    {
        cards::deal d(cards::position::north);
        cards::bid b("P");
        Test(d.contrct.AddBid(b), "test 4 passes1");
        Test(d.contrct.AddBid(b), "test 4 passes2");
        Test(d.contrct.AddBid(b), "test 4 passes3");
        Test(d.contrct.AddBid(b), "test 4 passes4");
        Test(d.contrct.finalContract.IsValid(),
             "Hand is passed out without a bid, contract is valid pass");
        std::cout << d.contrct.to_string() << "\n";
    }
    {
        cards::deal d(cards::position::south);
        cards::bid b("P");
        Test(d.contrct.AddBid(b), "test 4 passes1");
        Test(d.contrct.AddBid(b), "test 4 passes2");
        Test(d.contrct.AddBid(b), "test 4 passes3");
        cards::bid s1("1S");
        Test(d.contrct.AddBid(s1), "Add a spade");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.IsValid(), " Contract is valid and incomplete");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.finalContract.IsValid(), "Contract is a spade");
        cards::bid testC("1S");
        testC.SetBidder(cards::position::east);
        Test(d.contrct.finalContract == testC, "Is 1 spade by E");
        Test(d.contrct.IsValid(), " Contract is valid");
    }

    {
        cards::deal d(cards::position::south);
        cards::bid b("P");
        Test(d.contrct.AddBid(b), "test 4 passes1");
        Test(d.contrct.AddBid(b), "test 4 passes2");
        Test(d.contrct.AddBid(b), "test 4 passes3");
        cards::bid s1("1S");
        Test(d.contrct.AddBid(s1), "Add a spade");
        cards::bid c2("2C");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.AddBid(b), "add a pass");
        Test(d.contrct.finalContract.IsValid(), "Contract is a spade");
        cards::bid testC("1S");
        testC.SetBidder(cards::position::east);
        Test(d.contrct.finalContract == testC, "Is 1 spade by E");
    }

    {
        cards::deal d(cards::position::south);
        cards::bid b("D");
        Test(!d.contrct.AddBid(b), "Must double a contract");
    }

    {
        cards::deal d(cards::position::south);
        cards::bid b("R");
        Test(!d.contrct.AddBid(b), "Must redouble a contract");
    }

    {
        cards::deal d(cards::position::south);
        Test(d.contrct.AddBid(cards::bid("1H")), "one spade opener");
        Test(d.contrct.AddBid(cards::bid("D")), "double it");
        Test(d.contrct.AddBid(cards::bid("R")), "redouble it");
        Test(d.contrct.AddBid(cards::bid("P")), "pass");
        Test(d.contrct.AddBid(cards::bid("P")), "pass2");
        Test(d.contrct.AddBid(cards::bid("P")), "pass3");
        Test(d.contrct.finalContract.IsValid(), "Contract is valid");
        cards::bid testC("1H");
        testC.SetBidder(cards::position::south);
        Test(d.contrct.finalContract == testC, "Is 1 spade by S");
        std::cout << d.to_link() << "\n";
    }
    {
        cards::deal d(cards::position::east);
        Test(d.contrct.AddBid(cards::bid("P")), "pass by E");          // E
        Test(d.contrct.AddBid(cards::bid("1H")), "one heart opener "); // S
        Test(d.contrct.AddBid(cards::bid("D")), "double it");          // W
        Test(!d.contrct.NextBidValid(cards::bid("D")), "can't double our contract");
        Test(d.contrct.AddBid(cards::bid("P")), "pass a double"); // N
        Test(!d.contrct.NextBidValid(cards::bid("D")), "Can't double as already doubled");
        Test(d.contrct.AddBid(cards::bid("P")), "pass partners double"); // E
        Test(d.contrct.AddBid(cards::bid("R")), "redouble it");          // S
        Test(!d.contrct.NextBidValid(cards::bid("D")), "Can't double if redoubled");
        Test(d.contrct.AddBid(cards::bid("P")), "pass");  // W
        Test(d.contrct.AddBid(cards::bid("P")), "pass2"); // N
        Test(d.contrct.AddBid(cards::bid("P")), "pass3"); // E
        Test(d.contrct.finalContract.IsValid(), "Contract is valid");
        Test(!d.contrct.NextBidValid(cards::bid("2S")), "can't bid after 3 passes");
        cards::bid testC("1H");
        testC.SetBidder(cards::position::south);
        Test(d.contrct.finalContract == testC, "Is 1 heart by S");
    }

    {
        cards::deal d(cards::position::east);
        Test(d.contrct.AddBid(cards::bid("P")), "pass by E");
        Test(d.contrct.AddBid(cards::bid("1H")), "one heart opener "); // S
        Test(d.contrct.AddBid(cards::bid("P")), "pass by W");          // W
        Test(d.contrct.AddBid(cards::bid("P")), "pass by N");          // N
        Test(d.contrct.AddBid(cards::bid("D")), "D bu E");             // E
        Test(d.contrct.AddBid(cards::bid("P")), "P");                  // S
        Test(d.contrct.AddBid(cards::bid("P")), "p2");                 // W
        Test(d.contrct.AddBid(cards::bid("P")), "P3");                 // N
        Test(d.contrct.finalContract.IsValid(), "Contract is valid");
        cards::bid testC("1H");
        testC.SetBidder(cards::position::south);
        Test(d.contrct.finalContract == testC, "Is 1 spade by S");
    }

    {
        cards::deal d(cards::position::east);
        Test(d.contrct.AddBid(cards::bid("P")), "pass by E");
        Test(d.contrct.AddBid(cards::bid("1H")), "one heart opener "); // S
        Test(d.contrct.AddBid(cards::bid("P")), "pass by W");          // W
        Test(!d.contrct.NextBidValid(cards::bid("R")), " Can't redouble if not doubled");
        Test(d.contrct.AddBid(cards::bid("P")), "pass by N"); // N
        Test(d.contrct.AddBid(cards::bid("D")), "D by E");    // E
        Test(d.contrct.AddBid(cards::bid("P")), "P");         // S
        Test(!d.contrct.NextBidValid(cards::bid("R")), " Can't redouble as it is their bid");
        Test(d.contrct.AddBid(cards::bid("P")), "p2");       // W
        Test(d.contrct.AddBid(cards::bid("R")), "Redouble"); // N
        Test(d.contrct.AddBid(cards::bid("P")), "P");        // E
        Test(d.contrct.AddBid(cards::bid("P")), "p2");       // S
        Test(d.contrct.AddBid(cards::bid("P")), "p3");       // W
        Test(!d.contrct.NextBidValid(cards::bid("3S")), " Can't bid after 3 passes");
        Test(!d.contrct.AddBid(cards::bid("3S")), " Can't bid after 3 passes");
        Test(d.contrct.finalContract.IsValid(), "Contract is valid");
        cards::bid testC("1H");
        testC.SetBidder(cards::position::south);
        Test(d.contrct.finalContract == testC, "Is 1 spade by S");
    }

    return testsFailed;
}

int RunDealTests() {

    int testsFailed = 0;
    int testNumber = 0;

    auto Test = [&testsFailed, &testNumber](const bool result, const std::string &description,
                                            const bool UpdateNumber = true) {
        if (!result) {
            std::cout << "Test Deals failed " << testNumber << " " << description << "\n";
            ++testsFailed;
        }

        if (UpdateNumber)
            ++testNumber;

        return result;
    };

    auto TestDeal = [&testsFailed, &testNumber, &Test](const cards::deal &d) {
        {
            // Test assignment
            cards::deal d2 = d;

            Test(d2 == d, "Test if equal");
            std::swap(d2.hands[int(cards::position::south)].crd[5],
                      d2.hands[int(cards::position::east)].crd[5]);
            d2.hands[int(cards::position::east)].SetSuits();
            d2.hands[int(cards::position::south)].SetSuits();
            Test(d2 != d, "Test if not equal");
        }

        Test(d.hands[static_cast<int>(cards::position::south)].PointCount() +
                     d.hands[static_cast<int>(cards::position::east)].PointCount() +
                     d.hands[static_cast<int>(cards::position::west)].PointCount() +
                     d.hands[static_cast<int>(cards::position::north)].PointCount() ==
                 40,
             "There are 40 points in a deck");

        for (int i = 0; i < 4; ++i) {
            Test(d.hands[static_cast<int>(cards::position::south)].SuitLength(i) +
                         d.hands[static_cast<int>(cards::position::east)].SuitLength(i) +
                         d.hands[static_cast<int>(cards::position::west)].SuitLength(i) +
                         d.hands[static_cast<int>(cards::position::north)].SuitLength(i) ==
                     13,
                 "There are 13 cards in " + cards::SuitChar(static_cast<cards::suit>(i)));
        }
    };

    std::string st;
    for (int i = 0; i < 50; ++i) {
        cards::deal d;
        d.contrct.SetDealer(cards::position::north);
        d.SetVulnerability(cards::vulnerability::both);
        Test(d.GetVulnerability() == cards::vulnerability::both, "check vul");
        TestDeal(d);

        st = d.to_string();
        st = d.to_link();
        // std::cout << st << "\n";
    }

    {
        cards::deal d;
        d.contrct.SetDealer(cards::position::north);
        d.SetVulnerability(cards::vulnerability::both);
        cards::deal d2 = d;
        cards::trick t;
        t.SetLeadPos(cards::position::east);

        cards::Hand &h = d2.hands[static_cast<int>(cards::position::east)];
        cards::Card cd;
        cd = h.crd[0];
        h.PlayCard(cd);
        t.Lead(cd);

        d2.AddTrick(t);
        Test(d != d2, "different number of tricks played");
    }
    // std::cout << st << "\n";
    // std::cout << " dealt 50 hands\n";

    return testsFailed;
}

int TestTricks() {

    int testsFailed = 0;
    int testNumber = 0;

    auto Test = [&testsFailed, &testNumber](const bool result, const std::string &description,
                                            const bool UpdateNumber = true) {
        if (!result) {
            std::cout << "Test Tricks failed " << testNumber << " " << description << "\n";
            ++testsFailed;
        }

        if (UpdateNumber)
            ++testNumber;

        return result;
    };

    {
        cards::trick t1;
        t1.SetLeadPos(cards::position::south);
        t1.SetNoTrumps();

        t1.Lead(cards::MakeCard("3S"));
        Test(!bool(t1.WonBy()), "No winner as not enough tricks");

        const auto TestTrickText = "Test tricks";

        Test(t1.CardWillWin(cards::MakeCard("4S")), TestTrickText);
        Test(!t1.CardWillWin(cards::MakeCard("2S")), TestTrickText);
        Test(!t1.CardWillWin(cards::MakeCard("JH")), TestTrickText);
    }

    {
        cards::trick t2;
        t2.SetLeadPos(cards::position::west);
        t2.SetTrumps(cards::suit::hearts);

        t2.Lead(cards::MakeCard("3D"));

        const auto testText = "Checking suit";
        Test(t2.CardWillWin(cards::MakeCard("5D")), testText);
        Test(!t2.CardWillWin(cards::MakeCard("2D")), testText);
        Test(!t2.CardWillWin(cards::MakeCard("5C")), testText);
        Test(t2.CardWillWin(cards::MakeCard("2H")), testText);
    }

    {
        cards::trick t3;
        t3.SetLeadPos(cards::position::north);
        t3.SetTrumps(cards::suit::clubs);

        t3.Lead(cards::MakeCard("QD"));

        t3.PlayCard(cards::MakeCard("4D"));
        t3.PlayCard(cards::MakeCard("5D"));
        t3.PlayCard(cards::MakeCard("KD"));

        std::cout << t3.to_link() << "\n";

        Test(t3.to_link() == "pc|DQ|pc|D4|pc|D5|pc|DK|", "convert trick to link syntax");

        Test(t3.PlayersToGo() == 0, "players to go is zero");
        Test(*(t3.WonBy()) == cards::position::west, "Correct return from WonBy");
    }

    {
        cards::trick t4;
        t4.SetLeadPos(cards::position::east);
        t4.SetTrumps(cards::suit::diamonds);

        t4.Lead(cards::MakeCard("QH"));

        t4.PlayCard(cards::MakeCard("4H"));
        t4.PlayCard(cards::MakeCard("3D"));
        t4.PlayCard(cards::MakeCard("KH"));

        Test(t4.PlayersToGo() == 0, "no players to go");
        Test(*(t4.WonBy()) == cards::position::west, "Correct return from WonBy");
    }

    {
        cards::trick t5;
        t5.SetLeadPos(cards::position::east);
        t5.SetTrumps(cards::suit::hearts);

        t5.Lead(cards::MakeCard("QH"));     // E
        t5.PlayCard(cards::MakeCard("4H")); // S
        t5.PlayCard(cards::MakeCard("3D")); // W
        t5.PlayCard(cards::MakeCard("KH")); // N

        Test(t5.PlayersToGo() == 0, "no players to go");
        Test(*(t5.WonBy()) == cards::position::north, "Correct return from WonBy");
    }

    {
        cards::trick t6;
        t6.SetLeadPos(cards::position::east);
        t6.SetTrumps(cards::suit::hearts);

        t6.Lead(cards::MakeCard("QD"));     // E
        t6.PlayCard(cards::MakeCard("4H")); // S
        t6.PlayCard(cards::MakeCard("3D")); // W
        t6.PlayCard(cards::MakeCard("KH")); // N

        Test(t6.PlayersToGo() == 0, "no players to go");
        Test(*(t6.WonBy()) == cards::position::north, "Correct return from WonBy");
    }

    return testsFailed;
}

int RunAllTests() {
    int testsFailed = 0;
    testsFailed += TestStructCard();
    testsFailed += RunDealTests();
    testsFailed += TestBid();
    testsFailed += TestContract();
    testsFailed += TestTricks();

    if (testsFailed > 0) {
        std::cout << "Some tests failed" << std::endl;
    }
    return testsFailed;
}
} // namespace TestCards