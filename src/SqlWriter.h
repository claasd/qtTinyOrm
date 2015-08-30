#ifndef SQLWRITER_H
#define SQLWRITER_H

#include "DataObjects.h"

class SqlWriter
{
public:
	enum Type
	{
		SqLiteType,
		MysqlType
	};

	SqlWriter(const Database& db);
	void Write(const QString& filename, Type type);
private:
	Database mDatabase;
	Type mType;
	QString SqlColumnName(const Column& col);
};

#endif //SQLWRITER_H