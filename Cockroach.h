#ifndef COCKROACH_H
#define COCKROACH_H

#include <QString>
#include <QPoint>
#include <QObject>

class Cockroach : public QObject {
    Q_OBJECT
public:
    Cockroach(const QString &name, const QString &imagePath, int startX, int startY);

    QString getName() const;
    QString getImagePath() const;
    void setPosition(int x, int y);
    QPoint getPosition() const;

    int getRaceCount() const;
    int getWinCount() const;
    void incrementRaceCount();
    void incrementWinCount();
    void resetPosition(); // Добавляем объявление метода

    QJsonObject toJson() const;  // Сохранение в JSON
    void fromJson(const QJsonObject &json); // Загрузка из JSON
    void setRaceCount(int count);
    void setWinCount(int count);

private:
    QString name;       // Имя таракана
    QString imagePath;  // Путь к изображению
    QPoint position;    // Текущая позиция на экране
    QPoint startPosition; // Стартовая позиция таракана

    int raceCount = 0;  // Количество забегов
    int winCount = 0;   // Количество побед


};

#endif // COCKROACH_H
