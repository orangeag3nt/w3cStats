#pragma once
#include <QObject>
#include <QList>
#include <QSharedPointer>
#include "Storage.h"

class QNetworkReply;
class QJsonObject;

struct WorkerPrivate;
class Worker : public QObject
{
	Q_OBJECT
public:
	Worker();
	~Worker();

	QList<QSharedPointer<Game>> result() const;

private:
	void doWork();
	void doQueue();
	void slotNetworkReplyFinished(QNetworkReply* r);
	void parsePlayers(QNetworkReply* r);
	void parseMatches(QNetworkReply* r);
	QSharedPointer<Game> gameFromJson(const QJsonObject& obj) const;

	WorkerPrivate* d;
};