#include "ClassWriter.h"

#include <QFile>
#include <QStringList>
#include <QDir>
#include <QSet>

ClassWriter::ClassWriter(const Database &db) : mDatabase(db)
{
	tab = "\t";
	exceptionName = "std::runtime_error";
}

void ClassWriter::Write(const QString &dir)
{
	QDir(dir).mkdir("base");
	QListIterator<Table> tables(mDatabase.tables);
	while(tables.hasNext())
		WriteTable(tables.next(), dir);
}

void ClassWriter::WriteTable(const Table& table, const QString& dir)
{
	mCurrentTable = table;
	mExternTables.clear();
	mExternTables2.clear();



	QListIterator<Table> tableIter(mDatabase.tables);
	while(tableIter.hasNext())
	{
		const Table& otherTable = tableIter.next();
		if(table.name == otherTable.name)
			continue;
		QListIterator<ForeignKey> keyIter(otherTable.foreignKeys);
		while(keyIter.hasNext())
		{
			const ForeignKey& key = keyIter.next();
			if(key.foreignTable == table.name)
			{
				mExternTables[otherTable.name].append(key);
			}
		}
	}

	QListIterator<ForeignKey> keyIter(table.foreignKeys);
	while(keyIter.hasNext())
	{
		const ForeignKey& key = keyIter.next();
		mExternTables2[key.foreignTable].append(key);
	}


	QString className = getClassName(table);
	QString classPeerName = getPeerClassName(table);
	QString baseClassName = getBaseClassName(table);
	QString baseClassPeerName = getBasePeerClassName(table);
	
	

	QFile baseHFile(dir+QString("/base/%1.h").arg(baseClassName));
	if(!baseHFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create base header file");

	QFile baseHPeerFile(dir+QString("/base/%1.h").arg(baseClassPeerName));
	if(!baseHPeerFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create base peer header file");

	QFile hFile(dir+QString("/%1.h").arg(className));
	if(!hFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create header file");

	QFile hPeerFile(dir+QString("/%1.h").arg(classPeerName));
	if(!hPeerFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create peer header file");

	QFile baseCppFile(dir+QString("/base/%1.cpp").arg(baseClassName));
	if(!baseCppFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create base peer header file");

	QFile baseCppPeerFile(dir+QString("/base/%1.cpp").arg(baseClassPeerName));
	if(!baseCppPeerFile.open(QFile::WriteOnly | QFile::Text))
		throw std::runtime_error("Could not create base peer header file");

	WriteBaseHeader(table, QTextStream(&baseHFile));
	WriteBasePeerHeader(table, QTextStream(&baseHPeerFile));
	WriteHeader(table, QTextStream(&hFile));
	WritePeerHeader(table, QTextStream(&hPeerFile));

	WriteBaseSrc(table, QTextStream(&baseCppFile));
	WriteBasePeerSrc(table, QTextStream(&baseCppPeerFile));
	

	
}

void ClassWriter::WriteHeader(const Table &table, QTextStream &out)
{
	out << HeaderStart(getClassName(table), QStringList("base/"+getBaseClassName(table)+".h"), getBaseClassName(table), false);
	out << endl;

	out << HeaderEnd(getClassName(table));
}

void ClassWriter::WritePeerHeader(const Table &table, QTextStream &out)
{
	out << HeaderStart(getPeerClassName(table), QStringList("base/"+getBasePeerClassName(table)+".h"), getBasePeerClassName(table), false);
	out << endl;

	out << HeaderEnd(getPeerClassName(table));
}

void ClassWriter::WriteBaseHeader(const Table &table, QTextStream &out)
{
	QString realClassName = getClassName(table);
	QString className = getBaseClassName(table);
	QString peerClassName = getBasePeerClassName(table);
	
	QSet<QString> classes;
	foreach(QString tableName, mExternTables.keys())
		classes.insert(getClassName(tableName));
	foreach(QString tableName, mExternTables2.keys())
		classes.insert(getClassName(tableName));
	classes.insert(realClassName);
    out << HeaderStart(QStringList(className) << peerClassName << classes.toList(), QStringList("<QSharedDataPointer>") << "../qtTinyOrm/DataObject.h" << "../qtTinyOrm/Criteria.h" );
    out << HeaderLine("virtual bool isNew() const;");
    out << HeaderLine("virtual bool isModified() const;");
    out << HeaderLine("virtual bool isDeleted() const;");
    out << HeaderLine("virtual bool save(const QString& connectionName = QString());");
    out << HeaderLine("virtual bool remove(const QString& connectionName = QString());");
    out << HeaderLine("virtual void copyTo("+ className + "& cls) const;");
    out << HeaderLine("virtual " +realClassName + " clone() const;");
    out << HeaderLine("virtual void debugPrint() const;");
	out << endl;

	QListIterator<Column> columns(table.columns);
	while(columns.hasNext())
	{
		const Column& col = columns.next();
		QString methodName = toCamelCase(col.name);
		out << HeaderLine(QString("// Get/Set methods for column '%1'").arg(col.name));
        out << HeaderLine(QString("virtual %1 get%2() const;").arg(ColumnTypeName(col)).arg(methodName));
        out << HeaderLine(QString("virtual bool set%2(%1 value);").arg(ColumnTypeName(col, true)).arg(methodName));
		out << endl;
	}
	QMapIterator<QString, QList<ForeignKey>> referenceIter(mExternTables);
	while(referenceIter.hasNext())
	{
		referenceIter.next();
		const QString& otherTableName = referenceIter.key();
		const QList<ForeignKey>& fKeyList = referenceIter.value();
		out << HeaderLine(QString("// Get/Add methods for foreign objects '%1'").arg(otherTableName));
		for(int i=0; i<fKeyList.size(); i++)
		{
			QString methodName = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1, true);
			QString methodName2 = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1, false);
            out << HeaderLine(QString("virtual QList<%1> get%2(const qtTinyOrm::Criteria& _crit = qtTinyOrm::Criteria(), const QString& connectionName = QString()) const;").arg(getClassName(otherTableName)).arg(methodName));
            out << HeaderLine(QString("virtual QList<%1> get%2(const QString& connectionName) const;").arg(getClassName(otherTableName)).arg(methodName));
            out << HeaderLine(QString("virtual bool add%2(%1& cls, const QString& connectionName = QString());").arg(getClassName(otherTableName)).arg(methodName2));
			out << endl;
		}
	}

	QMapIterator<QString, QList<ForeignKey>> referenceIter2(mExternTables2);
	while(referenceIter2.hasNext())
	{
		referenceIter2.next();
		const QString& otherTableName = referenceIter2.key();
		const QList<ForeignKey>& fKeyList = referenceIter2.value();
		out << HeaderLine(QString("// Get/Set methods for foreign objects '%1'").arg(otherTableName));
		for(int i=0; i<fKeyList.size(); i++)
		{
			QString methodName = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1);
            out << HeaderLine(QString("virtual %1 get%2(const QString& connectionName = QString()) const;").arg(getClassName(otherTableName)).arg(methodName));
            out << HeaderLine(QString("virtual bool set%2(const %1& cls);").arg(getClassName(otherTableName)).arg(methodName));
			out << endl;
		}
	}

	out << HeaderLine("private:", 1);
	out << HeaderLine("friend class "+ peerClassName+";");
	out << HeaderLine("friend class qtTinyOrm::Peer;");
	out << HeaderLine(QString("QSharedDataPointer<qtTinyOrm::DataObject> mData;"));
	out << HeaderEnd(className);
}

QString ClassWriter::getFKMethodName(const QString& tableName, const QList<QString>& columns, bool multipleReferences, bool getIsList)
{
	QString methodName = getClassName(tableName);
	if(getIsList)
		methodName.append("s");
	if(multipleReferences)
	{
		methodName.append("RelatedBy");
		for(int i=0;i<columns.size(); i++)
		{
			if(i>0)
				methodName.append("And");
			methodName.append(toCamelCase(columns[i]));
		}
	}
	return methodName;
}

void ClassWriter::WriteBaseSrc(const Table &table, QTextStream &out)
{
	QString realClassName = getClassName(table);
	QString className = getBaseClassName(table);
	QString peerClassName = getBasePeerClassName(table);

	QSet<QString> includes;
	includes.insert("../" + realClassName);
	includes.insert(peerClassName);
	foreach(QString tableName, mExternTables.keys())
	{
		includes.insert("../"+getClassName(tableName));
		includes.insert(getBasePeerClassName(tableName));
	}
	foreach(QString tableName, mExternTables2.keys())
	{
		includes.insert("../"+getClassName(tableName));
		includes.insert(getBasePeerClassName(tableName));
	}

	QSetIterator<QString> includeIter(includes);
	while(includeIter.hasNext())
		out << "#include \"" << includeIter.next() << ".h\"" << endl;
	
    out << "#include <QDebug>" << endl;
	out << endl;



	out << "using namespace " << mDatabase.name << ";" << endl << endl;

	out << className << "::" << className << "()" << endl << "{" << endl;
	out << SourceLine("mData = new qtTinyOrm::DataObject("+ peerClassName + "::NUM_COLUMNS," + peerClassName +  "::COLUMNS);");
	out << "}" << endl << endl;

	out << className << "::~" << className << "()" << endl << "{" << endl;
	out << "}" << endl << endl;

	out << "bool "+ className << "::isNew() const" << endl << "{" << endl;
	out << SourceLine("return mData->isNew();");
	out << "}" << endl << endl;

	out << "bool "+ className << "::isModified() const" << endl << "{" << endl;
	out << SourceLine("return mData->isModified();");
	out << "}" << endl << endl;

	out << "bool "+ className << "::isDeleted() const" << endl << "{" << endl;
	out << SourceLine("return mData->isDeleted();");
	out << "}" << endl << endl;

    out << "bool "+ className << "::save(const QString& connectionName)" << endl << "{" << endl;
    out << SourceLine("return " +peerClassName + "::doSave(*this, connectionName);");
	out << "}" << endl << endl;

    out << "bool "+ className << "::remove(const QString& connectionName)" << endl << "{" << endl;
    out << SourceLine("return "+ peerClassName+ "::doDelete(*this, connectionName);");
	out << "}" << endl << endl;

    out << realClassName + " "+ className << "::clone() const" << endl << "{" << endl;
	out << SourceLine(realClassName + " cls;");
    out << SourceLine("copyTo(cls);");
    out << SourceLine("return cls;");
    out << "}" << endl << endl;

    out << "void "+ className << "::copyTo(" + className +"& cls) const" << endl << "{" << endl;

	QListIterator<Column> columns(table.columns);
	while(columns.hasNext())
	{
		const Column& col = columns.next();
		if(!col.primaryKey)
		{
			QString methodName = toCamelCase(col.name);
			out << SourceLine("cls.set" + methodName + "(get" + methodName + "());");
		}
	}
	out << "}" << endl << endl;

    out << "void "+ className << "::debugPrint() const" << endl << "{" << endl;
    out << SourceLine("qDebug() << \"begin " +realClassName + ":\";");
    out << SourceLine("mData->debugPrint();");
    out << SourceLine("qDebug() << \"end " +realClassName + ";\";");
    out << "}" << endl << endl;


	columns.toFront();
	while(columns.hasNext())
	{
		const Column& col = columns.next();
		QString methodName = toCamelCase(col.name);
		out << ColumnTypeName(col) << " " << className+"::get" << methodName << "() const" << endl << "{" << endl;
		out << SourceLine("return mData->getValue(" + peerClassName + "::" + col.name.toUpper() + ")." + ColumnVariantCommand(col) + ";");
		out << "}" << endl << endl;

		out << "bool " << className+"::set" << methodName << "(" << ColumnTypeName(col,true) << " value)" << endl << "{" << endl;
		out << SourceLine("return mData->setValue(" + peerClassName + "::" + col.name.toUpper() + ", value);");
		out << "}" << endl << endl;
	}

	QMapIterator<QString, QList<ForeignKey>> referenceIter(mExternTables);
	while(referenceIter.hasNext())
	{
		referenceIter.next();
		const QString& otherTableName = referenceIter.key();
		const QList<ForeignKey>& fKeyList = referenceIter.value();
		for(int i=0;i <fKeyList.size(); i++)
		{
			QString methodName = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1, true);
			QString methodName2 = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1);
			QString objName = getClassName(otherTableName);
			QString objPeerName = getBasePeerClassName(otherTableName);
            out << QString("QList<%1> ").arg(objName) << className << QString("::get%1(const qtTinyOrm::Criteria& _crit, const QString& connectionName) const").arg(methodName) << endl << "{" << endl;
			out << SourceLine("qtTinyOrm::Criteria crit(_crit);");
			out << tab << "crit";
			for(int j=0; j<fKeyList[i].foreignColumns.size(); j++)
			{
				QString local = fKeyList[i].localColumns[j];
				QString foreign = fKeyList[i].foreignColumns[j];
				out << ".add(" << objPeerName << "::" << local.toUpper() << ", get" << toCamelCase(foreign) << "())";
			}
			out << ";" << endl;
            out << SourceLine("return "+ objPeerName +"::doSelect(crit, connectionName);");
			out << "}" << endl << endl;	

            out << QString("QList<%1> ").arg(objName) << className << QString("::get%1(const QString& connectionName) const").arg(methodName) << endl << "{" << endl;
            out << SourceLine(QString("return get%1(qtTinyOrm::Criteria(), connectionName);").arg(methodName));
            out << "}" << endl << endl;

            out << "bool " << className << QString("::add%1(%2& cls, const QString& connectionName)").arg(methodName2).arg(objName) << endl << "{" << endl;
			out << SourceLine("if(cls.isDeleted()|| isNew() || isDeleted())");
			out << SourceLine("return false;",2);
			for(int j=0; j<fKeyList[i].foreignColumns.size(); j++)
			{
				QString local = fKeyList[i].localColumns[j];
				QString foreign = fKeyList[i].foreignColumns[j];
				out << tab << "cls.set" << toCamelCase(local) << "(get" << toCamelCase(foreign) << "());" << endl;
			}
            out << SourceLine("cls.save(connectionName);");
			out << SourceLine("return true;");
			out << "}" << endl << endl;	

		}
	}


	QMapIterator<QString, QList<ForeignKey>> referenceIter2(mExternTables2);
	while(referenceIter2.hasNext())
	{
		referenceIter2.next();
		const QString& otherTableName = referenceIter2.key();
		const QList<ForeignKey>& fKeyList = referenceIter2.value();
		for(int i=0; i <fKeyList.size(); i++)
		{
			QString methodName = getFKMethodName(otherTableName, fKeyList[i].localColumns, fKeyList.size() > 1);
			QString objName = getClassName(otherTableName);
			QString objPeerName = getBasePeerClassName(otherTableName);
            out << objName << " " << className << QString("::get%1(const QString& connectionName) const").arg(methodName) << endl << "{" << endl;
			out << SourceLine("qtTinyOrm::Criteria crit;");
			out << tab << "crit";
			for(int j=0; j<fKeyList[i].foreignColumns.size(); j++)
			{
				QString local = fKeyList[i].localColumns[j];
				QString foreign = fKeyList[i].foreignColumns[j];
				out << ".add(" << objPeerName << "::" << foreign.toUpper() << ", get" << toCamelCase(local) << "())";
			}
			out << ";" << endl;
            out << SourceLine("return "+ objPeerName +"::doSelectOne(crit, connectionName);");
			out << "}" << endl << endl;	
            out << "bool " << className << QString("::set%1(const %2& cls)").arg(methodName).arg(objName) << endl << "{" << endl;
			out << SourceLine("if(cls.isNew() || cls.isDeleted())");
			out << SourceLine("return false;",2);
			for(int j=0; j<fKeyList[i].foreignColumns.size(); j++)
			{
				QString local = fKeyList[i].localColumns[j];
				QString foreign = fKeyList[i].foreignColumns[j];
				out << tab << "set" << toCamelCase(local) << "(cls.get" << toCamelCase(foreign) << "());" << endl;
			}
			out << SourceLine("return true;");
			out << "}" << endl << endl;	

		}
	}

	

}

void ClassWriter::WriteBasePeerHeader(const Table &table, QTextStream &out)
{
	
	QString className = getClassName(table);
	QString baseClassName = getBaseClassName(table);
	QString peerClassName = getBasePeerClassName(table);
    out << HeaderStart(peerClassName, QStringList("<QList>") << "../"+className+".h" << "../qtTinyOrm/Peer.h" << "../qtTinyOrm/Criteria.h", "qtTinyOrm::Peer");
	QStringList pKeys;

	QListIterator<Column> columns(table.columns);
	
	while(columns.hasNext())
	{
		const Column& col = columns.next();
		out << HeaderLine("const static QString " + col.name.toUpper() + ";");
		if(col.primaryKey)
			pKeys.append(ColumnTypeName(col)+ " "+ toCamelCase(col.name, false));
	}

	out << endl;
	out << HeaderLine("const static QString DATABASE_NAME;");
	out << HeaderLine("const static QString TABLE_NAME;");
	out << HeaderLine("const static int NUM_COLUMNS;");
	out << HeaderLine("const static QString COLUMNS[];");
	out << endl;

	if(!pKeys.empty())
        out << HeaderLine("static " + className + " retrieveByPk(" + pKeys.join(", ")+ ", const QString& connectionName = QString());");

    out << HeaderLine("static QList<" + className + "> doSelect(const qtTinyOrm::Criteria& crit = qtTinyOrm::Criteria(), const QString& connectionName = QString());");
    out << HeaderLine("static " + className + " doSelectOne(const qtTinyOrm::Criteria& crit = qtTinyOrm::Criteria(), const QString& connectionName = QString());");
    out << HeaderLine("static bool doSave(" + baseClassName + "& cls, const QString& connectionName = QString(), bool setAutoFields = true);");
    out << HeaderLine("static bool doDelete(" + baseClassName + "& cls, const QString& connectionName = QString());");

	out << HeaderEnd(peerClassName);
}

void ClassWriter::WriteBasePeerSrc(const Table &table, QTextStream &out)
{
	QString className = getClassName(table);
	QString baseClassName = getBaseClassName(table);
	QString peerClassName = getBasePeerClassName(table);

	out << "#include \"" << peerClassName << ".h\"" << endl << endl;
	out << "using namespace " << mDatabase.name << ";" << endl << endl;

	QListIterator<Column> columns(table.columns);
	QStringList fields;
	
	QList<Column> pKeys;

	QString autoIncCol;

	while(columns.hasNext())
	{
		const Column& col = columns.next();
		fields << col.name.toUpper();
        out << "const QString " << peerClassName << "::" << col.name.toUpper() << QString(" = \"%1\";").arg(col.name) << endl;
		if(col.primaryKey)
			pKeys.append(col);
		if(col.autoIncrement)
			autoIncCol = col.name.toUpper();
	}

	out << endl << "const int " << peerClassName << "::NUM_COLUMNS = " << fields.size() << ";" << endl;
	out << endl << "const QString " << peerClassName << "::COLUMNS[] = {" << fields.join(", ") << "};" << endl;
	out << "const QString " << peerClassName << "::TABLE_NAME = \"" << table.name << "\";" << endl;
	out << "const QString " << peerClassName << "::DATABASE_NAME = \"" << mDatabase.name << "\";" << endl << endl;
	if(!pKeys.empty())
	{
		out << className << " " << peerClassName << "::retrieveByPk(";
		QListIterator<Column> keys(pKeys);
		QString addCritLine = "crit";
		while(keys.hasNext())
		{
			const Column& col = keys.next();
			out << ColumnTypeName(col) << " " << toCamelCase(col.name, false);
			if(keys.hasNext())
				out << ",";
			addCritLine.append(".add(" + col.name.toUpper() + ", " + toCamelCase(col.name, false) + ")");
		}
        out << ", const QString& connectionName)" << endl << "{" << endl;
		out << SourceLine("qtTinyOrm::Criteria crit;");
		out << SourceLine(addCritLine+";");
        out << SourceLine("return doSelectOne(crit, connectionName);");
		out << "}" << endl << endl;
	}
    out << className << " " << peerClassName << "::doSelectOne(const qtTinyOrm::Criteria& _crit, const QString& connectionName)" << endl << "{" << endl;
	out << SourceLine("qtTinyOrm::Criteria crit(_crit);");
	out << SourceLine("crit.setLimit(1);");
    out << SourceLine("QList<"+className+"> result = doSelect(crit, connectionName);");
	out << SourceLine("if(result.isEmpty())");
	out << SourceLine("return "+className+"();",2);
	out << SourceLine("return result[0];");
	out << "}" << endl << endl;

    out << "QList<" << className << "> " << peerClassName << "::doSelect(const qtTinyOrm::Criteria& crit, const QString& connectionName)" << endl << "{" << endl;
	out << SourceLine("QList<"+className+"> results;");
    out << SourceLine("QList<QMap<QString, QVariant>> data = Peer::doSelect(NUM_COLUMNS, COLUMNS, TABLE_NAME, crit, connectionName);");
	out << SourceLine("for(int i=0;i<data.size(); i++)");
	out << SourceLine("{");
	out << SourceLine(className +" cls;",2);
	out << SourceLine("cls.mData->hydrate(data[i]);",2);
	out << SourceLine("results.append(cls);",2);
	out << SourceLine("}");
	out << SourceLine("return results;");
	out << "}" << endl << endl;

    out << "bool " << peerClassName << "::doSave(" << baseClassName << "& cls, const QString& connectionName, bool setAutoFields)" << endl << "{" << endl;
	out << SourceLine("QList<QPair<QString, QVariant>> modifiedData = cls.mData->getModifedValues();");
	out << SourceLine("if(modifiedData.empty())");
	out << SourceLine("return true;",2);
	out << SourceLine("if(cls.mData->isNew())");
	out << SourceLine("{");
    out << SourceLine(QString("QVariant autIncVal = Peer::doInsert(TABLE_NAME, modifiedData, ") +( autoIncCol.isEmpty() ? "false" : "true") + ", connectionName, setAutoFields);",2);
	if(!autoIncCol.isEmpty())
		out << SourceLine("cls.mData->setValue(" + autoIncCol+ ", autIncVal);",2);
	out << SourceLine("cls.mData->setIsNew(false);",2);
	out << SourceLine("}");
	out << SourceLine("else");
	out << SourceLine("{");
	out << SourceLine("qtTinyOrm::Criteria crit;",2);
	if(!pKeys.isEmpty())
	{
		QListIterator<Column> keys(pKeys);
		QString addCritLine = "crit";
		while(keys.hasNext())
		{
			const Column& col = keys.next();
			addCritLine.append(".add(" + col.name.toUpper() + ", cls.mData->getValue(" +  col.name.toUpper() + "))");
		}
		out << SourceLine(addCritLine + ";",2);
	}
    out << SourceLine("Peer::doUpdate(TABLE_NAME, modifiedData, crit, connectionName, setAutoFields);",2);
	out << SourceLine("}");
	out << SourceLine("cls.mData->clearModifiedColumns();");
	out << SourceLine("return true;");
	out << "}" << endl << endl;


    out << "bool " << peerClassName << "::doDelete(" << baseClassName << "& cls, const QString& connectionName)" << endl << "{" << endl;
	out << SourceLine("if(cls.mData->isNew() || cls.mData->isDeleted())");
	out << SourceLine("return false;",2);
	out << SourceLine("qtTinyOrm::Criteria crit;");
	if(!pKeys.isEmpty())
	{
		QListIterator<Column> keys(pKeys);
		QString addCritLine = "crit";
		while(keys.hasNext())
		{
			const Column& col = keys.next();
			addCritLine.append(".add(" + col.name.toUpper() + ", cls.mData->getValue(" +  col.name.toUpper() + "))");
		}
		out << SourceLine(addCritLine + ";");
	}
    out << SourceLine("Peer::doDelete(TABLE_NAME, crit, connectionName);");
	out << SourceLine("cls.mData->setIsDeleted(true);");
	out << SourceLine("return true;");
	out << "}" << endl << endl;
}

QString ClassWriter::HeaderStart(const QStringList& classNames, const QStringList& includes, const QString& parent, bool writeCtor)
{
	QString className = classNames.first();
	QString outStr;
	QString package = mDatabase.name;
	QTextStream out (&outStr);
	QString define = QString("%1_%2_H").arg(package.toUpper()).arg(className.toUpper());
	out << QString("#ifndef ") << define << endl << QString("#define ") << define << endl << endl;
	
	QStringListIterator includeIter(includes);
	while(includeIter.hasNext())
	{
		QString include = includeIter.next();
		if(include[0] == '<')
			out << "#include " << include << endl;
		else
			out << "#include \"" << include << "\"" << endl;
	}
	if(!includes.empty())
		out << endl;

	out << "namespace " << package << endl << "{" << endl;
	QStringListIterator iter(classNames);
	iter.toBack();
	while(iter.hasPrevious())
	{
		out << tab << "class " + iter.previous();
		if(iter.hasPrevious())
			out << ";" << endl;
	}
	if(!parent.isEmpty())
		out << ": public " + parent;
	out << endl << tab << "{" << endl;

	if(writeCtor)
	{
		out << HeaderLine("public:",1);

        out << HeaderLine("explicit "+className+"();");
        out << HeaderLine("virtual ~"+className+"();");
		out << endl;
	}
	
	return outStr;
}

QString ClassWriter::HeaderEnd(const QString& className)
{
	QString package = mDatabase.name;
	QString define = QString("%1_%2_H").arg(package.toUpper()).arg(className.toUpper());
	QString outStr;
	QTextStream out (&outStr);
	out << HeaderLine("}; // end class " + className, 1);

	out << "}; // end namespace " << package << endl << endl;
	out << QString("#endif //") << define << endl;
	return outStr;
}

QString ClassWriter::HeaderLine(const QString& str, int tabs, int endLines )
{
	QString data;
	while(tabs-->0)
		data .append(tab);
	data.append(str);
	while(endLines-- > 0)
		data.append("\n");
	return data;
}

QString ClassWriter::toCamelCase(const QString &name, bool upper)
{
	QStringList parts = name.split("_");
	for(int i=0; i< parts.size(); i++)
	{
		QString& part = parts[i];
		if(!part.isEmpty() && (upper || i>0))
			part[0] = part[0].toUpper();
	};
	return parts.join("");
}

QString ClassWriter::ColumnTypeName(const Column& col, bool asReference)
{
	QString format = "%1";
	if(asReference)
		format = "const %1&";
	switch(col.type)
	{
	case IntegerType:
		return "int";
	case VarcharType:
	case LongVarcharType:
		return format.arg("QString");
	case BooleanType:
		return "bool";
	case DecimalType:
		return "double";
	case DateTimeType:
		return format.arg("QDateTime");
	case DateType:
		return format.arg("QDate");
	default:
		throw std::runtime_error(QString("Unknown columnt type: %1").arg(col.type).toStdString());
	}
}


QString ClassWriter::ColumnVariantCommand(const Column& col)
{
	switch(col.type)
	{
	case IntegerType:
		return "toInt()";
	case VarcharType:
	case LongVarcharType:
		return "toString()";
	case BooleanType:
		return "toInt() != 0";
	case DecimalType:
		return "toDouble()";
	case DateTimeType:
		return "toDateTime()";
	case DateType:
		return "toDate()";
	default:
		throw std::runtime_error(QString("Unknown columnt type: %1").arg(col.type).toStdString());
	}
}
