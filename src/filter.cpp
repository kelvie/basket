/***************************************************************************
 *   Copyright (C) 2003 by S�astien Laot                                 *
 *   slaout@linux62.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <qlayout.h>
#include <qpixmap.h>
#include <qimage.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qtoolbutton.h>
#include <qlabel.h>

#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kdialog.h>

#include "filter.h"
#include "global.h"
#include "bnpview.h"
#include "tools.h"
#include "tag.h"
#include "focusedwidgets.h"

/** FilterBar */

FilterBar::FilterBar(QWidget *parent, const char *name)
	: //QWidget(parent, name),/*, m_blinkTimer(this), m_blinkedTimes(0)*/
	QWidget(parent)
{
	kDebug() << "Creating FilterBar" << endl;
	QHBoxLayout *hBox  = new QHBoxLayout( this );

	// Create every widgets on the Filter Bar:

	QIcon resetIconSet = KIconLoader::global()->loadIconSet( "edit-clear-locationbar", KIconLoader::Toolbar );
	QIcon inAllIconSet = KIconLoader::global()->loadIconSet( "edit-find",              KIconLoader::Toolbar );

	m_resetButton = new QToolButton(this);
	m_resetButton->setIcon( resetIconSet );
	m_resetButton->setToolTip( i18n("Reset Filter") );
	m_resetButton->setAutoRaise( true );

	m_lineEdit           = new FocusedLineEdit(this);
//	m_lineEdit->setMaximumWidth(150);

//FIXME 1.5 QLabel *label        = new QLabel(m_lineEdit, i18n("&Filter: "), this);
	//FIXME 1.5 Add & before F: ALT+F to get to the line_edit
	QLabel *label = new QLabel( i18n("Filter: "), this );

	QLabel *label2       = new QLabel( i18n("T&ag: "), this);

	m_inAllBasketsButton = new QToolButton( this );
	m_inAllBasketsButton->setIcon( inAllIconSet );
	m_inAllBasketsButton->setToolTip( i18n("Filter all Baskets") );
	m_inAllBasketsButton->setAutoRaise( true );
	m_inAllBasketsButton->setCheckable( true );
	//FIXME Global::bnpView->toggleFilterAllBaskets(true);

	// Configure the Reset button:
	m_resetButton->setEnabled(false);

	// Configure the Tags combobox:
	m_tagsBox            = new FocusedComboBox( this );
//	repopulateTagsComnbo();

	// Layout all those widgets:
	hBox->addStretch();
	hBox->addWidget(m_resetButton);
	hBox->addSpacing(KDialog::spacingHint());
	hBox->addWidget(label);
	hBox->addWidget(m_lineEdit);
	hBox->addSpacing(KDialog::spacingHint());
	hBox->addWidget(label2);
	hBox->addWidget(m_tagsBox);
	hBox->addSpacing(KDialog::spacingHint());
	hBox->addWidget(m_inAllBasketsButton);

	m_data = new FilterData(); // TODO: Not a pointer! and return a const &  !!

//	connect( &m_blinkTimer,         SIGNAL(timeout()),                   this, SLOT(blinkBar())                  );
	connect(  m_resetButton,        SIGNAL(clicked()),                   this, SLOT(reset())                     );
	connect(  m_lineEdit,           SIGNAL(textChanged(const QString&)), this, SLOT(textChanged(const QString&)) );
	connect(  m_tagsBox,            SIGNAL(activated(int)),              this, SLOT(tagChanged(int))             );

//	connect(  m_inAllBasketsButton, SIGNAL(clicked()),                   this, SLOT(inAllBaskets())              );
	connect(  m_inAllBasketsButton, SIGNAL(toggled(bool)), Global::bnpView, SLOT(toggleFilterAllBaskets(bool)) );

	connect( m_lineEdit, SIGNAL(escapePressed()),  this, SIGNAL(escapePressed()) );
	connect( m_lineEdit, SIGNAL(returnPressed()),  this, SIGNAL(returnPressed()) );
	connect( m_tagsBox,  SIGNAL(escapePressed()),  this, SIGNAL(escapePressed()) );
	connect( m_tagsBox,  SIGNAL(returnPressed2()), this, SIGNAL(returnPressed()) );

	kDebug() << "exiting..." << endl;
}

FilterBar::~FilterBar()
{
}

void FilterBar::setFilterAll(bool filterAll)
{
//	m_inAllBasketsButton->setOn(filterAll);
}

void FilterBar::setFilterData(const FilterData &data)
{
//	m_lineEdit->setText(data.string);
//
//	int index = 0;
//	switch (data.tagFilterType) {
//		default:
//		case FilterData::DontCareTagsFilter: index = 0; break;
//		case FilterData::NotTaggedFilter:    index = 1; break;
//		case FilterData::TaggedFilter:       index = 2; break;
//		case FilterData::TagFilter:          filterTag(data.tag);     return;
//		case FilterData::StateFilter:        filterState(data.state); return;
//	}
//
//	if (m_tagsBox->currentItem() != index) {
//		m_tagsBox->setCurrentItem(index);
//		tagChanged(index);
//	}
}

void FilterBar::repopulateTagsComnbo()
{
//	static const int ICON_SIZE = 16;
//
//	m_tagsBox->clear();
//	m_tagsMap.clear();
//	m_statesMap.clear();
//
//	m_tagsBox->insertItem("",                   0);
//	m_tagsBox->insertItem(i18n("(Not tagged)"), 1);
//	m_tagsBox->insertItem(i18n("(Tagged)"),     2);
//
//	int index = 3;
//	Tag     *tag;
//	State   *state;
//	QString  icon;
//	QString  text;
//	QPixmap  emblem;
//	for (Tag::List::iterator it = Tag::all.begin(); it != Tag::all.end(); ++it) {
//		tag   = *it;
//		state = tag->states().first();
//		// Insert the tag in the combo-box:
//		if (tag->countStates() > 1) {
//			text = tag->name();
//			icon = "";
//		} else {
//			text = state->name();
//			icon = state->emblem();
//		}
//		emblem = KIconLoader::global()->loadIcon(icon, KIcon::Desktop, ICON_SIZE, KIcon::DefaultState, 0L, /*canReturnNull=*/true);
//		m_tagsBox->insertItem(emblem, text, index);
//		// Update the mapping:
//		m_tagsMap.insert(index, tag);
//		++index;
//		// Insert sub-states, if needed:
//		if (tag->countStates() > 1) {
//			for (State::List::iterator it2 = tag->states().begin(); it2 != tag->states().end(); ++it2) {
//				state = *it2;
//				// Insert the state:
//				text = state->name();
//				icon = state->emblem();
//				emblem = KIconLoader::global()->loadIcon(icon, KIcon::Desktop, ICON_SIZE, KIcon::DefaultState, 0L, /*canReturnNull=*/true);
//				// Indent the emblem to show the hierarchy relation:
//				if (!emblem.isNull())
//					emblem = Tools::indentPixmap(emblem, /*depth=*/1, /*deltaX=*/2 * ICON_SIZE / 3);
//				m_tagsBox->insertItem(emblem, text, index);
//				// Update the mapping:
//				m_statesMap.insert(index, state);
//				++index;
//			}
//		}
//	}
}

void FilterBar::reset()
{
//	m_lineEdit->setText(""); // m_data->isFiltering will be set to false;
//	if (m_tagsBox->currentItem() != 0) {
//		m_tagsBox->setCurrentItem(0);
//		tagChanged(0);
//	}
}

void FilterBar::filterTag(Tag *tag)
{
//	int index = 0;
//
//	for (QMap<int, Tag*>::Iterator it = m_tagsMap.begin(); it != m_tagsMap.end(); ++it)
//		if (it.data() == tag) {
//			index = it.key();
//			break;
//		}
//	if (index <= 0)
//		return;
//
//	if (m_tagsBox->currentItem() != index) {
//		m_tagsBox->setCurrentItem(index);
//		tagChanged(index);
//	}
}

void FilterBar::filterState(State *state)
{
//	int index = 0;
//
//	for (QMap<int, State*>::Iterator it = m_statesMap.begin(); it != m_statesMap.end(); ++it)
//		if (it.data() == state) {
//			index = it.key();
//			break;
//		}
//	if (index <= 0)
//		return;
//
//	if (m_tagsBox->currentItem() != index) {
//		m_tagsBox->setCurrentItem(index);
//		tagChanged(index);
//	}
}

void FilterBar::inAllBaskets()
{
	// TODO!
}

void FilterBar::setEditFocus()
{
	m_lineEdit->setFocus();
}

bool FilterBar::hasEditFocus()
{
	return m_lineEdit->hasFocus();
}

const FilterData& FilterBar::filterData()
{
	return *m_data;
}

void FilterBar::textChanged(const QString &text)
{
	m_data->string = text;
	m_data->isFiltering = (!m_data->string.isEmpty() || m_data->tagFilterType != FilterData::DontCareTagsFilter);
	m_resetButton->setEnabled(m_data->isFiltering);
	emit newFilter(*m_data);
}

void FilterBar::tagChanged(int index)
{
//	m_data->tag   = 0;
//	m_data->state = 0;
//	switch (index) {
//		case 0:
//			m_data->tagFilterType = FilterData::DontCareTagsFilter;
//			break;
//		case 1:
//			m_data->tagFilterType = FilterData::NotTaggedFilter;
//			break;
//		case 2:
//			m_data->tagFilterType = FilterData::TaggedFilter;
//			break;
//		default:
//			// Try to find if we are filtering a tag:
//			QMapIterator<int, Tag*> it = m_tagsMap.find(index);
//			if (it != m_tagsMap.end()) {
//				m_data->tagFilterType = FilterData::TagFilter;
//				m_data->tag           = *it;
//			} else {
//				// If not, try to find if we are filtering a state:
//				QMapIterator<int, State*> it2 = m_statesMap.find(index);
//				if (it2 != m_statesMap.end()) {
//					m_data->tagFilterType = FilterData::StateFilter;
//					m_data->state         = *it2;
//				} else {
//					// If not (should never happens), do as if the tags filter was reseted:
//					m_data->tagFilterType = FilterData::DontCareTagsFilter;
//				}
//			}
//			break;
//	}
//	m_data->isFiltering = (!m_data->string.isEmpty() || m_data->tagFilterType != FilterData::DontCareTagsFilter);
//	m_resetButton->setEnabled(m_data->isFiltering);
//	emit newFilter(*m_data);
}

#include "filter.moc"
