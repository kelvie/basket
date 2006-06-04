/***************************************************************************
 *   Copyright (C) 2005 by Sébastien Laoût                                 *
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

#ifndef KCOLORCOMBO2_H
#define KCOLORCOMBO2_H

#include <qcombobox.h>
#include <qcolor.h>
#include <qpixmap.h>

class KColorPopup;

/**
 * @short A combobox to display or allow user selection of a color in a user-friendly way.
 *
 * A combobox widget that popup an array of colors for the user to easily pick a common color.\n
 * He/she can use the popup to quickly pick a reasonable color or open a color chooser dialog for a more precise choice.\n
 * The user can also choose a default color (the standard background color, text color, etc... it's to the programmer to make sense of this property).\n
 * \n
 * The user is also offered some facilities: like KColorButton he/she can copy a color or paste it
 * (with standard keyboard shortcuts, usually Ctrl+C and Ctrl+V), and he/she can drag or drop colors.
 *
 * @par Quick usage:
 * Just create a new KColorCombo2() with the initial color and eventually an allowed default color
 * (eg. KGlobalSettings::baseColor() for a background color, KGlobalSettings::textColor()...).\n
 * You will be noticed of the color the user selects with the signal changed(), or you can use color() to get the color at any moment.\n
 * Note that they can return an invalid color (see QColor::isValid()) if the user chosen the default color (if he can choose that).\n
 * It's then easy to save in settings, but if you want the real color (even for the default), you can get it with effectiveColor().
 *
 * @par Notes about default color:
 * If you set a default color using Qt or KDE standard colors, the user can change them in the KDE Control Center,
 * but this widget willn't be update and will still show the old one.\n
 * To be noticed of such color change and then update the widget with the new standard color, you can use one of those two methods:
 * @code
 * void QWidgetDerivate::paletteChange(const QPalette &oldPalette) { // QWidgetDerivate is a parent or near custom widget
 *     theComboBox->setDefaultColor(theNewDefaultColor);
 *     QWidget::paletteChange(oldPalette);
 * }
 * @endcode
 * or connect the signal KApplication::kdisplayPaletteChanged() to a slot that will set the default color of this widget.
 *
 * @par Advanced usage:
 * By default, the combobox show a well balanced rainbow, OK for most usages, and you don't need to do anything for it to work.\n
 * You however can set your own color array by calling newColorArray() with the number of columns and rows.
 * Then, setColorAt() several times to fill the array.\n
 * This allow the most flexibility. But if you just want a rainbow with more or less colors, setRainbowPreset() is what you want.\n
 * If you worry about performance issues of creating a combobox with the default color array and then allocating another color array by yourself,
 * note that the default color array is not allocated in the constructor, but as soon as it is demanded (on first popup if no array has been
 * set before, or on first call of any accessors: colorAt(), columnCount(), setColorAt()...).
 * Finally, colorRectPixmap() and drawColorRect() allow to draw the color rounded-rectangle in other places for a consistent look.
 *
 * @see KGlobalSettings Use one of the static functions to get KDE standard colors for default values.
 * @see KColorButton    The same, but without the rainbow popup or the choice of a default color.
 * @see KColorDialog    The dialog that is shown when the user click the "Other..." entry.
 * @author Sébastien Laoût <slaout@linux62.org>
 *
 * @image html commoncolorselector.png "Common Color Selector ComboBox"
 */
class KColorCombo2 : public QComboBox
{
  Q_OBJECT
  Q_PROPERTY(QColor color        READ color        WRITE setColor)
  Q_PROPERTY(QColor defaultColor READ defaultColor WRITE setDefaultColor)

  public slots:
	/**
	 * Change the selected color.\n
	 * If the popup is open, it will not reflect the change. FIXME: Should it?
	 * @param color The new selected color. Can be invalid to select the default one.\n
	 *              If @p color is invalid and no default color is allowed, the function will keep the old one.
	 */
	void setColor(const QColor &color);

	/**
	 * Change the default color.
	 * @param color The color to return if the user choose the default one. If it is not valid, the user willn't be allowed to choose a default one.
	 * @see defaultColor() to get it.
	 */
	void setDefaultColor(const QColor &color);

  signals:
	/**
	 * Emitted when the color of the widget is changed, either with setColor() or via user selection.
	 * @see color() to know the content of @p newColor.
	 */
	void changed(const QColor &newColor);

  public:
	/**
	 * Constructs a color combobox with parent @p parent called @p name.
	 * @param color         The initial selected color. If it is not valid, the default one will then be selected.\n
	 *                      But if @p color is invalid and there is no default color, the result is undefined.
	 * @param defaultColor  The color to return if the user choose the default one. If it is not valid, the user willn't be allowed to choose a default one.
	 */
	KColorCombo2(const QColor &color, const QColor &defaultColor, QWidget *parent = 0, const char *name = 0);

	/**
	 * Constructs a color combobox with parent @p parent called @p name.\n
	 * The user is not allowed to choose a default color, unless you call setDefaultColor() later.
	 * @param color         The initial selected color. If it is invalid, the result is undefined.
	 */
	KColorCombo2(const QColor &color, QWidget *parent = 0L, const char *name = 0L);

	/**
	 * Destroys the combobox.
	 */
	virtual ~KColorCombo2();

	/**
	 * Get the color chosen by the user.\n
	 * Can be invalid, if the user chosen the default one.\n
	 * Ideal to store it in settings for later recall.
	 * @see effectiveColor() if you want the color to be always valid.
	 */
	QColor color() const;

	/**
	 * Return the color chosen by the user.\n
	 * If the user chosen the default color, the default one is then returned, so the returned color is always valid.\n
	 * Ideal to directly use to draw.
	 * @see color() if you want to be notified of a default color choice.
	 */
	QColor effectiveColor() const;

	/**
	 * Returns the default color or an invalid color if no default color is set (if the user isn't allowed to choose a default color).
	 * @see setDefaultColor() to change it.
	 */
	QColor defaultColor() const;

	/**
	 * Allocate a new color array of the specified dimention.\n
	 * The new array will have invalid colors: you should then assign them one by one.\n
	 * If one or both of the dimentions are negative or null, this function do nothing (both dimentions are always ensured to be at least equal to 1).
	 * @param columnCount The number of columns of the array.
	 * @param rowCount    The number of rows of the array.
	 * @see setColorAt() to set all colors once the array have been created.
	 */
	void newColorArray(int columnCount, int rowCount);

	/**
	 * Get the number of columns in the array that the user can see to choose.
	 * @see rowCount() for the number of rows, and colorAt() to get a color from the array.
	 */
	int columnCount() const;

	/**
	 * Get the number of rows in the array that the user can see to choose.
	 * @see columnCount() for the number of columns, and colorAt() to get a color from the array.
	 */
	int rowCount() const;

	/**
	 * Set a color in the array at position (column,row).\n
	 * If one or both of the indexes are out of range, this function do nothing.\n
	 * @p column and @p row start from 0 to columnCount()-1 and columnRow()-1.
	 *
	 * @param column The x coordinate of the color to set or change.
	 * @param row    The y coordinate of the color to set or change.
	 * @param color  The color to assign at this position.
	 */
	void setColorAt(int column, int row, const QColor &color);

	/**
	 * Get a color in the array that the user can see to choose.\n
	 * @p column and @p row start from 0 to columnCount()-1 and columnRow()-1.
	 *
	 * @return The asked color, or an invalid color if the index is out of limit of the array.
	 * @see columnCount() and rowCount() to get the array dimentions.
	 */
	QColor colorAt(int column, int row)/* const*/;

	/**
	 * Fill the array of colors (that will be shown to the user in the popup that appears when he/she click the arrow) with a rainbow of different luminosity.\n
	 * This rainbow representation have the advantage of being natural and well structured for a human to be able to select reasonable colors.\n
	 * This function will allocate a color array by itself depending on the parameters (no need to call newColorArray()).
	 * @param colorColumnCount The number of columns. The 360 possible colors of the rainbow will be splitted to take the wanted number of colors, equaly separated.
	 * @param lightRowCount    There is always at least 1 row of colors: the "pure" colors: pure red, pure blue, pure green, pure fushia...\n
	 *                         Additionnaly, you can add row on top: they will contain the same colors, but lighter.\n
	 *                         The parameter @p lightRowCount specify how many different lighting grades shoud be shown (from near to white, but not white, to "pure").
	 * @param darkRowCount     Finally, on bottom of the row of "pure colors", you can put any variety of dark colors (from "pure", to near to black, but not black).\n
	 *                         So, the number of rows is equal to @p lightRowCount + 1 + @p darkRowCount. On top are light colors, gradually going to dark ones on bottom.
	 * @param withGray         If true, another column (so there will be @p colorColumnCount+1 columns) is added on the very-right of the popup
	 *                         to show different gray values, matching the brightness of the sibling colors.
	 *
	 * The most acceptable parameters:
	 * @li The default values are good to have the 7 colors of the rainbow + colors between them, and light/dark colors are well distinct.
	 * @li If the color is a background color, you can set @p darkRowCount to 0, so only light colors are shown.
	 * @li The inverse is true for text color choice: you can set @p lightRowCount to 0.
	 * @li But be careful: some advanced users prefer white text on dark background, so you eg. can set @p lightRowCount to a big value and
	 *     @p darkRowCount to a small one for a fewer choice of dark colors, but at least some ones.
	 */
	void setRainbowPreset(int colorColumnCount = 12, int lightRowCount = 4, int darkRowCount = 4, bool withGray = true);
	//void setHsvPreset(QColor hue[], QColor saturation[], QColor value[], bool withGray = true);

	/**
	 * Returns a pixmap of a colored rounded-rectangle. The four corners are transparent.\n
	 * Useful if you want to set such a rectangle as an icon for a menu entry, or for drag and drop operation...
	 * @param color     The color of the rectangle. If the color is invalid, a rainbow is then drawn (like for the "Other..." entry in the popup).
	 * @param isDefault True if @p color is the default one and should then be draw with a diagonal line.
	 * @param width     The width of the rectangle pixmap to return.
	 * @param height    The height of the rectangle pixmap to return.
	 *
	 * @see drawColorRect() if you need to draw it directly: it's faster.
	 */
	static QPixmap colorRectPixmap(const QColor &color, bool isDefault, int width, int height);

	/**
	 * Draw an image of a colored rounded-rectangle.\n
	 * This is like colorRectPixmap() but significantly faster because there is nothing to copy, and no transparency mask to create and apply.
	 * @param painter   The painter where to draw the image.
	 * @param x         The x coordinate on the @p painter where to draw the rectangle.
	 * @param y         The y coordinate on the @p painter where to draw the rectangle.
	 * @param color     The color of the rectangle. If the color is invalid, a rainbow is then drawn (like for the "Other..." entry in the popup).
	 * @param isDefault True if @p color is the default one and should then be draw with a diagonal line.
	 * @param width     The width of the rectangle pixmap to return.
	 * @param height    The height of the rectangle pixmap to return.
	 *
	 * @see colorRectPixmap() to get a transparent pixmap of the rectangle.
	 */
	static void drawColorRect(QPainter &painter, int x, int y, const QColor &color, bool isDefault, int width, int height);

	/**
	 * Get the height of a color rectangle for this combobox.\n
	 * This is equal to the text height, regarding to the current font of this combobox.
	 */
	int colorRectHeight() const;

	/**
	 * Get the width of a color rectangle, depending of the @p height of it.\n
	 * It typically return 1.4 * @p height for decent rectangle proportions.
	 */
	int colorRectWidthForHeight(int height) const;

  protected:
	virtual void popup();
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual bool eventFilter(QObject *object, QEvent *event);
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dropEvent(QDropEvent *event);
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void fontChange(const QFont &oldFont);

  private:
	/**
	 * Initialization routine common to every constructors.\n
	 * Constructors just have to initialize the QComboBox, m_color and m_defaultColor
	 * and this function do the rest to complete the creation of this widget.
	 */
	void init();

	/**
	 * Free up all memory allocated for the color array.\n
	 * But only if an array have previously been allocated, of course.
	 */
	void deleteColorArray();

	/**
	 * Update the only item of the combobox to mirror the new selected color.\n
	 * Mainly called on init() and setColor().
	 */
	void updateComboBox();

	KColorPopup *m_popup;
	QColor       m_color;
	QColor       m_defaultColor;
	bool         m_discardNextMousePress;
	QColor     **m_colorArray;
	int          m_columnCount;
	int          m_rowCount;
	QPoint       m_dragStartPos;

  protected:
	/**
	 * Keep place for future improvements without having to break binary compatibility.\n
	 * Does nothing for the moment.
	 */
	virtual void virtual_hook(int id, void *data);

  private:
	/**
	 * Keep place for future improvements without having to break binary compatibility.
	 */
	class KColorCombo2Private;

	KColorCombo2Private *d;
};



// TODO: setColorArray(QColor **, int, int) and use signals/slots ??

class KColorPopup : public QWidget
{
  Q_OBJECT
  public:
	KColorPopup(KColorCombo2 *parent);
	~KColorPopup();
	void relayout(); // updateGeometry() ??
  protected:
	void paintEvent(QPaintEvent */*event*/);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void doSelection();
	void validate();
	void updateCell(int column, int row);

	friend class KColorCombo2;

  private:
	KColorCombo2 *m_selector;
	QPixmap m_pixmap;
	int m_selectedRow;
	int m_selectedColumn;
	int m_columnOther;
	QColor m_otherColor;

	static const int MARGIN;
	static const int FRAME_WIDTH;
};



#endif // KCOLORCOMBO2_H
