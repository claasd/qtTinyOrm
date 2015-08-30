#include "Peer.h"

#include <QSqlQuery>
#include <QDebug>
#include <QSqlError>
#include <QTextStream>
#include <QFile>
#include <QDateTime>

using namespace qtTinyOrm;

Peer::Peer()
{
}

QList<QMap<QString, QVariant> > Peer::doSelect(int numFields, const QString fields[], const QString &table, const Criteria &criteria, const QString &connectionName)
{
	QString statementStr;
	QTextStream statement(&statementStr);
	statement << "SELECT ";
	for(int i=0; i< numFields; i++)
	{
		QString field = fields[i];
		statement << "`" + field.replace(QString("."),QString("`.`"))+ "`";
		if(i+1<numFields)
			statement << ", ";
	}
	statement << QString(" FROM `%1`").arg(table);
	statement << " " << criteria.toString();

    QSqlQuery query;
    if(!connectionName.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(connectionName));
    qDebug() << statementStr;
    QList<QMap<QString, QVariant>> result;
    if(!query.exec(statementStr))
	{
        QSqlError err = query.lastError();
        qCritical() << err.text();
        qCritical() << statementStr;
        return result;
	}
	while(query.next())
	{
		QMap<QString, QVariant> resultMap;
		for(int i=0; i< numFields; i++)
		{
			resultMap.insert(fields[i], query.value(i));
		}
		result.append(resultMap);
	}
	return result;
}

QVariant Peer::doInsert(const QString &table, const QList<QPair<QString,QVariant> >& columns, bool retrievLastInsert, const QString &connectionName)
{
    QString statementStr;
    QTextStream statement(&statementStr);
	statement << QString("INSERT INTO `%1`").arg(table);
	QStringList keys;
	QStringList values;
	for(int i=0; i< columns.size(); i++)
	{
        QString key = columns[i].first;
        QString field = "`" + key + "`";
		keys.append(field);
        QVariant value = columns[i].second;
		QString valueString;
        if((key == "created_at" || key == "modified_at"))
            value = QDateTime::currentDateTime();
        if(value.type() == QVariant::Bool)
			valueString = value.toBool() ? "1" : "0";
		else
			valueString = QString("\"%1\"").arg(value.toString());
		values.append(valueString);
	}
	statement << " (" << keys.join(", ") << ") VALUES (" << values.join(", ") << ")";

    QSqlQuery query;
    if(!connectionName.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(connectionName));

    qDebug() << statementStr;
	if(!query.exec(statementStr))
	{
		QSqlError err = query.lastError();
        qCritical() << err.text();
        qCritical() << statementStr;
        return QVariant();
	}
	if(!retrievLastInsert)
		return QVariant();
    return query.lastInsertId();
}

void Peer::doUpdate(const QString &table, const QList<QPair<QString, QVariant> > &columns, const Criteria &criteria, const QString &connectionName)
{
	QString statementStr;
	QTextStream statement(&statementStr);
	statement << QString("UPDATE `%1`").arg(table);
	QStringList keys;
	QStringList values;
    statement << " SET ";
	for(int i=0; i< columns.size(); i++)
	{
        QString key = columns[i].first;
        statement <<  "`" + key + "` = ";
        QVariant value = columns[i].second;
		QString valueString;
        if(key == "modified_at")
            value = QDateTime::currentDateTime();
        if(value.type() == QVariant::Bool)
			valueString = value.toBool() ? "1" : "0";
		else
			valueString = QString("\"%1\"").arg(value.toString());
		statement << valueString;
		if(i+1 < columns.size())
			statement << ", ";
	}
	statement << " " << criteria.toString();
    qDebug() << statementStr;
    QSqlQuery query;
    if(!connectionName.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(connectionName));
	if(!query.exec(statementStr))
	{
        QSqlError err = query.lastError();
        qCritical() << err.text();
        qCritical() << statementStr;
	}
}

void Peer::doDelete(const QString &table, const Criteria &criteria, const QString &connectionName)
{
	QString statementStr;
	QTextStream statement(&statementStr);
	statement << QString("DELETE FROM `%1`").arg(table);
	statement << " " << criteria.toString();
    QSqlQuery query;
    if(!connectionName.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(connectionName));
    qDebug() << statementStr;
	if(!query.exec(statementStr))
	{
		QSqlError err = query.lastError();
        qCritical() << err.text();
        qCritical() << statementStr;
    }
}

void Peer::executeSql(const QString &data, const QString& connectionName)
{
    QSqlQuery query;
    if(!connectionName.isEmpty())
        query = QSqlQuery(QSqlDatabase::database(connectionName));

    QStringList querys = data.split(";");
    foreach(QString q, querys)
    {
        q = q.trimmed();
        if(q.isEmpty() || q.startsWith("--"))
            continue;
        if(!query.exec(q))
        {
            QSqlError err = query.lastError();
            qCritical() << err.text();
            qCritical() << q;
        }
    }
}


void Peer::executeSqlFile(const QString &filename, const QString& connectionName)
{
    QFile f(filename);
    if(f.open(QFile::ReadOnly | QFile::Text))
    {
        QString data = f.readAll();
        executeSql(data, connectionName);
    }
}
