#include "Parser.h"
#include <exception>

Parser::Parser()
{

}

bool Parser::ParseBool(const QString& _str, bool defaultValue)
{
	if(_str.isEmpty())
		return defaultValue;
	QString str = _str.toLower();
	return str == "yes" || str == "true" || str == "1" || str=="enable";
}

void Parser::Parse(const QString &filename)
{
	QFile file(filename);
	if(!file.exists())
		throw std::runtime_error("Input files does not exist!");
	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		throw std::runtime_error("Could not open input file!");
	}

	mMetStartTag = false;
	mCurrentTable = 0;
	mCurrentForeignKey = 0;
	QXmlSimpleReader reader;
	reader.setContentHandler(this);
	reader.setErrorHandler(this);
	QXmlInputSource xmlInputSource(&file);
	if(!reader.parse(xmlInputSource))
	{
		QString err = errorString();
		if(err.isEmpty())
			err = QXmlDefaultHandler::errorString();
		throw std::runtime_error(errorString().toStdString());
	}
}

bool Parser::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &attributes)
{
	if(!mMetStartTag && qName == "database")
	{
		mDatabase.name = attributes.value("name");
		mMetStartTag = true;
		return true;
	}
	else if(!mMetStartTag)
	{
		mErrStr = QString("expected <database> tag, found <%1>!").arg(qName);
		return false;
	}
	else if(qName == "table")
	{
		mDatabase.tables.append(Table());
		mCurrentTable = &mDatabase.tables.last();
		mCurrentTable->name = attributes.value("name");
		return true;
	}
	else if(qName == "column")
	{
		mCurrentTable->columns.append(Column());
		Column& col = mCurrentTable->columns.last();
		col.name = attributes.value("name");
		col.type = ColumnTypeFromStr(attributes.value("type"));
		col.size = attributes.value("size").toInt();
		col.precision = attributes.value("precision").toInt();
		col.primaryKey = ParseBool(attributes.value("primaryKey"), false);
		col.autoIncrement = ParseBool(attributes.value("autoIncrement"), false);
		col.null = ParseBool(attributes.value("null"), true);
		col.hasDefault = attributes.index("default") > -1;
		col.defaultValue = attributes.value("default");
		if(col.hasDefault && col.type == BooleanType)
			col.defaultValue = ParseBool(col.defaultValue, false) ? "1" : "0";
		return true;
	}
	else if(qName == "foreign-key")
	{
		mCurrentTable->foreignKeys.append(ForeignKey());
		mCurrentForeignKey = &mCurrentTable->foreignKeys.last();
		mCurrentForeignKey->foreignTable = attributes.value("foreignTable");
		return true;
	}
	else if(qName == "reference")
	{
		mCurrentForeignKey->foreignColumns.append(attributes.value("foreign"));
		mCurrentForeignKey->localColumns.append(attributes.value("local"));
		return true;
	}
	mErrStr = QString("tag <%1> cold not be parsed!").arg(qName);
	return false;
}

bool Parser::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
	return true;
}

ColumnType Parser::ColumnTypeFromStr(const QString& _str)
{
	QString str = _str.toLower();
	if(str == "integer")
		return IntegerType;
	if(str == "varchar")
		return VarcharType;
	if(str == "longvarchar" || str == "text")
		return LongVarcharType;
	if(str == "boolean")
		return BooleanType;
	if(str == "decimal")
		return DecimalType;
	if(str == "datetime")
		return DateTimeType;
	if(str == "date")
		return DateType;
	throw std::runtime_error(QString("unknown type: %1").arg(str).toStdString());
};