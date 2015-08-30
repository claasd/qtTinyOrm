#ifndef QTTINYORM_CRITERIA_H
#define QTTINYORM_CRITERIA_H

#include <QVariant>

namespace qtTinyOrm
{
	struct CriteriaData;
	class Criteria
	{
	public:
        enum Mode
        {
            Equal,
            GreaterThen,
            LessThen,
            GreaterEqual,
            LessEqual,
            In,
            Like
        };

        enum Seperator
        {
            AndSeperator,
            OrSeperator
        };

        Criteria(Seperator sep = AndSeperator);
		Criteria(const Criteria& other);
        virtual ~Criteria();
        void setSeperator(Seperator sep);
        Criteria& add(const QString& field, const QVariant& value, Mode mode = Equal);
        Criteria& add(const Criteria& other);
		Criteria& setLimit(int limit);
        Criteria& addOrder(const QString& field, Qt::SortOrder order = Qt::AscendingOrder);
		
        QString toString(bool inner = false) const;



	private:
		CriteriaData *mData;
	}; // end class Peer
}; // end namespace qtTinyOrm

#endif //QTTINYORM_CRITERIA_H
