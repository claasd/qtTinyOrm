#ifndef PARSER_H
#define PARSER_H

#include <QXmlDefaultHandler>
#include <QString>
#include "DataObjects.h"

class Parser : public QXmlDefaultHandler
{
public:

	Parser();
	void Parse(const QString& filename);
	bool startElement(const QString &namespaceURI, const QString &localName,const QString &qName, const QXmlAttributes &attributes);
	bool endElement(const QString &namespaceURI, const QString &localName,const QString &qName);
	QString errorString() const { return mErrStr; };
	const Database& getDatabase() { return mDatabase; };
private:
	ColumnType ColumnTypeFromStr(const QString& _str);
	bool ParseBool(const QString& _str, bool defaultValue);
	bool mMetStartTag;
	Database mDatabase;
	Table* mCurrentTable;
	ForeignKey* mCurrentForeignKey;
	QString mErrStr;
};
#endif //PARSER_H