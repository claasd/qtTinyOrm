#ifndef QTTINYORM_PEER_H
#define QTTINYORM_PEER_H

#include <QStringList>
#include "Criteria.h"

namespace qtTinyOrm
{
	class Peer
	{
	private:
		Peer();

    public:
        static void executeSql(const QString& data, const QString &connectionName = QString());
        static void executeSqlFile(const QString& filename, const QString &connectionName = QString());

	protected:
        static QList<QMap<QString, QVariant> > doSelect(int numFields, const QString fields[], const QString& table, const Criteria& criteria = Criteria(), const QString &connectionName = QString());
        static QVariant doInsert(const QString &table, const QList<QPair<QString,QVariant> >& columns, bool retrievLastInsert = false, const QString &connectionName = QString());
        static void doUpdate(const QString& table, const QList<QPair<QString, QVariant> >& columns, const Criteria& criteria = Criteria(), const QString &connectionName = QString());
        static void doDelete(const QString& table, const Criteria& criteria = Criteria(), const QString &connectionName = QString());


	}; // end class Peer
}; // end namespace qtTinyOrm

#endif //QTTINYORM_PEER_H
