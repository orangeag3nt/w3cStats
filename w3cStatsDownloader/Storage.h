#pragma once
#include <QString>
#include <QSharedPointer>
#include <QDateTime>

struct Race
{
	enum Type {
		Random = 0,
		Human = 1,
		Orc = 2,
		Elf = 4,
		Undead = 8
	};

	static bool isValid(Race::Type t)
	{
		return t == Human || t == Orc || t == Elf || t == Undead;
	}

	static QString toString(Race::Type t)
	{
		switch (t) {
		case Random:
			return "R";
		case Human:
			return "H";
		case Orc:
			return "O";
		case Elf:
			return "E";
		case Undead:
			return "U";
		default:
			return QString();
		}
	}
};

struct Player
{
	QString name;
	Race::Type race = Race::Random;
	Race::Type rndRace = Race::Random;
	float oldMmr;
	float mmrGain;
};

struct Game
{
	int lengthSeconds;
	QDateTime startTime;
	QString id;
	QSharedPointer<Player> winner;
	QSharedPointer<Player> loser;
};