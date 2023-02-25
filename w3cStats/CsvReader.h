#pragma once

#include <QStringList>
class QTextStream;

struct CsvReaderPrivate;
class CsvReader
{
public:
	CsvReader(QTextStream* base);
	virtual ~CsvReader();
	QStringList read();
	bool atEnd();

private:
	CsvReaderPrivate* d;
};
