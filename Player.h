#ifndef PLAYER_H
#define PLAYER_H

#include <QString>
#include <QObject>
#include <QPointer>
#include "Cockroach.h"

class Player : public QObject {
    Q_OBJECT

public:
    Player(QObject *parent = nullptr) : QObject(parent) {}

    Player(const QString &name, int money = 1000);

    QString getName() const;
    int getMoney() const;
    int getWins() const;
    QPointer<Cockroach> getCockroach() const;
    void setCockroach(Cockroach *cockroach);
    void incrementWins();
    void decrementMoney(int amount);
    void incrementMoney(int amount);
    void setBet(int amount) { currentBet = amount; }
    int getBet() const { return currentBet; }

    void reset(); // Добавим объявление метода


private:
    QString name;             // Имя игрока
    int money;                // Деньги игрока
    int wins;                 // Количество побед
    int currentBet = 0; // Ставка игрока
    QPointer<Cockroach> chosenCockroach; // Выбранный таракан
};

#endif // PLAYER_H
