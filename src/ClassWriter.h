#ifndef CLASSWRITER_H
#define CLASSWRITER_H

#include "DataObjects.h"

#include <QTextStream>
#include <QStringList>

class ClassWriter
{
public:
	ClassWriter(const Database& db);
	void Write(const QString& dir);
private:
	void WriteTable(const Table& table, const QString& dir);

	QString HeaderStart(const QStringList& classNames, const QStringList& includes = QStringList(), const QString& parent = QString(), bool writeCtor = true);
	QString HeaderStart(const QString className, const QStringList& includes = QStringList(), const QString& parent = QString(), bool writeCtor = true)
		{ return HeaderStart(QStringList(className), includes, parent, writeCtor);};
	QString HeaderEnd(const QString& className);

	void WriteBaseHeader(const Table& table, QTextStream& stream);
	void WriteBasePeerHeader(const Table &table, QTextStream &out);
	void WriteHeader(const Table& table, QTextStream& stream);
	void WritePeerHeader(const Table& table, QTextStream& stream);

	void WriteBaseSrc(const Table& table, QTextStream& stream);
	void WriteBasePeerSrc(const Table &table, QTextStream &out);

	QString HeaderLine(const QString& str, int tabs = 2, int endLines = 1);
	QString SourceLine(const QString& str, int tabs = 1, int endLines = 1) { return HeaderLine(str, tabs, endLines); };

	QString toCamelCase(const QString& name, bool upper = true);
	QString ColumnTypeName(const Column& col, bool asReference = false);
	QString ColumnVariantCommand(const Column& col);

	QString getClassName(const Table& table) { return getClassName(table.name); };
	QString getClassName(const QString& name) { return toCamelCase(name); };
	QString getBaseClassName(const Table& table){ return getBaseClassName(table.name); };
	QString getBaseClassName(const QString& name){ return "Base" + getClassName(name); };
	QString getPeerClassName(const Table& table){ return getClassName(table) + "Peer"; };
	QString getBasePeerClassName(const Table& table){ return getBasePeerClassName(table.name); };
	QString getBasePeerClassName(const QString& name){ return getBaseClassName(name) + "Peer"; };

	QString getFKMethodName(const QString& tableName, const QList<QString>& columns, bool multipleReferences, bool isGetList = false);
	Database mDatabase;
	Table mCurrentTable;
	QMap<QString, QList<ForeignKey>> mExternTables, mExternTables2;
	QString tab;
	QString exceptionName;
};

#endif //CLASSWRITER_H