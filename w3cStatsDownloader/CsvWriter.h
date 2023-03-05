#pragma once

#include <QStringList>
class QTextStream;

struct CsvWriterPrivate;
class CsvWriter
{
public:
	CsvWriter(QTextStream* base);
	virtual ~CsvWriter();
	void write(const QStringList& items);
	static QString format(const QString& input);

private:
	CsvWriterPrivate* d;
};
