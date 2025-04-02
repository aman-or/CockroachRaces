#include "RaceManager.h"
#include "MainWindow.h"
#include "Player.h"
#include "Cockroach.h"
#include "./ui_MainWindow.h"

#include <random>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>


using std::srand;
using std::time;
using std::rand;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;
using std::vector;


RaceManager::RaceManager(MainWindow *mainWindow) : QObject(mainWindow), mainWindow(mainWindow) {
}

RaceManager::RaceManager(QObject* parent) : QObject(parent) {
}

void RaceManager::processNextPlayer() {
    if (mainWindow->currentPlayer < mainWindow->totalPlayers) {
        mainWindow->ui->enterAnswer->setVisible(true);
        mainWindow->ui->l_player_name->setText(QString("Введите имя игрока %1").arg(mainWindow->currentPlayer + 1));
        mainWindow->ui->l_player_name->setVisible(true);
        mainWindow->ui->player_name->setVisible(true);
        mainWindow->ui->player_name->clear();
        mainWindow->ui->chooseCockroach->setVisible(false);
    } else {
        mainWindow->currentPlayer = 0;
        processCockroachSelection();
    }
}

void RaceManager::processCockroachSelection() {
    mainWindow->updateRaceInfoUI();
    if (mainWindow->currentPlayer < mainWindow->totalPlayers) {
        mainWindow->displayCockroaches(true);
        mainWindow->ui->enterAnswer->setVisible(true);
        mainWindow->ui->l_player_name->setText(
            QString("Игрок %1, выберите таракана").arg(players[mainWindow->currentPlayer]->getName()));
        mainWindow->ui->player_name->setVisible(false);
        mainWindow->ui->chooseCockroach->setVisible(true);
    } else {
        mainWindow->ui->l_player_name->setText("Все игроки выбрали тараканов. Делайте ставки!");
        mainWindow->ui->chooseCockroach->setVisible(false);
        mainWindow->ui->enterAnswer->setVisible(false);
        mainWindow->displayCockroaches(false);
        mainWindow->displayBetFields();
    }
}

void RaceManager::onEnterAnswerClicked() {
    if (mainWindow->currentPlayer < players.size()) {
        int cockroachId = mainWindow->ui->chooseCockroach->currentIndex();
        if (cockroachId < 0 || cockroachId >= cockroaches.size()) {
            QMessageBox::warning(mainWindow, "Ошибка", "Некорректный выбор таракана!");
            return;
        }

        players[mainWindow->currentPlayer]->setCockroach(cockroaches[cockroachId]);
        qDebug() << "Игрок" << players[mainWindow->currentPlayer]->getName()
                 << "выбрал таракана:" << cockroaches[cockroachId]->getName();
        mainWindow->currentPlayer++;

        processCockroachSelection();
    } else {
        QString playerName = mainWindow->ui->player_name->text().trimmed();
        if (playerName.isEmpty()) {
            QMessageBox::warning(mainWindow, "Ошибка", "Имя игрока не может быть пустым!");
            return;
        }

        for (const auto& player : players) {
            if (player->getName().compare(playerName, Qt::CaseInsensitive) == 0) {
                QMessageBox::warning(mainWindow, "Ошибка", "Игрок с таким именем уже существует! Введите другое имя.");
                return;
            }
        }

        Player* newPlayer = new Player(playerName);
        players.append(newPlayer);
        mainWindow->currentPlayer++;

        processNextPlayer();
    }
}

void RaceManager::collectBets() {
    playerBets.clear();

    for (int i = 0; i < players.size(); ++i) {
        QPointer<Player> player = players.at(i);
        if (player) {
            QString playerBetField = QString("playerSum_%1").arg(i + 1);
            int bet = mainWindow->findChild<QLineEdit*>(playerBetField)->text().toInt();
            playerBets[player] = bet;
        }
    }
}

void RaceManager::onPlaceBetClicked() {
    collectBets();

    if (playerBets.isEmpty()) {
        QMessageBox::warning(mainWindow, "Ошибка", "Никто не сделал ставку!");
        return;
    }

    QString confirmationMessage = "Ставки сделаны:\n";
    bool allBetsValid = true;

    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        QPointer<Player> player = it.key();
        int betAmount = it.value();

        if (player) {
            if (betAmount > player->getMoney()) {
                QMessageBox::warning(mainWindow, "Ошибка",
                                     QString("У игрока %1 недостаточно денег для этой ставки!").arg(player->getName()));
                allBetsValid = false;
                break;
            }
            if (betAmount <= 0) {
                QMessageBox::warning(mainWindow, "Ошибка",
                                     QString("Ставка игрока %1 не может быть пустой или нулевой!").arg(player->getName()));
                allBetsValid = false;
                break;
            }
        }
    }

    if (!allBetsValid) {
        return;
    }

    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        QPointer<Player> player = it.key();
        int betAmount = it.value();

        if (player) {
            player->setBet(betAmount);
            player->decrementMoney(betAmount);
            confirmationMessage += QString("%1: %2$\n").arg(player->getName()).arg(player->getBet());
        }
    }

    QMessageBox::information(mainWindow, "Подтверждение", confirmationMessage);
    mainWindow->initializeCockroachPositions();
    mainWindow->ui->placeBetButton->setVisible(false);
    mainWindow->ui->playerSum_1->setVisible(false);
    mainWindow->ui->playerSum_2->setVisible(false);
    mainWindow->ui->playerSum_3->setVisible(false);
    mainWindow->ui->playerSum_4->setVisible(false);
    mainWindow->ui->playerSum_5->setVisible(false);
    mainWindow->ui->playerName_1->setVisible(false);
    mainWindow->ui->playerName_2->setVisible(false);
    mainWindow->ui->playerName_3->setVisible(false);
    mainWindow->ui->playerName_4->setVisible(false);
    mainWindow->ui->playerName_5->setVisible(false);
    mainWindow->ui->start->setVisible(true); // Показываем кнопку "Старт"

    QList<QLineEdit*> sumFields = {
        mainWindow->ui->playerSum_1, mainWindow->ui->playerSum_2, mainWindow->ui->playerSum_3,
        mainWindow->ui->playerSum_4, mainWindow->ui->playerSum_5
    };
    for (int i = 0; i < players.size(); ++i) {
        sumFields[i]->clear();
    }
}


void RaceManager::moveImages() {
    mainWindow->hideAllWidgets();
    mainWindow->clearSelection();
    mainWindow->ui->lcdTimer->setVisible(true);

    mainWindow->ui->image1->setVisible(true);
    mainWindow->ui->image2->setVisible(true);
    mainWindow->ui->image3->setVisible(true);
    mainWindow->ui->image4->setVisible(true);
    mainWindow->ui->image5->setVisible(true);
    mainWindow->ui->betTable->setVisible(true);

    mainWindow->ui->textBrowser->setVisible(true);

    random_device rd;
    mt19937 gen(rd());

    uniform_int_distribution<int> speedDist(2, 20);
    uniform_int_distribution<int> yDist(-5, 5);
    uniform_int_distribution<int> boostChance(1, 100);
    uniform_int_distribution<int> boostValue(10, 30);

    int textBrowserRightEdge = mainWindow->ui->textBrowser->x() + mainWindow->ui->textBrowser->width();
    bool stop = false;
    updateBetTable();

    for (size_t i = 0; i < cockroaches.size(); ++i) {
        QPixmap pixmap(cockroaches[i]->getImagePath());
        mainWindow->imageWidgets[i]->setPixmap(pixmap.scaled(mainWindow->imageWidgets[i]->size(), Qt::KeepAspectRatio));

        QPoint currentPos = cockroaches[i]->getPosition();
        int moveX = speedDist(gen);

        if (boostChance(gen) <= 10) {
            moveX += boostValue(gen);
        }

        int newX = currentPos.x() + moveX;
        int newY = currentPos.y() + yDist(gen);

        cockroaches[i]->setPosition(newX, newY);
         mainWindow->imageWidgets[i]->move(newX, newY);

        if (newX +  mainWindow->imageWidgets[i]->width() >= textBrowserRightEdge) { // Условие финиша (800 - примерное значение)
            stop = true;
        }
    }

    if (stop) {
        mainWindow->timer->stop();
        checkWinner();
        resetBets();
        updateBetTable();
    }
}

void RaceManager::checkWinner() {
    int maxX = 0;
    Cockroach* winner = nullptr;

    for (Cockroach* cockroach : cockroaches) {
        cockroach->incrementRaceCount();
        if (cockroach->getPosition().x() > maxX) {
            maxX = cockroach->getPosition().x();
            winner = cockroach;
        }
    }

    if (!winner) return;

    QMessageBox::information(nullptr, "Гонка окончена!",
                             QString("Победил таракан: %1!").arg(winner->getName()));

    winner->incrementWinCount();
    distributeWinnings(winner);

    resetBets();
    updateBetTable();

    mainWindow->onRaceFinished(); // Показываем кнопки "Продолжить" и "Завершить"
}

void RaceManager::distributeWinnings(Cockroach* winner) {
    int totalBet = 0;
    int winnerBetSum = 0;
    double bonusMultiplier = 1.0;

    qDebug() << "=== Начало распределения выигрыша ===";
    qDebug() << "Победивший таракан: " << winner->getName();

    // Подсчитываем общий банк
    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        Player* player = it.key();
        int bet = it.value();
        totalBet += bet;
        qDebug() << "Игрок" << player->getName() << "поставил" << bet << "на таракана"
                 << (player->getCockroach() ? player->getCockroach()->getName() : "NULL");
    }
    // Добавляем джекпот к общему банку
    totalBet += jackpot;
    jackpot = 0; // Обнуляем джекпот после его разыгрыша

    qDebug() << "Общий банк (с учетом джекпота): " << totalBet;

    // Сбрасываем сумму ставок на победителя перед пересчетом
    winnerBetSum = 0;  // ← Убираем повторное объявление, просто присваиваем 0

    // Подсчитываем сумму ставок на победителя
    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        if (it.key()->getCockroach() == winner) {
            winnerBetSum += it.value();
        }
    }

    qDebug() << "Общий банк: " << totalBet;
    qDebug() << "Общая сумма ставок на победителя: " << winnerBetSum;

    if (winnerBetSum == 0) {
        qDebug() << "Никто не поставил на победителя! 50% банка переходит в следующий раунд.";
        jackpot += totalBet / 2;
        return;
    }

    // Если только один игрок поставил на победителя, он получает бонус
    int winnerCount = 0;
    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        if (it.key()->getCockroach() == winner) {
            winnerCount++;
        }
    }
    if (winnerCount == 1) {
        bonusMultiplier = 1.5; // Бонус за редкую ставку
    }

    // Распределяем выигрыши
    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        Player* player = it.key();
        int bet = it.value();

        if (player->getCockroach() == winner) {
            double payout = static_cast<double>(bet) / winnerBetSum * totalBet * bonusMultiplier;
            int winnings = static_cast<int>(payout + 0.5);

            qDebug() << "Игрок" << player->getName() << "получает" << winnings << " (с бонусом " << bonusMultiplier << ")";

            player->incrementMoney(winnings);
            player->incrementWins();
        } else {
            // Утешительный приз за участие
            int consolationPrize = bet / 10;
            player->incrementMoney(consolationPrize);
            qDebug() << "Игрок" << player->getName() << "получает утешительный приз: " << consolationPrize;
        }
    }

    playerBets.clear(); // Очищаем ставки
    updateBetTable();
}

void RaceManager::resetBets() {
    for (auto it = playerBets.begin(); it != playerBets.end(); ++it) {
        it.value() = 0;
    }
}

void RaceManager::updateBetTable() {
    mainWindow->ui->betTable->clearContents(); // Очищаем содержимое таблицы
    mainWindow->ui->betTable->setRowCount(players.size()); // Устанавливаем количество строк

    int row = 0;
    for (const auto& player : players) {
        int money = player->getMoney();
        int bet = playerBets.value(player, 0); // Получаем ставку игрока (если есть)

        QTableWidgetItem* nameItem = new QTableWidgetItem(player->getName());
        QTableWidgetItem* moneyItem = new QTableWidgetItem(QString::number(money));
        QTableWidgetItem* betItem = new QTableWidgetItem(QString::number(bet));

        // Запрещаем редактирование ячеек
        nameItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        moneyItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        betItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

        mainWindow->ui->betTable->setItem(row, 0, nameItem);
        mainWindow->ui->betTable->setItem(row, 1, moneyItem);
        mainWindow->ui->betTable->setItem(row, 2, betItem);

        row++;
    }

    mainWindow->ui->betTable->update(); // Принудительно обновляем таблицу
}

