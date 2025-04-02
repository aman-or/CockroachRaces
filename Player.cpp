#include "Player.h"

Player::Player(const QString &name, int money)
    : name(name), money(money), wins(0), chosenCockroach(nullptr) {}

QString Player::getName() const { return name; }
int Player::getMoney() const { return money; }
int Player::getWins() const { return wins; }
QPointer<Cockroach> Player::getCockroach() const { return chosenCockroach; }

void Player::setCockroach(Cockroach *cockroach) {
    chosenCockroach = cockroach;
}

void Player::incrementWins() {
    wins++;
}

void Player::decrementMoney(int amount) {
    money -= amount;
}

void Player::incrementMoney(int amount) {
    money += amount;
}

void Player::reset() {
    currentBet = 0;      // Обнуляем ставку
    chosenCockroach = nullptr; // Сбрасываем выбранного таракана

}
