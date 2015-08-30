#ifndef DATAOBJECTS_H
#define DATAOBJECTS_H

#include <QVariant>
#include <QStringList>


enum ColumnType
{
	IntegerType,
	VarcharType,
	LongVarcharType,
	BooleanType,
	DecimalType,
	DateTimeType,
	DateType,
};


struct Column
{
	ColumnType type;
	QString name;
	int size;
	int precision;
	bool primaryKey;
	bool autoIncrement;
	bool hasDefault;
	bool null;
	QString defaultValue;
};

struct ForeignKey
{
	QString foreignTable;
	QStringList localColumns;
	QStringList foreignColumns;
};

struct Table
{
	QString name;
	QList<Column> columns;
	QList<ForeignKey> foreignKeys;
};

struct Database
{
	QString name;
	QList<Table> tables;
};

#endif //DATAOBJECTS_H