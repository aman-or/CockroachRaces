#include "Cockroach.h"
#include <QJsonObject>

Cockroach::Cockroach(const QString &name, const QString &imagePath, int startX, int startY)
    : name(name), imagePath(imagePath), position(startX, startY), startPosition(startX, startY), raceCount(0), winCount(0) {}

QString Cockroach::getName() const { return name; }
QString Cockroach::getImagePath() const { return imagePath; }
void Cockroach::setPosition(int x, int y) { position = QPoint(x, y); }
QPoint Cockroach::getPosition() const { return position; }

int Cockroach::getRaceCount() const { return raceCount; }
int Cockroach::getWinCount() const { return winCount; }
void Cockroach::incrementRaceCount() { raceCount++; }
void Cockroach::incrementWinCount() { winCount++; }
void Cockroach::resetPosition() { position = startPosition; }

void Cockroach::setRaceCount(int count) {
    raceCount = count;
}

void Cockroach::setWinCount(int count) {
    winCount = count;
}


// Сохранение таракана в JSON
QJsonObject Cockroach::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["raceCount"] = raceCount;
    json["winCount"] = winCount;
    return json;
}

// Загрузка таракана из JSON
void Cockroach::fromJson(const QJsonObject &json) {
    if (json.contains("name")) name = json["name"].toString();
    if (json.contains("raceCount")) setRaceCount(json["raceCount"].toInt());
    if (json.contains("winCount")) setWinCount(json["winCount"].toInt());
}
