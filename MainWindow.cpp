#include "MainWindow.h"
#include "./ui_MainWindow.h"
#include "Cockroach.h"
#include "Player.h"
#include "RaceManager.h"

#include <QDebug>
#include <QPixmap>
#include <QTimer>
#include <QMessageBox>
#include <random>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>


// Файл для хранения данных
const QString DATA_FILE ="cockroaches.json";


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), timer(new QTimer(this)), elapsedTime(0)
{
    ui->setupUi(this);
    ui->textBrowser->setHtml("<img src=':/resources/img/field.png' width='990' height='485'>");

    raceManager = new RaceManager(this);


    loadCockroachesFromFile();  // Загружаем тараканов из файла

    // Если список тараканов пуст после загрузки, добавляем стандартных
    if (raceManager->cockroaches.isEmpty()) {
        raceManager->cockroaches.append(new Cockroach("Fast", ":/resources/img/first.png", 10, 210));
        raceManager->cockroaches.append(new Cockroach("Speed", ":/resources/img/second.png", 10, 310));
        raceManager->cockroaches.append(new Cockroach("Storm", ":/resources/img/third.gif", 20, 400));
        raceManager->cockroaches.append(new Cockroach("Lord", ":/resources/img/fourth.png", 0, 100));
        raceManager->cockroaches.append(new Cockroach("Leopard", ":/resources/img/fifth.png", 10, 490));
    }

    saveCockroachesToFile(); // Сохраняем список

    updateRaceInfoUI();

    QRegularExpressionValidator* nameValidator = new QRegularExpressionValidator(QRegularExpression("^[a-zA-Zа-яА-ЯёЁ]+$"), this);
    ui->player_name->setValidator(nameValidator);

    QIntValidator* betValidator = new QIntValidator(0, 9999999, this);  // Ограничение на 7 цифр
    ui->playerSum_1->setValidator(betValidator);
    ui->playerSum_2->setValidator(betValidator);
    ui->playerSum_3->setValidator(betValidator);
    ui->playerSum_4->setValidator(betValidator);
    ui->playerSum_5->setValidator(betValidator);

    // Настройка виджетов
    imageWidgets = {ui->image1, ui->image2, ui->image3, ui->image4, ui->image5};

    // Заполнение выпадающего списка тараканами
    QStringList cockroachNames;
    for (auto& cockroach : raceManager->cockroaches) {
        cockroachNames.append(cockroach->getName());
    }
    ui->chooseCockroach->addItems(cockroachNames);

    // Подключение сигналов и слотов  
    connect(ui->startGame, &QPushButton::clicked, this, &MainWindow::onStartGame);
    connect(ui->enterCount, &QPushButton::clicked, this, &MainWindow::onPlayerCountEntered);
    connect(ui->start, &QPushButton::clicked, [this]() {timer->start(50); });
    connect(ui->chooseCockroach, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCockroachSelected);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    connect(ui->continueButton, &QPushButton::clicked, this, &MainWindow::onContinueClicked);
    connect(ui->finishButton, &QPushButton::clicked, this, &MainWindow::onFinishClicked);

    connect(ui->enterAnswer, &QPushButton::clicked, raceManager, &RaceManager::onEnterAnswerClicked);
    connect(ui->placeBetButton, &QPushButton::clicked, raceManager, &RaceManager::onPlaceBetClicked);
    connect(timer, &QTimer::timeout, raceManager, &RaceManager::moveImages);

    // Скрытие всех виджетов по умолчанию
    hideAllWidgets();
}


MainWindow::~MainWindow() {
    delete ui;
    qDeleteAll(raceManager->cockroaches);  // Очистка памяти
    qDeleteAll(raceManager->players);
    raceManager->players.clear();

}

void MainWindow::updateRaceInfoUI() {
    QLineEdit* winFields[] = {ui->win_1, ui->win_2, ui->win_3, ui->win_4, ui->win_5};
    QLineEdit* raceFields[] = {ui->races_1, ui->races_2, ui->races_3, ui->races_4, ui->races_5};

    for (int i = 0; i < raceManager->cockroaches.size() && i < 5; ++i) {
        winFields[i]->setText(QString::number(raceManager->cockroaches[i]->getWinCount()));
        raceFields[i]->setText(QString::number(raceManager->cockroaches[i]->getRaceCount()));

        winFields[i]->setReadOnly(true);
        raceFields[i]->setReadOnly(true);
    }
}

// Сохранение списка тараканов
void MainWindow::saveCockroachesToFile() {
    QJsonArray jsonArray;
    for (Cockroach* cockroach : raceManager->cockroaches) {
        jsonArray.append(cockroach->toJson());
    }

    QJsonDocument jsonDoc(jsonArray);
    QFile file(DATA_FILE);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Ошибка записи в файл:" << file.errorString();
        return;
    }

    if (file.write(jsonDoc.toJson()) == -1) {
        qWarning() << "Ошибка записи в файл!";
        return;
    }

    file.close();
    if (file.error() != QFile::NoError) {
        qWarning() << "Ошибка при закрытии файла!";
    }

    qDebug() << "Тараканы сохранены!";
}



// Загрузка списка тараканов
void MainWindow::loadCockroachesFromFile() {
    QFile file(DATA_FILE);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Файл не найден, создаем новый.";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (!jsonDoc.isArray()) {
        qWarning() << "Ошибка чтения JSON!";
        return;
    }

    QJsonArray jsonArray = jsonDoc.array();
    raceManager->cockroaches.clear();  // Очищаем список перед загрузкой новых данных

    for (const QJsonValue &value : jsonArray) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject obj = value.toObject();
        QString name = obj["name"].toString();
        int raceCount = obj["raceCount"].toInt();
        int winCount = obj["winCount"].toInt();

        // Устанавливаем путь к изображению и координаты по имени таракана
        QString imagePath;
        int x = 10, y = 100;

        if (name == "Fast") {
            imagePath = ":/resources/img/first.png";
            y = 210;
        } else if (name == "Speed") {
            imagePath = ":/resources/img/second.png";
            y = 310;
        } else if (name == "Storm") {
            imagePath = ":/resources/img/third.gif";
            y = 400;
        } else if (name == "Lord") {
            imagePath = ":/resources/img/fourth.png";
            y = 100;
        } else if (name == "Leopard") {
            imagePath = ":/resources/img/fifth.png";
            y = 490;
        } else {
            qWarning() << "Неизвестный таракан: " << name;
            continue;
        }

        // Создаем нового таракана
        Cockroach* newCockroach = new Cockroach(name, imagePath, x, y);

        // Восстанавливаем сохраненные данные
        newCockroach->setRaceCount(raceCount);
        newCockroach->setWinCount(winCount);

        raceManager->cockroaches.append(newCockroach);
    }

    qDebug() << "Загружено тараканов:" << raceManager->cockroaches.size();
}




void MainWindow::hideAllWidgets()
{
    ui->playerCount->setVisible(false);
    ui->start->setVisible(false);
    ui->enterAnswer->setVisible(false);
    ui->enterCount->setVisible(false);
    ui->textBrowser->setVisible(false);
    ui->chooseCockroach->setVisible(false);
    for (auto *image : imageWidgets) {
        image->setVisible(false);
    }
    for (QLabel *label : this->findChildren<QLabel*>()) {
        label->setVisible(false);
    }

    ui->player_name->setVisible(false);
    ui->playerName_1->setVisible(false);
    ui->playerName_2->setVisible(false);
    ui->playerName_3->setVisible(false);
    ui->playerName_4->setVisible(false);
    ui->playerName_5->setVisible(false);
    ui->playerSum_1->setVisible(false);
    ui->playerSum_2->setVisible(false);
    ui->playerSum_3->setVisible(false);
    ui->playerSum_4->setVisible(false);
    ui->playerSum_5->setVisible(false);

    ui->placeBetButton->setVisible(false);
    ui->betTable->setVisible(false);
    ui->lcdTimer->setVisible(false);

    ui->continueButton->setVisible(false);
    ui->finishButton->setVisible(false);
    displayCockroaches(false);

}

void MainWindow::displayCockroaches(bool check_tf) {
    ui->chooseCockroach->setVisible(check_tf); // Показываем выпадающий список для выбора таракана
    ui->image1->setVisible(check_tf);
    ui->image2->setVisible(check_tf);
    ui->image3->setVisible(check_tf);
    ui->image4->setVisible(check_tf);
    ui->image5->setVisible(check_tf);

    ui->cockroachName_1->setVisible(check_tf);
    ui->cockroachName_2->setVisible(check_tf);
    ui->cockroachName_3->setVisible(check_tf);
    ui->cockroachName_4->setVisible(check_tf);
    ui->cockroachName_5->setVisible(check_tf);


    ui->win_1->setVisible(check_tf);
    ui->win_2->setVisible(check_tf);
    ui->win_3->setVisible(check_tf);
    ui->win_4->setVisible(check_tf);
    ui->win_5->setVisible(check_tf);

    ui->races_1->setVisible(check_tf);
    ui->races_2->setVisible(check_tf);
    ui->races_3->setVisible(check_tf);
    ui->races_4->setVisible(check_tf);
    ui->races_5->setVisible(check_tf);

    ui->name->setVisible(check_tf);
    ui->races->setVisible(check_tf);
    ui->win->setVisible(check_tf);


}


void MainWindow::onStartGame()
{
    ui->startGame->setVisible(false);
    ui->l_count->setVisible(true);
    ui->playerCount->setVisible(true);
    ui->enterCount->setVisible(true);
}

void MainWindow::onPlayerCountEntered() {
    totalPlayers = ui->playerCount->value(); // Берем число игроков
    currentPlayer = 0; // Обнуляем счетчик игроков
    raceManager->processNextPlayer(); // Начинаем ввод данных первого игрока
    ui->l_count->setVisible(false);
    ui->playerCount->setVisible(false);
    ui->enterCount->setVisible(false);

}


void MainWindow::displayBetFields() {
    QList<QLabel*> nameLabels = {ui->playerName_1, ui->playerName_2, ui->playerName_3, ui->playerName_4, ui->playerName_5};
    QList<QLineEdit*> sumFields = {ui->playerSum_1, ui->playerSum_2, ui->playerSum_3, ui->playerSum_4, ui->playerSum_5};

    for (int i = 0; i < totalPlayers; ++i) {
        nameLabels[i]->setText(raceManager->players[i]->getName());
        nameLabels[i]->setVisible(true);
        sumFields[i]->setVisible(true);
    }

    ui->placeBetButton->setVisible(true);
}

void MainWindow::addPlayer(Player* newPlayer) {
    raceManager->players.append(newPlayer);  // Добавляем обычный указатель
}


void MainWindow::clearSelection() {
    for (QLabel* widget : imageWidgets) {
        widget->setStyleSheet(""); // Сбрасываем рамку
    }
}


void MainWindow::onCockroachSelected(int cockroachId) {
    if (cockroachId < 0 || cockroachId >= raceManager->cockroaches.size()) {
        qDebug() << "Ошибка: некорректный ID таракана!";
        return;
    }

    qDebug() << "Выделен таракан: " << raceManager->cockroaches[cockroachId]->getName();


    // Подсвечиваем выбранного таракана
    for (int i = 0; i < imageWidgets.size(); ++i) {
        imageWidgets[i]->setStyleSheet(i == cockroachId ? "border: 3px solid red;" : "");
    }


}


void MainWindow::startRace() {
    elapsedTime = 0;  // Обнуляем счетчик перед стартом
    ui->lcdTimer->display(elapsedTime);  // Обновляем начальное значение
    timer->start(1000);  // Запускаем таймер с интервалом 1 секунда
}

void MainWindow::stopRace() {
    timer->stop();  // Останавливаем таймер
}

void MainWindow::updateTimer() {
    elapsedTime++;  // Увеличиваем счетчик
    int minutes = elapsedTime / 60;
    int seconds = elapsedTime % 60;
    QString timeString = QString("%1:%2").arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0'));

    ui->lcdTimer->display(timeString);  // Выводим в LCD-таймер
}

void MainWindow::initializeCockroachPositions() {
    int yOffset = ui->textBrowser->y();
    int startX = ui->textBrowser->x();

    for (size_t i = 0; i < raceManager->cockroaches.size(); ++i) {
        int startY = yOffset + i * 100;
        raceManager->cockroaches[i]->setPosition(startX, startY);
        imageWidgets[i]->move(startX, startY); // Перемещаем изображения
    }
}

void MainWindow::onRaceFinished() {
    saveCockroachesToFile();
    updateRaceInfoUI(); // Обновляем UI после игры

    ui->continueButton->setVisible(true);
    ui->finishButton->setVisible(true);
}

void MainWindow::onFinishClicked() {
    resetGame();

    ui->startGame->setVisible(true);

}

void MainWindow::resetGame() {
    qDeleteAll(raceManager->players);
    raceManager->players.clear();
    initializeCockroachPositions();


    for (size_t i = 0; i < raceManager->cockroaches.size(); ++i) {
        raceManager->cockroaches[i]->resetPosition();  // Сбрасываем позицию таракана
        imageWidgets[i]->move(raceManager->cockroaches[i]->getPosition());  // Перемещаем изображение в исходное положение
    }

    hideAllWidgets();

    // Сбрасываем таймер
    elapsedTime = 0; // Обнуляем счетчик времени
    ui->lcdTimer->display("00:00"); // Обновляем отображение таймера

    // Перезагрузить начальный экран
    ui->startGame->setVisible(true);
    ui->playerCount->setValue(0); // Сбросить количество игроков
    ui->l_count->setVisible(false);
    ui->playerName_1->setVisible(false);
    ui->playerSum_1->setVisible(false);
    // Очистить и скрыть поля игроков, ставки и так далее...

    // Перевести игру в начальное состояние
    ui->finishButton->setVisible(false);
    ui->continueButton->setVisible(false);
}


void MainWindow::onContinueClicked() {
    // Удаляем игроков без денег
    for (auto it = raceManager->players.begin(); it != raceManager->players.end(); ) {
        if ((*it)->getMoney() <= 0) {
            qDebug() << "Игрок" << (*it)->getName() << "выбывает из игры.";
            delete *it; // Освобождаем память
            it = raceManager->players.erase(it); // Удаляем игрока из списка
        } else {
            ++it;
        }
    }

    // Если игроков не осталось, завершаем игру
    if (raceManager->players.isEmpty()) {
        QMessageBox::information(this, "Игра окончена", "Все игроки выбыли из игры!");
        resetGame(); // Сброс игры
        return;
    }

    // Обновляем totalPlayers для продолжения игры
    totalPlayers = raceManager->players.size();

    resetRaceData();
    ui->l_player_name->setVisible(true);
    raceManager->processCockroachSelection(); // Начинаем выбор тараканов для первого игрока
}


void MainWindow::resetRaceData() {
    // Очищаем только ставки и результаты забега
    elapsedTime = 0; // Обнуляем счетчик времени
    ui->lcdTimer->display("00:00"); // Обновляем отображение таймера

    for (size_t i = 0; i < raceManager->cockroaches.size(); ++i) {
        raceManager->cockroaches[i]->resetPosition();  // Сбрасываем позицию таракана
        imageWidgets[i]->move(raceManager->cockroaches[i]->getPosition());  // Перемещаем изображение в исходное положение
    }

    // Сбрасываем выбор тараканов для оставшихся игроков
    for (auto& player : raceManager->players) {
        player->setCockroach(nullptr); // Сброс выбранного таракана
    }

    // Обновляем интерфейс
    raceManager->updateBetTable(); // Обновляем таблицу ставок
    displayBetFields(); // Показываем поля для ставок
    displayCockroaches(true); // Показываем выбор тараканов

    hideAllWidgets();

    // Начинаем процесс выбора тараканов
    currentPlayer = 0; // Сбрасываем счетчик игроков

    // Сбрасываем ставки
    raceManager->playerBets.clear();
}

