#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Player.h"
#include "Cockroach.h"

#include <QMainWindow>
#include <QVector>
#include <QPointer>
#include <QLabel>
#include <QLCDNumber>
#include <QtCore>
#include <QtGui>
#include <QtWidgets>

class RaceManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class RaceManager;

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onStartGame();
    void onPlayerCountEntered();
    void onCockroachSelected(int cockroachId);  // Выбор таракана игроком
    void startRace();            // Начало гонки
    void stopRace();
    void addPlayer(Player* newPlayer);
    void updateTimer();
    void onRaceFinished();
    void resetGame();
    void onContinueClicked();
    void onFinishClicked();
    void resetRaceData();
    void initializeCockroachPositions();

private:
    Ui::MainWindow *ui;
    QLCDNumber *lcdTimer;
    QTimer *timer;
    QList<QLabel*> imageWidgets;
    int elapsedTime;  // Время в секундах
    int currentPlayer;   // Текущий игрок
    int totalPlayers;    // Общее количество игроков
    void displayCockroaches(bool check_tf);   // Отображение списка тараканов
    void displayBetFields();     // Отображение полей для ставок
    void hideAllWidgets();       // Скрытие всех виджетов
    void clearSelection();
    void saveCockroachesToFile();
    void loadCockroachesFromFile();
    void updateRaceInfoUI();

    // разделение по файлам
    RaceManager *raceManager;

};

#endif // MAINWINDOW_H
