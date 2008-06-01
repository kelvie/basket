/***************************************************************************
 *   Copyright (C) 2003 by Sébastien Laoût                                 *
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

#ifndef FILTER_H
#define FILTER_H

#include <qwidget.h>
#include <qmap.h>

#include "focusedwidgets.h"

class QToolButton;

class Tag;
class State;

/** The structure that contain all filter terms
  * @author Sébastien Laoût
  */
class FilterData
{
  public:
	// Useful Enum for tagFilterType:
	enum TagFilterType { DontCareTagsFilter = 0, NotTaggedFilter, TaggedFilter, TagFilter, StateFilter };
	// Constructor and Destructor:
	FilterData()  { isFiltering = false; tagFilterType = DontCareTagsFilter; tag = 0; state = 0; }
	~FilterData() {}
	// Filter data:
	QString  string;
	int      tagFilterType;
	Tag     *tag;
	State   *state;
	bool     isFiltering;
};

/** A QWidget that allow user to enter terms to filter in a Basket.
  * @author Sébastien Laoût
  */
class FilterBar : public QWidget
{
  Q_OBJECT
  public:
	FilterBar(QWidget *parent = 0, const char *name = 0);
	~FilterBar();
	const FilterData& filterData();
  signals:
	void newFilter(const FilterData &data);
	void escapePressed();
	void returnPressed();
  public slots:
	void repopulateTagsComnbo();
	void reset();
	void inAllBaskets();
	void setEditFocus();
	void filterTag(Tag *tag);
	void filterState(State *state);
	void setFilterAll(bool filterAll);
	void setFilterData(const FilterData &data);
  public:
	bool hasEditFocus();
	KLineEdit* lineEdit() { return m_lineEdit; }
  private slots:
	void textChanged(const QString &text);
	void tagChanged(int index);
  private:
	FilterData      *m_data;
	KLineEdit *m_lineEdit;
	QToolButton     *m_resetButton;
	KComboBox *m_tagsBox;
	QToolButton     *m_inAllBasketsButton;

	QMap<int, Tag*>   m_tagsMap;
	QMap<int, State*> m_statesMap;
};

#endif // FILTER_H
