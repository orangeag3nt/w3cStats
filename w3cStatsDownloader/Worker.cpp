#include "Worker.h"
#include <QTimer>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "Storage.h"

int Season = 14;
int MinMmr = 1900; //считываем все игры игроков, которым СЕЙЧАС >=MinMmr

struct WorkerPrivate
{
	QNetworkAccessManager* networkManager = nullptr;
	QList<QString> playerQueue;
	QHash<QString, QSharedPointer<Game>> games;

	int currentQueueTotalCount = 0;
	int currentQueueOffset = 0;
	QList<QSharedPointer<Game>> currentQueueGames;
	bool gotPlayers = false;
};

Worker::Worker()
{
	d = new WorkerPrivate();
	QTimer::singleShot(0, this, &Worker::doWork);
}

Worker::~Worker()
{
	if (d->networkManager)
		d->networkManager->deleteLater();
	delete d;
}

void Worker::doWork()
{
	d->networkManager = new QNetworkAccessManager(this);
	connect(d->networkManager, &QNetworkAccessManager::finished, this, &Worker::slotNetworkReplyFinished);

	QUrlQuery urlq;
	urlq.addQueryItem("season", QString::number(Season));
	//urlq.addQueryItem("searchFor", "ag3nt");
	urlq.addQueryItem("searchFor", "GM_1v1");

	QString host = "https://website-backend.w3champions.com/api/ladder/search";

	QUrl url(host);
	url.setQuery(urlq);

	QNetworkRequest request;

	request.setUrl(url);

	QSslConfiguration sslConfiguration;
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(sslConfiguration);

	d->networkManager->get(request);
}

void Worker::slotNetworkReplyFinished(QNetworkReply* r)
{
	if (d->gotPlayers)
		parseMatches(r);
	else
		parsePlayers(r);
}

void Worker::parsePlayers(QNetworkReply* r)
{
	QSet<QString> players;
	QByteArray receivedData = r->readAll();
	QJsonParseError err;
	QJsonDocument jdoc = QJsonDocument::fromJson(receivedData, &err);
	QJsonArray playerArray = jdoc.array();
	for (int i = 0; i < playerArray.count(); i++) {
		QJsonObject playerObject = playerArray.at(i).toObject();
		QString name = playerObject.value("player1Id").toString();

		int mmr = playerObject.value("player").toObject().value("mmr").toInt();
		if (mmr < MinMmr)
			continue;

		players << name;
		qDebug().noquote() << name << mmr;
	}
	d->playerQueue = players.toList();
	d->gotPlayers = true;
	QTimer::singleShot(0, this, &Worker::doQueue);
}

void Worker::doQueue()
{
	if (d->playerQueue.isEmpty()) {
		QCoreApplication::quit();
		return;
	}

	const QString name = d->playerQueue.at(0);

	QUrlQuery urlq;
	urlq.addQueryItem("season", QString::number(Season));
	urlq.addQueryItem("playerId", name);
	urlq.addQueryItem("gameMode", "GM_1v1");
	urlq.addQueryItem("offset", QString::number(d->currentQueueOffset));

	QString host = "https://website-backend.w3champions.com/api/matches/search";

	QUrl url(host);
	url.setQuery(urlq);

	QNetworkRequest request;

	request.setUrl(url);

	QSslConfiguration sslConfiguration;
	sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
	request.setSslConfiguration(sslConfiguration);

	qDebug().noquote() << request.url().toString();
	d->networkManager->get(request);
}

void Worker::parseMatches(QNetworkReply* r)
{
	QByteArray receivedData = r->readAll();
	QJsonParseError err;
	QJsonDocument jdoc = QJsonDocument::fromJson(receivedData, &err);
	QJsonObject obj = jdoc.object();
	QJsonArray matches = obj.value("matches").toArray();
	int totalCount = obj.value("count").toInt();

	if (d->currentQueueOffset != 0 && totalCount != d->currentQueueTotalCount) {
		d->currentQueueGames.clear();
		d->currentQueueTotalCount = 0;
		d->currentQueueOffset = 0;
		QTimer::singleShot(0, this, &Worker::doQueue);
		return;
	}

	d->currentQueueTotalCount = totalCount;
	for (int i = 0; i < matches.count(); i++) {
		QJsonObject match = matches.at(i).toObject();
		d->currentQueueGames << gameFromJson(match);
	}

	if (d->currentQueueGames.count() >= totalCount) {
		Q_ASSERT(d->currentQueueGames.count() == totalCount);

		for (const QSharedPointer<Game>& game : d->currentQueueGames) {
			d->games.insert(game->id, game);
		}

		d->currentQueueGames.clear();
		d->currentQueueTotalCount = 0;
		d->currentQueueOffset = 0;
		d->playerQueue.removeAt(0);
	} else {
		d->currentQueueOffset = d->currentQueueGames.count();
	}
	QTimer::singleShot(0, this, &Worker::doQueue);
}

QSharedPointer<Game> Worker::gameFromJson(const QJsonObject& obj) const
{
	QSharedPointer<Game> game(new Game());
	game->id = obj.value("id").toString();
	game->lengthSeconds = obj.value("durationInSeconds").toInt();
	game->startTime = QDateTime::fromString(obj.value("startTime").toString(), Qt::ISODateWithMs);
	QJsonArray teams = obj.value("teams").toArray();
	Q_ASSERT(teams.count() == 2);
	for (int i = 0; i < teams.count(); i++) {
		QJsonObject pObj = teams.at(i).toObject();
		Player* p1 = new Player();
		bool won = pObj.value("won").toBool();
		QJsonArray players = pObj.value("players").toArray();
		Q_ASSERT(players.count() == 1);
		pObj = players.at(0).toObject();
		p1->name = pObj.value("battleTag").toString();
		p1->mmrGain = pObj.value("mmrGain").toInt();
		p1->oldMmr = pObj.value("oldMmr").toInt();
		p1->race = (Race::Type)pObj.value("race").toInt();

		QJsonValue rndRace = pObj.value("rndRace");
		if (!rndRace.isNull())
			p1->rndRace = (Race::Type)rndRace.toInt();

		if (won) {
			game->winner.reset(p1);
		} else {
			game->loser.reset(p1);
		}
	}
	Q_ASSERT(!game->winner.isNull());
	Q_ASSERT(!game->loser.isNull());

	return game;
}

QList<QSharedPointer<Game>> Worker::result() const
{
	return d->games.values();
}