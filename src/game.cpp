#include <algorithm>
#include <random>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

using namespace std;

// ─── Card Definitions ──────────────────────────────────────────────────────────
enum Rank {
    ACE   = 1,
    TWO,
    THREE,
    FOUR,
    FIVE,
    SIX,
    SEVEN,
    EIGHT,
    NINE,
    TEN,
    JACK,
    QUEEN,
    KING
};

auto pipValue = [](Rank r) {
    if (r == ACE)    return 1;
    if (r >= TEN)    return 10;
    return int(r);
};

string rankToStr(Rank r) {
    switch (r) {
        case ACE:   return "A";
        case TWO:   return "2";
        case THREE: return "3";
        case FOUR:  return "4";
        case FIVE:  return "5";
        case SIX:   return "6";
        case SEVEN: return "7";
        case EIGHT: return "8";
        case NINE:  return "9";
        case TEN:   return "10";
        case JACK:  return "J";
        case QUEEN: return "Q";
        case KING:  return "K";
    }
    return "?";
}

static vector<Rank> deck;

static mt19937 rng{
    static_cast<unsigned>(
        chrono::high_resolution_clock::now()
            .time_since_epoch().count()
    )
};

void shuffle_deck(vector<Rank>& d) {
    shuffle(d.begin(), d.end(), rng);
}

void rebuild_deck() {
    deck.clear();
    for (int s = 0; s < 4; ++s) {
        for (int r = ACE; r <= KING; ++r) {
            deck.push_back(static_cast<Rank>(r));
        }
    }
}

// ─── Player / Dealer ──────────────────────────────────────────────────────────
struct player {
    string       name;
    double       capital;
    double       bet;
    vector<Rank> hand;
    bool         turn = true;

    player(string n, double cap, double b)
      : name(n), capital(cap), bet(b) {
        hit(); hit();
    }

    player(string n, double cap, double b, vector<Rank> h)
      : name(n), capital(cap), bet(b), hand(move(h)) {}

    void hit() {
        hand.push_back(deck.back()); deck.pop_back();
    }
    void stand() { turn = false; }

    int evaluate() const {
        int total = 0, aces = 0;
        for (auto c : hand) {
            if (c == ACE) ++aces;
            else          total += pipValue(c);
        }
        total += aces * 11;
        while (total > 21 && aces-- > 0) total -= 10;
        return total;
    }
    bool isBlackjack() const { return hand.size()==2 && evaluate()==21; }
    bool isBust()      const { return evaluate()>21; }

    static void dealerPlay(player& dealer) {
        while (dealer.evaluate() < 17) dealer.hit();
        dealer.stand();
    }

    void settle(const player& dealer) {
        cout << "\n--- Settling " << name << " ---\n";
        if (isBlackjack() && !dealer.isBlackjack()) {
            cout << "→ Natural Blackjack! You win 3:2\n";
            capital += 1.5 * bet;
            return;
        }
        if (isBlackjack() && dealer.isBlackjack()) {
            cout << "→ Both Blackjack → push\n";
            return;
        }
        if (isBust()) {
            cout << "→ " << name << " busts. Lose $" << bet << "\n";
            capital -= bet;
            return;
        }
        if (dealer.isBust()) {
            cout << "→ Dealer busts. " << name << " wins $" << bet << "\n";
            capital += bet;
            return;
        }
        int me = evaluate(), him = dealer.evaluate();
        cout << "→ " << name << ": " << me
             << " vs Dealer: " << him;
        if (me>him) {
            cout << " → win $" << bet << "\n";
            capital += bet;
        } else if (me<him) {
            cout << " → lose $" << bet << "\n";
            capital -= bet;
        } else {
            cout << " → push\n";
        }
    }

    void playHand() {
        cout << "\n" << name << "'s Hand: ";
        for (auto c:hand) cout << "["<<rankToStr(c)<<"] ";
        cout << "(Total="<<evaluate()<<")\n";

        if (isBlackjack()) {
            cout << "→ Natural Blackjack!\n";
            turn = false;
            return;
        }

        bool firstTurn = true;
        while (turn && !isBust()) {
            cout << "Choose ";
            cout << "(h)it, (s)tand";
            if (firstTurn) cout << ", (d)ouble";
            cout << ": ";
            string m; cin >> m;
            if (m=="h") {
                hit();
                cout << "→ Drew ["<<rankToStr(hand.back())<<"] Total="
                     <<evaluate()<<"\n";
            }
            else if (m=="s") {
                stand();
                cout << "→ "<<name<<" stands at "<<evaluate()<<"\n";
            }
            else if (m=="d" && firstTurn) {
                bet*=2; hit(); stand();
                cout << "→ Double down! Drew ["<<rankToStr(hand.back())
                     <<"] Total="<<evaluate()<<" Bet=$"<<bet<<"\n";
            }
            else {
                cout << "Invalid choice.\n";
                continue;
            }
            firstTurn = false;
        }
        if (isBust()) cout << "→ "<<name<<" busts at "<<evaluate()<<"\n";
    }
};

int main() {
    double playerCapital = 100.0;
    while (true) {
        if (playerCapital<=0) { cout<<"You are out of money. Game over.\n"; break; }
        cout<<"\nYour capital: $"<<playerCapital<<". Enter bet (or q to quit): ";
        string in; cin>>in;
        if (in=="q"||in=="Q") break;
        double bet; try { bet=stod(in); } catch(...) { cout<<"Invalid bet.\n"; continue; }
        if (bet<=0||bet>playerCapital) { cout<<"Bet must be >0 and ≤capital.\n"; continue; }
        rebuild_deck(); shuffle_deck(deck);
        player you("Evan", playerCapital, bet);
        player dealer("Dealer", 0,0);
        dealer.hit(); dealer.hit();
        cout<<"\nDealer shows: ["<<rankToStr(dealer.hand[1])<<"]\n";

        bool didSplit=false;
        if (pipValue(you.hand[0])==pipValue(you.hand[1]) && playerCapital>=bet*2) {
            cout<<"Split? (y/n): "; string a; cin>>a;
            if (a=="y"||a=="Y") {
                didSplit=true;
                vector<Rank> h1{you.hand[0]}, h2{you.hand[1]};
                h1.push_back(deck.back()); deck.pop_back();
                h2.push_back(deck.back()); deck.pop_back();
                double init=playerCapital;
                player p1("Hand 1",init,bet,h1);
                player p2("Hand 2",init,bet,h2);
                p1.playHand(); p2.playHand();
                player::dealerPlay(dealer);
                p1.settle(dealer); p2.settle(dealer);
                playerCapital=(p1.capital+p2.capital)-init;
                cout<<"Your capital after split: $"<<playerCapital<<"\n";
            }
        }
        if (!didSplit) {
            you.playHand();
            player::dealerPlay(dealer);
            you.settle(dealer);
            playerCapital=you.capital;
            cout<<"Your capital: $"<<playerCapital<<"\n";
        }
    }
    cout<<"\nThanks for playing! Final capital: $"<<playerCapital<<"\n";
    return 0;
}
