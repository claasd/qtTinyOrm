#include "SqlWriter.h"

#include <QFile>
#include <QStringList>
#include <QTextStream>

SqlWriter::SqlWriter(const Database &db) : mDatabase(db)
{

}

void SqlWriter::Write(const QString &filename, Type type)
{
	mType = type;
	const QString tab("    ");
	QFile file(filename);
	if(!file.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not write SQL File");
	QTextStream out;
	out.setDevice(&file);
	if(mType != SqLiteType)
	{
		out << QString(" -- Create database") << endl << endl;
		out << QString("CREATE DATABASE IF NOT EXISTS `%1`;").arg(mDatabase.name) << endl << endl;
		out << QString("USE `%1`;").arg(mDatabase.name) << endl << endl;
	};
	QListIterator<Table> tables(mDatabase.tables);
	while(tables.hasNext())
	{
		const Table& table = tables.next();
		out << QString("DROP TABLE IF EXISTS `%1`;").arg(table.name) << endl;	
		out << QString("CREATE TABLE `%1` (").arg(table.name) << endl;
		QListIterator<Column> columns(table.columns);
		QStringList pKeys;
		while(columns.hasNext())
		{
			const Column& col = columns.next();
			out << tab << QString("`%1` ").arg(col.name) << SqlColumnName(col);
			if(!col.null || col.primaryKey)
				out << " NOT";
			out << " NULL";
			if(col.primaryKey)
			{
				if(mType == SqLiteType)
					out << " PRIMARY KEY";
				else
					pKeys.append(col.name);
			}
			if(col.autoIncrement)
				if(mType == SqLiteType)
					out << " AUTOINCREMENT";
				else
					out << " AUTO_INCREMENT";
			if(col.hasDefault)
				out << QString(" DEFAULT '%1'").arg(col.defaultValue);
			if(columns.hasNext() || !pKeys.empty() || !table.foreignKeys.empty())
				out << ",";
			out << endl;
		}
		if(!pKeys.empty())
		{
			out << tab << QString("PRIMARY KEY ( `%1` )").arg(pKeys.join("`, `"));
			if(!table.foreignKeys.empty())
				out << ",";
			out << endl;
		}
		QListIterator<ForeignKey> foreignKeys(table.foreignKeys);
		int id = 0;
		while(foreignKeys.hasNext())
		{
			const ForeignKey& key = foreignKeys.next();
			id++;
			if(mType != SqLiteType)
				out << tab << "INDEX `" << table.name << "_FI_" << id << "` (`" << key.localColumns.join("`, `") << "`)," << endl;
			out << tab << "CONSTRAINT `" << table.name << "_FK_" << id << "`" << endl;
			out << tab << tab << "FOREIGN KEY (`" << key.localColumns.join("`, `") << "`)" << endl;;
			out << tab << tab << "REFERENCES `" << key.foreignTable <<"` (`" << key.foreignColumns.join("`, `") << "`)";
			if(foreignKeys.hasNext())
				out << ",";
			out << endl;
		}
		out << ");" << endl << endl;
	}

	file.close();
}

QString SqlWriter::SqlColumnName(const Column& col)
{
	QString type;
	switch(col.type)
	{
	case BooleanType:
	case IntegerType:
		return "INTEGER";
	case VarcharType:
		return QString("VARCHAR(%1)").arg(col.size);
	case LongVarcharType:
		return "TEXT";
	case DecimalType:
		return QString("DECIMAL(%1, %2)").arg(col.size).arg(col.precision);
	case DateTimeType:
		return "DATETIME";
	case DateType:
		return "DATE";
	default:
		throw std::runtime_error(QString("Unknown columnt type: %1").arg(col.type).toStdString());
	}
};