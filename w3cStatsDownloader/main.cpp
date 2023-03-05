// NEED libcrypto-1_1-x64.dll + libssl-1_1-x64.dll in Windows


#include <QTextStream>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include "Worker.h"
#include "Storage.h"
#include "CsvWriter.h"

/*
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
*/


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QList<QSharedPointer<Game>> games;

    Worker w;
    a.exec();
    games = w.result();

    QFile f("e:/w3c_now.csv");
    if (!f.open(QIODevice::WriteOnly)) {
        qDebug() << f.errorString();
        return 666;
    }
    QTextStream stream(&f);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(true);

    CsvWriter writer(&stream);
    writer.write(QStringList() << "_id" << "match.lengthSeconds" << "match._created_at"
        << "0.battleTag" << "0.race" << "0.random" << "0.won" << "0.mmr.rating" << "0.mmr.rd"
        << "1.battleTag" << "1.race" << "1.random" << "1.won" << "1.mmr.rating" << "1.mmr.rd");
    std::sort(games.begin(), games.end(), [](const QSharedPointer<Game>& r1, const QSharedPointer<Game>& r2)->bool {
        return r1->startTime < r2->startTime;
    });

    int MinGameLength = 2 * 60; //секунды
    for (const QSharedPointer<Game>& g : games) {
        if (g->lengthSeconds < MinGameLength)
            continue;
        QStringList lst;
        lst << g->id << QString::number(g->lengthSeconds) << g->startTime.toString(Qt::ISODateWithMs)
            << g->winner->name << QString::number(g->winner->race) << QString::number(g->winner->rndRace) << "True" << QString::number(g->winner->oldMmr) << QString::number(g->winner->mmrGain)
            << g->loser->name << QString::number(g->loser->race) << QString::number(g->loser->rndRace) << "False" << QString::number(g->loser->oldMmr) << QString::number(-g->loser->mmrGain);
        writer.write(lst);
    }
    qDebug() << "Export completed.";

	return 0;
}
