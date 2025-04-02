#ifndef RACEMANAGER_H
#define RACEMANAGER_H

#include "Cockroach.h"
#include "Player.h"
#include "MainWindow.h"

#include <QObject>
#include <QVector>
#include <QPointer>


class MainWindow;

class RaceManager : public QObject
{
    Q_OBJECT
    friend class MainWindow;

public:
    explicit RaceManager(MainWindow *mainWindow);
    explicit RaceManager(QObject* parent = nullptr); // Второй конструктор
    void processNextPlayer();
    void processCockroachSelection();
    void onEnterAnswerClicked();
    void collectBets();
    void onPlaceBetClicked();
    void moveImages();
    void checkWinner();
    void distributeWinnings(Cockroach* winner);
    void resetBets();


private:
    MainWindow *mainWindow;
    QVector<QPointer<Cockroach>> cockroaches;  // Тараканы
    QList<QLabel*> imageWidgets;
    QMap<Player*, int> playerBets;
    QList<Player*> players;   // Игроки
    int jackpot = 0;
    void updateBetTable();
};

#endif // RACEMANAGER_H
