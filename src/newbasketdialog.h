#ifndef NEWBASKETDIALOG_H
#define NEWBASKETDIALOG_H

#include <KDialog>

class KLineEdit;
class QString;

class NewBasketDialog : public KDialog {
	Q_OBJECT

	public:
		NewBasketDialog();
		~NewBasketDialog();

		bool isOk() { return m_isOk; }
		QString name();

	protected slots:
		//void slotButtonClicked( int button );
		void validate( const QString& newName );
		void setOk();
	
	private:
		bool m_isOk;
		KLineEdit* m_name;
};

#endif
