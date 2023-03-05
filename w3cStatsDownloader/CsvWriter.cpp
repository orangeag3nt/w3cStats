#include "CsvWriter.h"
#include <QTextStream>

struct CsvWriterPrivate
{
	QTextStream* stream;
};

CsvWriter::CsvWriter(QTextStream* base)
{
	d = new CsvWriterPrivate();
	d->stream = base;
}

CsvWriter::~CsvWriter()
{
	delete d;
}

void CsvWriter::write(const QStringList& items)
{
	int count = items.count();
	for (int i = 0; i < count; i++) {
		if (i > 0)
			*d->stream << ",";
		*d->stream << format(items.at(i));
	}
	*d->stream << "\n";
}

QString CsvWriter::format(const QString& input)
{
	QString res;
	res.reserve(input.length());
	bool needQuotes = false;
	for (const QChar& c : input) {
		if (c == ',') {
			needQuotes = true;
		} else if (c == '"') {
			needQuotes = true;
			res += c;
		}
		res += c;
	}
	if (needQuotes)
		res = '"' + res + '"';

	return res;

}