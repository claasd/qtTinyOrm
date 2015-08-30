#ifndef QTTINYORM_DATAOBJECT_H
#define QTTINYORM_DATAOBJECT_H

#include <QSharedData>
#include <QVariant>
#include <QDateTime>
#include <QSet>

namespace qtTinyOrm
{
	class Peer;

	class DataObject : public QSharedData
	{
	public:
		DataObject(int numFields, const QString fieldNames[]);
		DataObject(const DataObject& other);
		QVariant getValue(const QString& field) const;
		bool setValue(const QString& field, const QVariant& data);

        bool isNew() const { return mIsNew; }
        bool isDeleted() const { return mIsDeleted; }
        bool isModified() const { return !mModifiedColumns.empty(); }
        QList<QPair<QString,QVariant> > getModifedValues() const;
		void hydrate(const QMap<QString, QVariant>& data);

        void setIsNew(bool isNew) { mIsNew = isNew; }
        void setIsDeleted(bool isDeleted) { mIsDeleted = isDeleted; }
        void clearModifiedColumns() { mModifiedColumns.clear(); }
        void debugPrint();
    private:
		QMap<QString, QVariant> mData;
        QSet<QString> mModifiedColumns;
		bool mIsNew;
		bool mIsDeleted;

	}; // end class DataObject
}; // end namespace qtTinyOrm

#endif //QTTINYORM_DATAOBJECT_H
