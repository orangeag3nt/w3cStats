#include <QString>
#include <QSharedPointer>

struct Race
{
	enum Type {
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
	Race::Type race;
	float bestMmr;

	QString toString() const
	{
		return QString("%0 %1 (%2)").arg(name, Race::toString(race), QString::number((int)bestMmr));
	}
};

struct Game
{
	QString id;
	QSharedPointer<Player> winner;
	QSharedPointer<Player> loser;
};