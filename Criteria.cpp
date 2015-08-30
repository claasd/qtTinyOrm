#include "Criteria.h"

#include <QTextStream>
#include <QstringList>

namespace qtTinyOrm
{
	

	struct CriteriaData
	{
        CriteriaData() : limit(0), seperator("AND") {}
        QString seperator;
		struct Filter
		{
			QString field;
			QVariant data;
            Criteria::Mode mode;
            Criteria other;
            Filter(const QString& f, const QVariant& d, Criteria::Mode m) : field(f), data(d), mode(m) {}
            Filter(const Criteria& c) : other(c) {}
		};
		typedef QListIterator<Filter> FilterIterator;

		struct Order
		{
			QString field;
            Qt::SortOrder order;
            Order(const QString& f, Qt::SortOrder o) : field(f), order(o) {}
		};
		typedef QListIterator<Order> OrderIterator;

		QList<Filter> filter;
		QList<Order> order;
		int limit;

        static QString decode(const QVariant& var)
        {
            switch(var.userType())
            {
                case QVariant::Bool:
                case QVariant::Int:
                case QVariant::Char:
                    return QString::number(var.toInt());
                default:
                    return QString("\"%1\"").arg(var.toString().replace("\"", "\\\""));
            }
        }
	};
};

using namespace qtTinyOrm;

Criteria::Criteria(Seperator sep)
{
	mData = new CriteriaData();
    setSeperator(sep);
}

Criteria::Criteria(const Criteria &other)
{
	mData = new CriteriaData();
    mData->seperator = other.mData->seperator;
	mData->filter = other.mData->filter;
	mData->order = other.mData->order;
	mData->limit = other.mData->limit;
}

Criteria::~Criteria()
{
    delete mData;
}

void Criteria::setSeperator(Criteria::Seperator sep)
{
    if(sep == OrSeperator)
        mData->seperator = "OR";
    else
        mData->seperator = "AND";
}


Criteria& Criteria::add(const QString &field, const QVariant &value, Mode mode)
{
    mData->filter.append(CriteriaData::Filter(field, value, mode));
    return *this;
}

Criteria &Criteria::add(const Criteria &other)
{
    mData->filter.append(CriteriaData::Filter(other));
    return *this;
}

Criteria& Criteria::addOrder(const QString &field, Qt::SortOrder order)
{
    mData->order.append(CriteriaData::Order(field, order));
	return *this;
}

Criteria& Criteria::setLimit(int limit)
{
	mData->limit = limit;
	return *this;
}

QString Criteria::toString(bool inner) const
{
	QString data;
	QTextStream stream(&data);

	CriteriaData::FilterIterator filter(mData->filter);
    if(filter.hasNext() && !inner)
		stream << " WHERE";
	while(filter.hasNext())
	{
		const CriteriaData::Filter& f = filter.next();
        if(f.field.isEmpty())
        {
            stream << " (" << f.other.toString(true) << ")";
        }
        else
        {
            QString name = "`" + f.field + "`";
            name = name.replace(".", "`.`");
            stream << " " << name;
            switch(f.mode) {
                case GreaterThen:
                    stream << " > "; break;
                case LessThen:
                    stream << " < "; break;
                case GreaterEqual:
                    stream << " >= "; break;
                case LessEqual:
                    stream << " <= "; break;
                case In:
                    stream << " IN "; break;
                case Like:
                    stream << " LIKE "; break;
                default:
                    stream << " = "; break;
            }
            if(f.mode == In)
            {
                QStringList temp;
                foreach(const QVariant& var, f.data.toList())
                {
                    temp << mData->decode(var);
                }
                stream << "(" << temp.join(", ") << ")";

            }
            else
                stream << mData->decode(f.data);
        }
		if(filter.hasNext())
            stream << " " << mData->seperator;
	}
	CriteriaData::OrderIterator order(mData->order);
	if(order.hasNext())
        stream << " ORDER BY ";
	while(order.hasNext())
	{
		const CriteriaData::Order& o = order.next();
        QString name = "`" + o.field + "` ";
        name = name.replace(".", "`.`");
        stream << name << (o.order == Qt::AscendingOrder ? "ASC" : "DESC");
		if(order.hasNext())
		stream << ", ";
	}
	if(mData->limit > 0)
		stream << " LIMIT 0, "<< mData->limit;
	return data;
}
