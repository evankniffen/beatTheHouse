import random
import matplotlib.pyplot as plt
import numpy as np
import math 

winnings = 0
deck = []
splits = 0
splitTest = False
wins = 0
losses = 0
pushes = 0
hardArray = [
    ['hit']*10,
    ['hit']*10,
    ['hit']*10,
    ['hit']*10,
    ['hit']*10,
    ['hit', 'double', 'double', 'double', 'double', 'hit', 'hit', 'hit', 'hit', 'hit'],
    ['double']*8 + ['hit', 'hit'],
    ['double']*10,
    ['hit', 'hit', 'stand', 'stand', 'stand', 'hit', 'hit', 'hit', 'hit', 'hit'],
    ['stand']*5 + ['hit']*5,
    ['stand']*5 + ['hit']*5,
    ['stand']*5 + ['hit']*5,
    ['stand']*5 + ['hit']*5,
    ['stand']*10,
    ['stand']*10,
    ['stand']*10,
    ['stand']*10,
    ['stand']*10
]

softArray = [
    ['hit']*10,
    ['hit']*5 + ['double'] + ['hit']*4,
    ['hit']*3 + ['double']*2 + ['hit']*5,
    ['hit']*3 + ['double']*2 + ['hit']*5,
    ['hit']*2 + ['double']*3 + ['hit']*5,
    ['hit'] + ['double']*4 + ['hit']*5,
    ['stand'] + ['double']*4 + ['stand']*2 + ['hit']*3,
    ['stand']*10,
    ['stand']*10,
    ['stand']*10
]

splitArray = [
    # Dealer's upcard: 2  3  4  5  6  7  8  9  10 A
    ['split']*10,  # Pair of Aces
    ['hit']*2 + ['split']*4 + ['hit']*4,  # Pair of 2s
    ['hit']*2 + ['split']*4 + ['hit']*4,  # Pair of 3s
    ['hit']*10,  # Pair of 4s
    ['double']*8 + ['hit']*2,  # Pair of 5s
    ['split']*5 + ['hit']*5,  # Pair of 6s
    ['split']*6 + ['hit']*4,  # Pair of 7s
    ['split']*10,  # Pair of 8s
    ['split']*5+['stand']+['split']*2 + ['stand']*2,  # Pair of 9s
    ['stand']*10,  # Pair of 10s (never split 10s)
]

def initialize_game():
    global turn, playerHand, dealerHand, winner, deck, mult, splitBets
    winnings = 0
    mult = 1
    playerHand = []
    dealerHand = []
    winner = None
    turn = True
    deck = [10] * 16 + ['A'] * 4 + [i for i in range(2, 10)] * 4
    random.shuffle(deck)
    splitBets = []

def double():
    global bet, turn, winner
    if len(playerHand) == 2:
        bet *= 2
        #print("Player doubles. New bet is:", bet)
        hit(True)
        if winner is not None:
            return
        stand()
    else:
        #print("Would double w/ 2, forced to hit")
        hit(True)

def stand():
    global winner, turn
    if not turn or winner is not None:
        return
    #print("Player stands.", playerHand)
    turn = False
    dealer_actions()

def dealer_actions():
    global winner
    #print("Dealer's turn begins.")
    while handSum(dealerHand) < 17:
        hit(False)
    if winner is None:
        if handSum(dealerHand) == 21:
            bj(False)
        elif handSum(dealerHand) > 21:
            bust(False)
        else:
            evaluate_winner()

def bj(turnBool):
    global winner, mult, winnings, wins, losses
    if turnBool and handSum(playerHand) == 21:
        mult = 1.5
        winner = True
        #print("Player hits blackjack!", playerHand)
        if splitTest:
            mult = 1
        winnings += mult * bet
        wins += 1


    elif not turnBool and handSum(dealerHand) == 21:
        winner = False
        #print("Dealer hits blackjack!", dealerHand)
        winnings -= bet
        losses += 1

def bust(turnBool):
    global winner, winnings, wins, losses
    if winner is None:
        if turnBool:
            #print("Player busts.")
            winner = False
            winnings -= bet
            losses += 1

        else:
            #print("Dealer busts.")
            winner = True
            winnings += mult * bet
            wins += 1

def hit(turnBool):
    global winner, deck
    if not deck:
        deck = [10] * 16 + ['A'] * 4 + [i for i in range(2, 10)] * 4
        random.shuffle(deck)
    card = deck.pop()
    if turnBool:
        playerHand.append(card)
        #print("Player hits and receives: ", card, "| ", playerHand)
        if handSum(playerHand) > 21:
            bust(True)
    else:
        dealerHand.append(card)
        #print("Dealer hits and receives: ", card, "| ", dealerHand)
        if handSum(dealerHand) > 21:
            bust(False)

def handSum(hand):
    total = sum(card if isinstance(card, int) else 0 for card in hand)
    aces = hand.count('A')
    for _ in range(aces):
        if total + 11 <= 21:
            total += 11
        else:
            total += 1
    return total

def evaluate_winner():
    global winner, winnings, wins, losses, pushes
    if handSum(playerHand) > handSum(dealerHand):
        winner = True
        ###print("Player wins with a hand of", handSum(playerHand))
        winnings += bet * mult
        wins += 1
    elif handSum(playerHand) < handSum(dealerHand):
        winner = False
        ###print("Dealer wins with a hand of", handSum(dealerHand))
        winnings -= bet
        losses += 1
    else:
        winner = "Push"
        pushes += 1
        ###print("The game is a push.")

def split():
    global playerHand, deck, bet, splits, splitBets, splitTest
    
    if len(playerHand) == 2 and playerHand[0] == playerHand[1]:
        ###print("Player splits the pair.")
        splits += 1
        splitTest = True
        splitBets.append(bet)
        hand1 = [playerHand[0], deck.pop()]
        hand2 = [playerHand[1], deck.pop()]
        play_hand(hand1)
        play_hand(hand2)
    else:
        return
        #print("Cannot split: Not a pair or not the initial hand.")

def player_turn():
    global turn, winner, playerHand
    while turn and winner is None:
        dealer_index = -1 if dealerHand[0] == 'A' else dealerHand[0] - 2
        player_total = handSum(playerHand)

        if len(playerHand) == 2 and playerHand[0] == playerHand[1]:
            pair_rank = playerHand[0]
            if pair_rank == 'A':
                action = splitArray[0][dealer_index]
            else:
                action = splitArray[pair_rank - 1][dealer_index]

            if action == 'split':
                split()
                return

        if 'A' in playerHand and player_total - 11 <= 10:
            action = softArray[player_total - 12][dealer_index]
        else:
            action = hardArray[player_total - 4][dealer_index]

        if action == 'hit':
            hit(True)
        elif action == 'double':
            double()
        elif action == 'stand':
            stand()

        if handSum(playerHand) >= 21:
            break

def play():
    global winnings, mult, bet, winner, playerHand, dealerHand
    initialize_game()
    playerHand.extend([deck.pop() for _ in range(2)])
    dealerHand.extend([deck.pop() for _ in range(2)])

    ##print("Player's hand:", playerHand)
    ##print("Dealer's first card:", dealerHand[0])

    if handSum(playerHand) == 21:
        bj(True)

    if winner is None:
        player_turn()

    if winner is None:
        dealer_actions()

    ##print("Winnings after this game:", winnings)

def play_hand(hand):
    global playerHand, winner, bet, mult, turn
    playerHand = hand
    winner = None
    bet = 1
    mult = 1
    turn = True
    ##print("Split hand:", playerHand)
    player_turn()

def start_game(bet_amount):
    global bet, splitTest
    bet = bet_amount
    splitTest = False
    play()
    ##print("Cumulative Winnings:", winnings)
    ##print("Winner:", winner)
    ##print("Player Hand:", playerHand)
    ##print("Dealer Hand:", dealerHand)
    ##print()

def multiple_runs(n,sims, bet_amount):
    global winnings
    cumulative_winnings = []
    cumulative_winnings_array = np.zeros((sims,n))
    final_winnings = np.zeros(sims)
    max_winnings = -math.inf
    max_index = 0
    for i in range(sims):
        for j in range(1,n):
            start_game(bet_amount)
            cumulative_winnings.append(winnings)
            cumulative_winnings_array[i,j] = winnings
        if winnings > max_winnings:
            max_winnings = winnings
            max_index = i
        final_winnings[i] = winnings
        winnings = 0

    total_games = wins + losses + pushes
    win_percentage = (wins / total_games) * 100 if total_games > 0 else 0
    loss_percentage = (losses / total_games) * 100 if total_games > 0 else 0
    push_percentage = (pushes / total_games) * 100 if total_games > 0 else 0
    split_percentage = (splits / (n*sims)) * 100 if total_games > 0 else 0

    print("\nFinal Results:")
    print(f"Total Games Played: {n*sims} | Games including splits: {total_games}")
    print(f"Wins: {wins} ({win_percentage:.5f}%)")
    print(f"Losses: {losses} ({loss_percentage:.5f}%)") 
    print(f"Pushes: {pushes} ({push_percentage:.5f}%)")
    print(f"Splits: {splits} ({split_percentage:.5f}%)")
    #print(f"Final Total Winnings: {winnings}")

    plt.subplot(1, 2, 1)
    for i in range(sims):
        if i != max_index:
            plt.plot(range(1,n+1), cumulative_winnings_array[i], linewidth=1, alpha=0.5)
        else:
            plt.plot(range(1,n+1), cumulative_winnings_array[i], linewidth=3, alpha=0.5, zorder=10)

    plt.suptitle(f"Cumulative Winnings Over Time (Sims: {sims})")
    plt.xlabel("Game Number")
    plt.ylabel("Cumulative Winnings")
    plt.title( f"Max run: {max_winnings}")
    plt.grid(True)

    plt.subplot(1, 2, 2)
    plt.hist(final_winnings, bins=20, edgecolor='black', alpha=0.7, density=True)
    plt.title(f"(Max, Mean, Min) = ({max_winnings},{np.mean(final_winnings)},{np.min(final_winnings)})")
    plt.xlabel("Winnings")
    plt.ylabel("Probability Density")
    plt.grid(True)

    plt.tight_layout()
    plt.show()



multiple_runs(1000,10,1)



