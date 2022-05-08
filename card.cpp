module;

#include <algorithm>
#include <array>
#include <cassert>
#include <compare>
#include <iostream>
#include <optional>
#include <random>
#include <ranges>
#include <string>

export module cards;

export namespace cards {
using CardInt = unsigned char;

enum {
    CardsInSuit = 13,
    CardsInHand = 13,
    SuitsInDeck = 4,
    MaxBidSize = 7,
    numPlayers = 4,
    CardsInDeck = CardsInSuit * SuitsInDeck
};

const CardInt PlayedBit = 128;

enum class position : char { south = 0, west = 1, north = 2, east = 3 };

inline std::string PositionToString(position p) {
    const std::array<const char *, numPlayers> stp = {"South", "West", "North", "East"};
    std::string retval = stp[static_cast<char>(p)];
    return retval;
};

std::string positiontolinkdealer(position p) { return std::to_string(static_cast<int>(p) + 1); }

constexpr inline bool IsOpponent(position me, position test) {
    return (static_cast<char>(me) % 2) != (static_cast<char>(test) % 2);
}

constexpr inline position Lefty(position me) {
    return static_cast<position>((static_cast<int>(me) + 1) % numPlayers);
}

inline position operator++(position &p, int) {
    char ch = static_cast<char>(p);
    assert(ch >= 0);
    assert(ch <= 3);
    position retp = static_cast<position>(ch);
    ch++;
    p = static_cast<position>(ch);
    return retp;
};

inline int positiondiff(position lead, position current) {
    int l = static_cast<int>(lead);
    int r = static_cast<int>(current);
    int res = (r - l + numPlayers) % numPlayers;
    assert(res >= 0 && res < numPlayers);
    return res;
}

inline position OffsetPosition(position lead, int offset) {
    assert(offset > 0);
    assert(offset <= numPlayers);
    int l = static_cast<int>(lead);
    int res = (l + offset) % numPlayers;
    return static_cast<position>(res);
}

inline int GetPositionOffset(position lead, position pos) {
    int l = static_cast<int>(lead);
    int p = static_cast<int>(pos);
    return (p + numPlayers - l) % numPlayers;
}

enum class suit : char { clubs = 0, diamonds = 1, hearts = 2, spades = 3, notrumps = 99 };
const std::string SuitVal = "CDHS";
const std::string CardVal = "23456789TJQKA";

inline bool IsValid(const suit s) {
    int i = static_cast<int>(s);
    return (i >= 0) && (i < SuitsInDeck);
}

const char SuitChar(suit s) {
    assert(int(s) >= 0);
    assert(int(s) < SuitsInDeck);
    return SuitVal[static_cast<int>(s)];
}

std::optional<suit> CharToSuit(char c) {
    auto pos = SuitVal.find(c);
    if (pos != std::string::npos)
        return static_cast<suit>(pos);
    return {};
}

std::optional<CardInt> SuitToNumber(const std::string &st) {
    auto pos = std::string::npos;
    auto stlen = st.length();
    if (stlen > 2) {
        pos = SuitVal.find(st[2]);
    } else if (stlen > 1) {
        pos = SuitVal.find(st[1]);
    }
    if (pos != std::string::npos) {
        return static_cast<CardInt>(pos * CardsInSuit);
    }
    return {};
}

std::optional<CardInt> CardToNumber(const std::string &st) {
    auto pos = std::string::npos;
    auto stlen = st.length();
    if (stlen > 2) {
        if (st[0] == '1' && st[1] == '0')
            return 8;
    }
    if (stlen > 1) {
        pos = CardVal.find(st[0]);
    }
    if (pos != std::string::npos) {
        return static_cast<CardInt>(pos);
    }
    return {};
}

inline CardInt NotPlayed(const CardInt crd) { return crd & ~PlayedBit; }

struct Card {
    CardInt crd;

    bool IsValid() const {
        CardInt ch = NotPlayed(crd);
        return ch < CardsInDeck;
    }
    suit Suit() const {
        CardInt ch = (NotPlayed(crd) / CardsInSuit);
        assert(ch >= 0);
        assert(ch <= 3);
        return static_cast<suit>(ch);
    };

    char val() const {
        CardInt ch = NotPlayed(crd);
        assert(ch >= 0);
        assert(ch < 52);
        ch = ch % CardsInSuit;
        assert(ch < 13);
        return static_cast<char>(ch);
    }

    int PointCount() const {
        char ch = val() - 8;
        if (ch > 0)
            return ch;
        return 0;
    }

    bool CardHasPlayed() const { return (crd & PlayedBit) != 0; }
    void SetPlayed(bool newval = true) {
        if (newval)
            crd = crd | PlayedBit;
        else
            crd = crd & ~PlayedBit;
    };

    static const bool Use_T_Val = false;

    std::string ShowVal(const bool use10 = true) const {
        assert(IsValid());

        std::string s;
        if (use10 && (val() == 8))
            s = "10";
        else
            s += CardVal[val()];
        return s;
    }

    std::string ShowSuit() const {
        assert(IsValid());
        return {SuitChar(Suit())};
    }

    std::string to_string() const {
        assert(IsValid());
        return ShowVal() + ShowSuit();
    }

    std::string to_link() const {
        assert(IsValid());
        return ShowSuit() + ShowVal(Use_T_Val);
    }

    bool operator==(const Card &cd) const { return NotPlayed(crd) == NotPlayed(cd.crd); }

    auto operator<=>(const Card &cd) const { return NotPlayed(crd) <=> NotPlayed(cd.crd); }

    bool FromString(const std::string &inval) {
        crd = CardsInDeck + 1; // invalid value
        auto suit = SuitToNumber(inval);
        auto val = CardToNumber(inval);
        if (suit && val) {
            crd = *suit + *val;
            return true;
        }
        return false; // unreachable
    }
};

Card MakeCard(const std::string &st) {
    Card cd;
    cd.FromString(st);
    return cd;
}

struct Hand {
    std::array<Card, CardsInHand> crd;
    std::array<char, SuitsInDeck> SuitLengths;

    void SetSuits() {
        std::sort(std::begin(crd), std::end(crd));
        std::ranges::fill(SuitLengths, 0);
        for (const auto &cd : crd) {

            assert(!cd.CardHasPlayed());
            SuitLengths[static_cast<int>(cd.Suit())]++;
        }
    }

    bool PlayCard(const Card cd) {
        auto ub = std::upper_bound(std::begin(crd), std::end(crd), cd);

        if (ub != std::end(crd)) {
            ub--;
            if (ub != std::begin(crd)) {
                int ind = std::distance(std::begin(crd), ub);
                assert(cd == crd[ind]);
                crd[ind].SetPlayed();
                SuitLengths[static_cast<int>(crd[ind].Suit())]--;
                return true;
            }
        }
        return false;
    }

    int SuitLengthRemaining(const suit s) const {
        assert(IsValid(s));
        return SuitLengths[static_cast<int>(s)];
    }

    int PointCount() const {
        int count = 0;
        for (const auto &c : crd) {
            count += c.PointCount();
        }
        return count;
    }
    std::string to_string() const {
        std::array<std::string, SuitsInDeck> suits;
        for (const auto &cd : crd) {
            std::string &suitSt = suits[static_cast<int>(cd.Suit())];
            if (suitSt.length() != 0) {
                suitSt.insert(0, 1, ',');
            }
            suitSt.insert(0, cd.ShowVal());
        }
        std::string out;
        out += "S " + suits[static_cast<int>(suit::spades)] + "\n";
        out += "H " + suits[static_cast<int>(suit::hearts)] + "\n";
        out += "D " + suits[static_cast<int>(suit::diamonds)] + "\n";
        out += "C " + suits[static_cast<int>(suit::clubs)] + "\n";
        return out;
    }

    std::string to_link() const {
        std::array<std::string, SuitsInDeck> suits;
        for (const auto &cd : crd) {
            std::string &suitSt = suits[static_cast<int>(cd.Suit())];

            suitSt.insert(0, cd.ShowVal(cards::Card::Use_T_Val));
        }
        std::string lnk;
        lnk += "S" + suits[static_cast<int>(suit::spades)];
        lnk += "H" + suits[static_cast<int>(suit::hearts)];
        lnk += "D" + suits[static_cast<int>(suit::diamonds)];
        lnk += "C" + suits[static_cast<int>(suit::clubs)];
        return lnk;
    }

    int SuitLength(int s) const {
        assert(int(s) >= 0);
        assert(int(s) < SuitsInDeck);
        return SuitLengths[static_cast<int>(s)];
    }

    bool operator==(const Hand &h1) {
        for (int i = 0; i < SuitsInDeck; ++i) {
            if (SuitLength(i) != h1.SuitLength(i))
                return false;
        }
        for (int i = 0; i < CardsInHand; ++i) {
            if (crd[i] != h1.crd[i])
                return false;
            assert(crd[i] == h1.crd[i]);
        }
        return true;
    }
};

class bid {
  private:
    char s;
    enum {
        InvalidBid = -1,
        ModValue = 5,
        PassValue = 0,
        DoubleValue = 1,
        ReDoubleValue = 2,
        NoTrumpValue = 4,
        offset = 3
    };

    position bidder;

  public:
    bid() {
        s = InvalidBid;
        bidder = position::south;
    }

    bool FromString(const std::string &st) {
        auto len = st.length();

        if (len > 0) {
            switch (st[0]) {
            case 'P': {
                SetPass();
                return true;
            }
            case 'D': {
                SetDouble();
                return true;
            }
            case 'R': {
                SetReDouble();
                return true;
            }
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7': {
                int contractsize = st[0] - '0';
                if (len > 1) {
                    switch (st[1]) {
                    case 'N': {
                        SetNoTrumps(contractsize);
                        return true;
                    }
                    default: {
                        auto optsuit = CharToSuit(st[1]);
                        if (optsuit) {
                            SetSuit(*optsuit, contractsize);
                            return true;
                        };
                        assert(false); // unreachable
                        break;         // unreachable
                    };
                    }
                };
                break;
            }
            default: {         // unreachable
                assert(false); // unreachable
                break;         // unreachable
            }
            }
        }
        return false;
    }

    bid(const std::string &st) : bid() { FromString(st); }

    std::string DumpRaw() { return std::to_string(s); }

    void SetBidder(position b) { bidder = b; }

    position GetBidder() const { return bidder; }

    bool IsValid() const {
        return (s >= 0) && (s <= static_cast<int>(MaxBidSize) * static_cast<int>(ModValue) +
                                     static_cast<int>(offset));
    }

    void SetNoTrumps(int num) {
        if ((num > 0) && (num <= MaxBidSize)) {
            s = ModValue * (num - 1) + offset + NoTrumpValue;
        } else {
            s = InvalidBid; // unreachable
            assert(false);  // unreachable
        }
    }

    void SetPass() { s = PassValue; }
    void SetDouble() { s = DoubleValue; }
    void SetReDouble() { s = ReDoubleValue; }

    void SetSuit(suit st, int num) {
        if ((num > 0) && (num < MaxBidSize)) {
            int su = static_cast<int>(st);
            assert(su >= 0);
            assert(su < SuitsInDeck);
            s = (num - 1) * ModValue + su + offset;
        } else {
            s = -1;        // unreachable
            assert(false); // unreachable
        }
    }

    std::optional<int> bidSize() const {
        if (s >= 0) {
            if (s >= offset) {
                int cs = ((s - offset) / ModValue) + 1;
                assert(cs < MaxBidSize);
                assert(cs > 0);
                return cs;
            }
        }
        return {};
    }

    bool IsABid() const {
        // rather than a pass or a double...
        return (s >= offset);
    }

    bool IsNoTrumps() const {
        assert(IsValid());
        if (s > offset) {
            return ((s - offset) % ModValue) == NoTrumpValue;
        }
        return false; // unreachable
    }

    bool IsPass() const {
        assert(IsValid());
        return s == PassValue;
    }

    bool IsDouble() const {
        assert(IsValid());
        return s == DoubleValue;
    }

    bool IsReDouble() const {
        assert(IsValid());
        return (s == ReDoubleValue);
    }

    std::optional<suit> bidSuit() const {
        if (s >= offset) {
            int res = (s - offset) % ModValue;
            if (res < SuitsInDeck)
                return static_cast<suit>(res);
        }
        return {};
    }

    auto operator<=>(const bid &otherc) const {

        assert(IsValid());
        assert(otherc.IsValid());
        if (s != otherc.s)
            return s <=> otherc.s;
        return bidder <=> otherc.bidder;
    }

    bool operator==(const bid &otherc) const {
        assert(IsValid());
        assert(IsValid());

        return (s == otherc.s) && (bidder == otherc.bidder);
    }
    std::string to_link() const {
        if (s >= 0) {
            if (IsPass()) {
                return "P";
            }
            if (IsDouble()) {
                return "D";
            }
            if (IsReDouble()) {
                return "R";
            }
            if (IsNoTrumps()) {
                return std::to_string(*bidSize()) + "NT";
            }
            return std::to_string(*bidSize()) + SuitChar(*bidSuit());
        }
        assert(false); // unreachable
        return "INV";  // unreachable
    }
    std::string to_string() const {
        std::string st = to_link();
        if (st.size() < 3)
            st.append(3 - st.size(), ' ');
        return st;
    }
};

enum class vulnerability { neither, eastwest, northsouth, both };

inline std::string vulnerabilitytoLink(vulnerability v) {

    switch (v) {
    case vulnerability::neither:
        return "";
    case vulnerability::eastwest:
        return "e";
    case vulnerability::northsouth:
        return "n";
    case vulnerability::both:
        return "b";
    }
    return ""; // unreachable
}

struct contract {
    position declarer;
    position dealer;
    bid finalContract;
    std::vector<bid> bids;

    bool HasThreePasses() const {
        // Check is > 3, not >= as otherwise not finished...
        if (bids.size() > 3) {
            /*bool res1  = std::find_if_not(std::begin(bids), std::begin(bids) + 3, [](const bid &b)
               { return b.IsPass();
                   }) == std::begin(bids) + 3;*/
            return bids[0].IsPass() && bids[1].IsPass() && bids[2].IsPass();
        }
        return false;
    }

    bool IsValid() const {
        if (finalContract.IsValid()) {
            return HasThreePasses();
        } else {
            return true;
        }
    }

    auto PotentialContract() const {
        return std::find_if(std::begin(bids), std::end(bids),
                            [](const bid &b) { return b.IsABid(); });
    }

    void SetDealer(position newd) {
        bid b;
        finalContract = b; // invalidates finalContract
        bids.clear();
        dealer = newd;
    }

    position GetDealer() const { return dealer; }

    position NextToBid() const {
        assert(!finalContract.IsValid());
        if (bids.size() > 0)
            return Lefty(bids[0].GetBidder());
        else
            return dealer;
    }

    bool NextBidValid(const bid &newBid) const {
        if (!newBid.IsValid()) {
            assert(false); // unreachable
            return false;  // unreachable
        }
        if (finalContract.IsValid()) {
            return false; // Are we finished bidding already?
        }
        if (newBid.IsPass()) {
            return true; // We can always pass
        }

        position me = NextToBid();

        if (newBid.IsDouble()) {
            auto pc = PotentialContract();
            if (pc == bids.end()) {
                return false; // No potential contract to double
            }
            if (!IsOpponent((*pc).GetBidder(), me)) {
                return false; // Cannot double our contract
            }
            if (begin(bids) == pc) {
                return true; // righty bid so we can double
            }
            if (begin(bids)->IsReDouble()) {
                return false; // Righty redoubled so we can't double
            }
            assert((begin(bids))->IsPass()); // If righty didn't redouble or bid
                                             // they passed
            if ((begin(bids) + 1)->IsDouble()) {
                return false; // Partner already doubled
            } else
                return true;
            assert(false);
            return false;
        }

        if (newBid.IsReDouble()) {
            auto pc = PotentialContract();
            if (pc == bids.end()) {
                return false; // No Potential contract to redouble
            }
            if (IsOpponent((*pc).GetBidder(), me)) {
                return false; // Cannot redouble their  contract
            }
            if (bids[0].IsDouble()) {
                return true; // Righty doubled so we can redouble
            }
            // righty passed, partner passed, lefty doubled
            if (pc - begin(bids) > 2) {
                return (bids[2].IsDouble() && bids[1].IsPass() && bids[0].IsPass());
            } else
                return false;
            assert(false); // unreachable
        }

        if (newBid.IsABid()) {
            auto pc = PotentialContract();
            // has their been a previous bid
            if (pc == std::end(bids)) {
                return true;
            }
            // is the bid bigger
            if (newBid > (*pc)) {
                return true;
            } else
                return false;
        }
        assert(false); // unreachable
        return false;
    }

    bool AddBid(bid addBid) {
        if (finalContract.IsValid()) {
            return false;
        }
        if (NextBidValid(addBid)) {
            addBid.SetBidder(NextToBid());
            bids.insert(bids.begin(), addBid);
            if (addBid.IsPass()) {
                if (HasThreePasses()) {
                    auto pc = PotentialContract();
                    if (pc == std::end(bids)) {
                        finalContract = addBid;
                    } else {
                        finalContract = *pc;
                        declarer = (*pc).GetBidder();
                    }
                }
            }
            return true;
        }
        return false;
    }

    std::string to_string() const {
        if (bids.size() == 0) {
            return "No bids yet\n";
        }

        std::string out = "S   W   N   E\n";
        size_t i = static_cast<int>(dealer);
        assert(i < 4);
        std::string chars(i * 4, ' ');

        std::cout << "length chars " << chars.length() << "\n";
        out += chars;

        bool nlneeded = false;

        std::for_each(std::rbegin(bids), std::rend(bids), [&out, &i, &nlneeded](const bid &b) {
            out += b.to_string();
            out += ' ';
            ++i;
            nlneeded = true;
            if ((i % 4 == 0)) {
                out += '\n';
                nlneeded = false;
            }
        });
        if (nlneeded)
            out += '\n';

        if (finalContract.IsValid()) {
            out += "Contract " + finalContract.to_string() + '\n';
        }
        return out;
    }
};

class trick {
  private:
    std::array<Card, numPlayers> crd;
    position leadPlayer;
    position wonByPlayer;
    signed char cardsPlayed;
    suit trumps;

  public:
    std::optional<position> WonBy() const {
        if (cardsPlayed == numPlayers)
            return wonByPlayer;
        return {};
    }

    Card GetCard(const cards::position p) const {
        return crd[GetPositionOffset(leadPlayer, wonByPlayer)];
    }

    Card GetCardPlayed(int i) const {
        assert(i >= 0);
        assert(i < CardsInSuit);
        return crd[i];
    }

    void SetLeadPos(position p) {
        cardsPlayed = 0;
        leadPlayer = p;
    }

    void SetTrumps(suit s) {
        cardsPlayed = 0;
        trumps = s;
    }

    void SetNoTrumps() {
        cardsPlayed = 0;
        trumps = suit::notrumps;
    }

    void InitFromContract(const cards::contract &c) {
        assert(c.IsValid());
        assert(c.finalContract.IsValid());
        if (c.finalContract.IsABid()) {
            if (c.finalContract.IsNoTrumps()) {
                SetNoTrumps();
            } else {
                SetTrumps(*c.finalContract.bidSuit());
            }
        }
    }

    void Lead(Card ld) {
        assert(ld.IsValid());
        cardsPlayed = 1;
        crd[0] = ld;
        wonByPlayer = leadPlayer;
    }

    int PlayersToGo() const {
        assert(cardsPlayed >= 0);
        assert(cardsPlayed <= numPlayers);
        return numPlayers - cardsPlayed;
    }

    std::string to_link() const {
        std::string lnk;
        for (const auto &c : crd) {
            lnk += "pc|" + c.to_link() + "|";
        }
        return lnk;
    }

  private:
    bool CardWillWinNT(Card newc) const {
        int winpos = positiondiff(leadPlayer, wonByPlayer);
        assert(newc.IsValid());
        assert(crd[winpos].IsValid());
        assert(cardsPlayed < numPlayers && cardsPlayed > 0);
        assert(winpos < cardsPlayed);
        assert(leadPlayer == wonByPlayer || (crd[0].Suit() == crd[winpos].Suit()));

        if (newc.Suit() == crd[winpos].Suit())
            return newc > crd[winpos];
        else
            return false;
    }

    bool CardWillWinSuit(Card newc, suit trumps) const {
        assert(cardsPlayed > 0);
        if (crd[0].Suit() == trumps) {
            return CardWillWinNT(newc);
        }
        assert(cardsPlayed > 0);
        assert(newc.IsValid());
        int winpos = positiondiff(leadPlayer, wonByPlayer);
        assert(crd[winpos].IsValid());
        assert(cardsPlayed < numPlayers);
        assert(winpos < cardsPlayed);
        assert((crd[winpos].Suit() == trumps) || crd[winpos].Suit() == crd[0].Suit());
        if (newc.Suit() == trumps) {
            if (crd[winpos].Suit() == trumps)
                return newc > crd[winpos];
            else
                return true;
        } else {
            if (crd[winpos].Suit() == trumps)
                return false;
            else {
                if (newc.Suit() == crd[winpos].Suit())
                    return newc > crd[winpos];
                else
                    return false;
            }
        }
    }

  public:
    bool CardWillWin(Card pc) const {
        if (trumps == suit::notrumps)
            return CardWillWinNT(pc);
        else
            return CardWillWinSuit(pc, trumps);
    }

    void PlayCard(Card pc) {
        if (cardsPlayed == 0) {
            Lead(pc);
        } else if (cardsPlayed < numPlayers) {
            if (CardWillWin(pc))
                wonByPlayer = OffsetPosition(leadPlayer, cardsPlayed);
            crd[cardsPlayed++] = pc;
        }
        assert(false); // unreachable
    }
};

bool CardIsValidFromHand(const Hand &h, const trick &t, int index) {
    assert(index >= 0);
    assert(index < CardsInSuit);
    const Card &cd = h.crd[index];
    assert(cd.IsValid());
    if (cd.CardHasPlayed())
        return false;

    int ptg = t.PlayersToGo();

    if (ptg == 0) {     // The trick has been played
        assert(false);  // unreachable
        return false;   // unreachable
    }
    if (ptg == cards::numPlayers) {  // You can lead any card
        return true;
    }
    suit cardS = cd.Suit();
    suit leadS = t.GetCardPlayed(0).Suit();
    if (cardS == leadS)
        return true;
    if (h.SuitLengthRemaining(leadS) == 0)
        return true;
    return false;
}

struct deal {
    std::array<Hand, cards::numPlayers> hands;
    std::vector<trick> tricks;
    contract contrct;
    vulnerability v;

    deal() {
        std::array<CardInt, CardsInDeck> a1;
        std::iota(a1.begin(), a1.end(), 0);
        std::random_device rd;
        std::mt19937 gen{rd()};
        std::ranges::shuffle(a1, gen);

        int i = 0;
        for (auto &h : hands) {
            for (auto &cd : h.crd) {
                cd.crd = a1[i];
                i++;
            }
            h.SetSuits();
        }
    }

    int GetTricksPlayed() const { return tricks.size(); }

    void SetVulnerability(vulnerability vc) { v = vc; }

    vulnerability GetVulnerability() const { return v; }

    deal(position dealer) : deal() { contrct.SetDealer(dealer); }

    void AddTrick(const cards::trick &t) {
        assert(t.PlayersToGo() == 0);
        for (int i = 0; i < cards::numPlayers; ++i) {
            cards::position p = static_cast<cards::position>(i);
            if (!hands[i].PlayCard(t.GetCard(p))) {
                assert(false);
                return;
            }
        }
        tricks.emplace_back(t);
        assert(tricks.size() < CardsInHand);
    }

    bool operator==(const cards::deal &d1) {
        for (int i = 0; i < cards::numPlayers; ++i) {
            if (hands[i] != d1.hands[i]) {
                return false;
            }
        }

        if (GetTricksPlayed() != d1.GetTricksPlayed())
            return false;
        return true;
    }
    std::string to_string() const {
        std::string out;
        position p{position::south};
        for (auto &&h : hands) {
            out += PositionToString(p++) + " " + std::to_string(h.PointCount());
            out += '\n';
            out += h.to_string();
            out += '\n';
        }
        return out;
    }
    std::string to_link() const {
        std::string lnk = "https://www.bridgebase.com/tools/"
                          "handviewer.html?lin=";
        lnk += "st||";
        lnk += "pn|~Msouth,~Mwest,~Mnorth,~Meast|";
        lnk += "md|";
        lnk += positiontolinkdealer(contrct.GetDealer());

        {
            int i = 0;
            for (const auto &h : hands) {
                lnk += h.to_link();
                if (++i < numPlayers)
                    lnk += ",";
            }
        }

        lnk += "|";

        lnk += "sv|" + vulnerabilitytoLink(v) + "|";

        lnk += "rh||";     // not sure what this is yet...
        lnk += "ah|deal|"; // hand description

        int i = contrct.bids.size();

        std::for_each(std::rbegin(contrct.bids), std::rend(contrct.bids), [&lnk, &i](const bid &b) {
            lnk += "mb|" + b.to_link() + "|an||";
            --i; // add descriptions when we have them..
        });

        for (const auto &t : tricks) {
            lnk += t.to_link();
        }

        return lnk;
    }
};
} // namespace cards
