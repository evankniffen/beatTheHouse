#include <QApplication>
#include <QWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFontDatabase>
#include <thread>
#include "game.cpp"  // import game logic

// ASCII art helpers
static QVector<QString> asciiCard(Rank r) {
    QString v = QString::fromStdString(rankToStr(r));
    if (v.size() == 1) v += ' ';
    return {
        "┌─────┐",
        QString("│%1   │").arg(v),
        "│     │",
        QString("│   %1│").arg(v),
        "└─────┘"
    };
}
static QString drawHand(const vector<Rank>& hand) {
    QVector<QVector<QString>> arts;
    for (auto c : hand) arts.append(asciiCard(c));
    QString result;
    for (int row = 0; row < 5; ++row) {
        for (auto &card : arts) {
            result += card[row] + ' ';
        }
        result += '\n';
    }
    return result;
}

class BlackjackUI : public QWidget {
    Q_OBJECT
public:
    BlackjackUI(QWidget* parent = nullptr)
      : QWidget(parent), capital(100.0), bet(10.0), you("You", capital, bet), dealer("Dealer", 0, 0)
    {
        textEdit = new QTextEdit;
        textEdit->setReadOnly(true);
        textEdit->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

        hitBtn = new QPushButton("Hit");
        standBtn = new QPushButton("Stand");
        doubleBtn = new QPushButton("Double");
        splitBtn = new QPushButton("Split");
        newGameBtn = new QPushButton("New Game");
        quitBtn = new QPushButton("Quit");

        QHBoxLayout* btnLayout = new QHBoxLayout;
        btnLayout->addWidget(hitBtn);
        btnLayout->addWidget(standBtn);
        btnLayout->addWidget(doubleBtn);
        btnLayout->addWidget(splitBtn);

        QHBoxLayout* bottomLayout = new QHBoxLayout;
        bottomLayout->addWidget(newGameBtn);
        bottomLayout->addWidget(quitBtn);

        QVBoxLayout* mainLayout = new QVBoxLayout;
        mainLayout->addWidget(textEdit);
        mainLayout->addLayout(btnLayout);
        mainLayout->addLayout(bottomLayout);
        setLayout(mainLayout);
        setWindowTitle("Blackjack");
        resize(600, 400);

        connect(hitBtn, &QPushButton::clicked, this, &BlackjackUI::onHit);
        connect(standBtn, &QPushButton::clicked, this, &BlackjackUI::onStand);
        connect(doubleBtn, &QPushButton::clicked, this, &BlackjackUI::onDouble);
        connect(splitBtn, &QPushButton::clicked, this, &BlackjackUI::onSplit);
        connect(newGameBtn, &QPushButton::clicked, this, &BlackjackUI::onNewGame);
        connect(quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);

        onNewGame();
    }

private slots:
    void onNewGame() {
        bool ok;
        double b = QInputDialog::getDouble(this, "Place Bet", "Bet:", bet, 1, capital, 1, &ok);
        if (!ok) return;
        bet = b;
        rebuild_deck(); shuffle_deck(deck);
        you = player("You", capital, bet);
        dealer = player("Dealer", 0, 0);
        dealer.hit(); dealer.hit();
        gameOver = false;
        updateUI();
    }

    void onHit() {
        if (gameOver) return;
        you.hit();
        if (you.isBust()) finalize(); else updateUI();
    }
    void onStand() {
        if (gameOver) return;
        finalize();
    }
    void onDouble() {
        if (gameOver) return;
        if (you.hand.size() == 2) {
            you.bet *= 2;
            you.hit();
            finalize();
        }
    }
    void onSplit() {
        // split not implemented in GUI
    }

private:
    void finalize() {
        gameOver = true;
        // dealer plays
        while (dealer.evaluate() < 17) dealer.hit();
        // settle
        you.settle(dealer);
        capital = you.capital;
        updateUI();
    }

    void updateUI() {
        QString s;
        s += "Dealer:\n" + drawHand(dealer.hand) + "\n";
        s += "You:    \n" + drawHand(you.hand) + "\n";
        s += QString("Your capital: $%1  Bet: $%2\n").arg(capital).arg(you.bet);
        textEdit->setPlainText(s);
    }

    QTextEdit* textEdit;
    QPushButton* hitBtn;
    QPushButton* standBtn;
    QPushButton* doubleBtn;
    QPushButton* splitBtn;
    QPushButton* newGameBtn;
    QPushButton* quitBtn;

    double capital;
    double bet;
    player you;
    player dealer;
    bool gameOver;
};

#include "ascii_ui.moc"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    BlackjackUI w;
    w.show();
    return app.exec();
}
