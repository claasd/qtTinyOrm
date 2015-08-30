#include "DataObject.h"

using namespace qtTinyOrm;

#include <QDebug>

DataObject::DataObject(int numFields, const QString fieldNames[])
{
	mIsNew = true;
	mIsDeleted = false;
	for(int i=0; i< numFields; i++)
	{
		mData.insert(fieldNames[i], QVariant());
	}
}

DataObject::DataObject(const DataObject &other) : 
	QSharedData(other), 
	mData(other.mData), 
	mIsNew(other.mIsNew),
	mModifiedColumns(other.mModifiedColumns)
{
}

QVariant DataObject::getValue(const QString &field) const
{
	if(!mData.contains(field))
		return QVariant();
	return mData[field];
}

bool DataObject::setValue(const QString &field, const QVariant &data)
{
	if(!mData.contains(field))
		return false;
	if(mData[field] != data)
	{
		mData[field] = data;
		mModifiedColumns.insert(field);
	}
	return true;
}

void DataObject::hydrate(const QMap<QString,QVariant>& data)
{
	QMapIterator<QString, QVariant> iter(data);
	while(iter.hasNext())
	{
		iter.next();
		if(mData.contains(iter.key()))
			mData[iter.key()] = iter.value();
	}
    mIsNew = false;
}

void DataObject::debugPrint()
{
    QMapIterator<QString, QVariant> iter(mData);
    while(iter.hasNext())
    {
        iter.next();
        qDebug() << iter.key() << ":" << iter.value();
    }
}

QList<QPair<QString, QVariant> > DataObject::getModifedValues() const
{
	QList<QPair<QString, QVariant>> result;
    QSet<QString> modified = mModifiedColumns;
    QSetIterator<QString> iter(modified);
	while(iter.hasNext())
	{
		const QString& key = iter.next();
		result.append(QPair<QString, QVariant>(key, mData[key]));
	}
    if(isNew() && mData.contains("created_at") && !modified.contains("created_at"))
        result.append(QPair<QString, QVariant>("created_at", QVariant()));
    if(mData.contains("modified_at") && !modified.contains("modified_at"))
        result.append(QPair<QString, QVariant>("modified_at", QVariant()));
	return result;
}
