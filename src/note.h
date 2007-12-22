#ifndef NOTE_H
#define NOTE_H

#include <QtGlobal>
#include <QString>
#include <QPointF>

#include <boost/shared_ptr.hpp>

class Note;

typedef boost::shared_ptr<Note> NotePtr;

/*
 * the basic class to store information that will be represented in the note on the graphics scene
 * also it's used by plugin to store data in akonadi
 */
class Note {

	public:
		Note( QPointF pos, const QString& text = QString() );
		Note( qreal x = 0, qreal y = 0, const QString& text = QString() );
		~Note();

		bool parseString( const QString& data );
		QString toString();

		void setPos( QPointF pos );
		void setPos( qreal x, qreal y );
		void setText( const QString& text = QString() );

		QPointF pos() const;
		qreal x() const;
		qreal y() const;
		const QString& text() const;

	private:
		QPointF mPos;
		QString mText;

};

#endif
