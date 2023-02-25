#include <QDebug>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "CsvReader.h"
#include "Storage.h"

int MinGameLength = 2 * 60; //секунды
int MinMmr = 2100; //ниже него вообще не учитываются игры
int MmrDiff = 175; //если у победителя +столько относительно лузера, то такие победы не в счёт
int TopMmr = 2350; //выше него не учитывается разница ммр

QMap<QString, QSharedPointer<Game>> _games;
QMap<QString, QSharedPointer<Player>> _players; //by key
QMap<QString, int> _wins; //by races pair HE = H>E; EH = E>H etc

QString playerKey(const QString& name, Race::Type race)
{
    return name + "_" + race;
}

QString playerKey(const QSharedPointer<Player>& player)
{
    return playerKey(player->name, player->race);
}

QSharedPointer<Player> getPlayer(const QString& name, Race::Type race, float mmr, float dmmr)
{
    if (!Race::isValid(race))
        return QSharedPointer<Player>(nullptr);
    QString key = playerKey(name, race);
    QSharedPointer<Player> p = _players.value(key);
    if (p.isNull()) {
        p.reset(new Player());
        p->name = name;
        p->race = race;
        p->bestMmr = 0;
        _players.insert(key, p);
    }
    float maxMmr = qMax(mmr, mmr + dmmr);
    p->bestMmr = qMax(maxMmr, p->bestMmr);

    return p;
}

bool read()
{
    QFile f("D:/Download/w3c_s13_match_data.csv");
    if (!f.open(QIODevice::ReadOnly)) {
        qDebug() << f.errorString();
        return false;
    }

    QTextStream s(&f);
    CsvReader reader(&s);
    QStringList columns = reader.read();

    int length = columns.indexOf("match.lengthSeconds");
    int gameIdIndex = columns.indexOf("_id");
    int dateIndex = columns.indexOf("match._created_at");
    int p1nameIndex = columns.indexOf("0.battleTag");
    int p1raceIndex = columns.indexOf("0.race");
    int p1wonIndex = columns.indexOf("0.won");
    int p1mmrIndex = columns.indexOf("0.mmr.rating");
    int p1dmmrIndex = columns.indexOf("0.mmr.rd");
    int p2nameIndex = columns.indexOf("1.battleTag");
    int p2raceIndex = columns.indexOf("1.race");
    int p2wonIndex = columns.indexOf("1.won");
    int p2mmrIndex = columns.indexOf("1.mmr.rating");
    int p2dmmrIndex = columns.indexOf("1.mmr.rd");

    int count = 0;
    while (!reader.atEnd()) {
        if (++count % 1000 == 0)
            qDebug() << count;

        QStringList lst = reader.read();

        float lengthSec = lst.at(length).toFloat();
        if (lengthSec < MinGameLength)
            continue;

        QString gameId = lst.at(gameIdIndex);
        QDateTime dt = QDateTime::fromString(lst.at(dateIndex), Qt::ISODateWithMs);

        QString p1name = lst.at(p1nameIndex);
        Race::Type p1race = (Race::Type)lst.at(p1raceIndex).toInt();
        bool p1won = lst.at(p1wonIndex) == "True";
        float p1mmr = lst.at(p1mmrIndex).toFloat();
        float p1dmmr = lst.at(p1dmmrIndex).toFloat();

        QString p2name = lst.at(p2nameIndex);
        Race::Type p2race = (Race::Type)lst.at(p2raceIndex).toInt();
        bool p2won = lst.at(p2wonIndex) == "True";
        float p2mmr = lst.at(p2mmrIndex).toFloat();
        float p2dmmr = lst.at(p2dmmrIndex).toFloat();

        Q_ASSERT(p1won != p2won);
        Q_ASSERT(p1dmmr > 0);
        Q_ASSERT(p2dmmr > 0);

        if (p1won)
            p2dmmr = -p2dmmr;
        else
            p1dmmr = -p1dmmr;

        QSharedPointer<Player> p1 = getPlayer(p1name, p1race, p1mmr, p1dmmr);
        QSharedPointer<Player> p2 = getPlayer(p2name, p2race, p2mmr, p2dmmr);
        if (p1.isNull() || p2.isNull() || p1race == p2race)
            continue;

        QSharedPointer<Game> game(new Game());
        game->id = gameId;
        game->winner = p1won ? p1 : p2;
        game->loser = p2won ? p1 : p2;
        _games.insert(gameId, game);
    }

    return true;
}

int main(int argc, char *argv[])
{
    if (!read())
        return 1;

    qDebug().noquote() << "Number of games without mmr filter:" << _games.count();

    for (const QSharedPointer<Game>& game : _games.values()) {
        if (game->winner->bestMmr < MinMmr || game->loser->bestMmr < MinMmr)
            continue;

        if (game->winner->bestMmr >= TopMmr && game->loser->bestMmr >= TopMmr) {
            //учёт
        } else if (game->winner->bestMmr - game->loser->bestMmr > MmrDiff) {
            continue;
        }

        qDebug().noquote() << game->winner->toString() << " > " << game->loser->toString();

        QString pair = Race::toString(game->winner->race) + Race::toString(game->loser->race);
        int count = _wins[pair];
        _wins[pair] = count + 1;
    }

    for (const QString& key : _wins.keys()) {
        int winCount = _wins[key];
        int loseCount = _wins[QString(key[1]) + key[0]];
        int totalCount = winCount + loseCount;
        qDebug().noquote() << QString(key[0]).toUpper() + "x" + QString(key[1]).toUpper() << QString::number(100.0 * winCount / totalCount, 'f', 2) + "%"
            << winCount << ":" << loseCount;
    }

    return 0;
}
