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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <qstring.h>
#include <kdialogbase.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qtooltip.h>
#include <qcolor.h>
#include <qpoint.h>
#include <qsize.h>

#include "global.h"
#include "container.h"

class QGroupBox;
class QButtonGroup;
class KColorCombo;
class KIntNumInput;
class KGlobalAccel;
class QLabel;

class Container;
class ContainerSystemTray;
class DebugWindow;
class LinkLook;
class LinkLookEditWidget;
class RunCommandRequester;
class IconSizeCombo;

/** Handle all global variables (to avoid lot of extern declarations)
  * @author S�astien Laot
  */
class Settings // FIXME: Distaptch new config events ?
{
  protected:
	/** Main window */
	static bool    s_treeOnLeft;
	static bool    s_filterOnTop;
	static bool    s_playAnimations;
	static bool    s_showNotesToolTip;
	static bool    s_bigNotes;
	static bool    s_exportTextTags;
	static bool    s_useGnuPGAgent;
	static bool    s_usePassivePopup;
	static int     s_middleAction;         // O:Nothing ; 1:Paste ; 2:Text ; 3:Html ; 4:Image ; 5:Link ; 6:Launcher ; 7:Color
	static bool    s_groupOnInsertionLine;
	static bool    s_spellCheckTextNotes;
	static int     s_basketTreeWidth;
	static QPoint  s_mainWindowPosition;
	static QSize   s_mainWindowSize;
	static bool    s_showEmptyBasketInfo;
	static bool    s_blinkedFilter;
	/** Note Addition */
	static int     s_newNotesPlace;        // 0:OnTop ; 1:OnBottom ; 2:AtCurrentNote
	static int     s_viewTextFileContent;
	static int     s_viewHtmlFileContent;
	static int     s_viewImageFileContent;
	static int     s_viewSoundFileContent;
	/** System tray Icon */
	static bool    s_useSystray;
	static bool    s_showIconInSystray;
	static bool    s_startDocked;
	static bool    s_hideOnMouseOut;
	static int     s_timeToHideOnMouseOut;
	static bool    s_showOnMouseIn;
	static int     s_timeToShowOnMouseIn;
	/** Programs */
	static bool    s_textUseProg;
	static bool    s_htmlUseProg;
	static bool    s_imageUseProg;
	static bool    s_animationUseProg;
	static bool    s_soundUseProg;
	static QString s_textProg;
	static QString s_htmlProg;
	static QString s_imageProg;
	static QString s_animationProg;
	static QString s_soundProg;
	/** Insert Note Default Values */
	static int     s_defImageX;
	static int     s_defImageY;
	static int     s_defIconSize;
  public:  /* And the following methods are just getter / setters */
	/** App settings GET */
	static inline bool    treeOnLeft()           { return s_treeOnLeft;           }
	static inline bool    filterOnTop()          { return s_filterOnTop;          }
	static inline bool    playAnimations()       { return s_playAnimations;       }
	static inline bool    showNotesToolTip()     { return s_showNotesToolTip;     }
	static inline bool    bigNotes()             { return s_bigNotes;             }
	static inline bool    exportTextTags()       { return s_exportTextTags;       }
	static inline bool    useGnuPGAgent()        { return s_useGnuPGAgent;        }
	static inline bool    blinkedFilter()        { return s_blinkedFilter;        }
	static inline bool    useSystray()           { return s_useSystray;           }
	static inline bool    showIconInSystray()    { return s_showIconInSystray;    }
	static inline bool    startDocked()          { return s_startDocked;          }
	static inline int     middleAction()         { return s_middleAction;         }
	static inline bool    groupOnInsertionLine() { return s_groupOnInsertionLine; }
	static inline bool    spellCheckTextNotes()  { return s_spellCheckTextNotes;  }
	static inline bool    hideOnMouseOut()       { return s_hideOnMouseOut;       }
	static inline int     timeToHideOnMouseOut() { return s_timeToHideOnMouseOut; }
	static inline bool    showOnMouseIn()        { return s_showOnMouseIn;        }
	static inline int     timeToShowOnMouseIn()  { return s_timeToShowOnMouseIn;  }
	static inline int     basketTreeWidth()      { return s_basketTreeWidth;      }
	static inline int     dropTimeToShow()       { return 7;                      } // TODO: 700 ; TODO: There is certainly a KGlobalConfig ???
	static inline bool    usePassivePopup()      { return s_usePassivePopup;      }
	static inline QPoint  mainWindowPosition()   { return s_mainWindowPosition;   }
	static inline QSize   mainWindowSize()       { return s_mainWindowSize;       }
	static inline bool    showEmptyBasketInfo()  { return s_showEmptyBasketInfo;  }
	/** Programs */
	static inline bool    isTextUseProg()        { return s_textUseProg;          }
	static inline bool    isHtmlUseProg()        { return s_htmlUseProg;          }
	static inline bool    isImageUseProg()       { return s_imageUseProg;         }
	static inline bool    isAnimationUseProg()   { return s_animationUseProg;     }
	static inline bool    isSoundUseProg()       { return s_soundUseProg;         }
	static inline QString textProg()             { return s_textProg;             }
	static inline QString htmlProg()             { return s_htmlProg;             }
	static inline QString imageProg()            { return s_imageProg;            }
	static inline QString animationProg()        { return s_animationProg;        }
	static inline QString soundProg()            { return s_soundProg;            }
	/** Insert Note Default Values */
	static inline int     defImageX()            { return s_defImageX;            }
	static inline int     defImageY()            { return s_defImageY;            }
	static inline int     defIconSize()          { return s_defIconSize;          }
	/** Note Addition */
	static inline int     newNotesPlace()        { return s_newNotesPlace;        }
	static inline int     viewTextFileContent()  { return s_viewTextFileContent;  }
	static inline int     viewHtmlFileContent()  { return s_viewHtmlFileContent;  }
	static inline int     viewImageFileContent() { return s_viewImageFileContent; }
	static inline int     viewSoundFileContent() { return s_viewSoundFileContent; }

	/** App settings SET */
	static void setTreeOnLeft(bool onLeft)
	{
		s_treeOnLeft = onLeft;
		if (Global::basketTree)
			Global::basketTree->setTreePlacement(onLeft);
	}
	static void setFilterOnTop(bool onTop)
	{
		if (s_filterOnTop != onTop) {
			s_filterOnTop = onTop;
			if (Global::basketTree)
				Global::basketTree->filterPlacementChanged(onTop);
		}
	}
	static void setShowNotesToolTip(bool show)
	{
		s_showNotesToolTip = show;
	}
	static void setUseSystray(bool useSystray)
	{
		if (s_useSystray != useSystray) {
			s_useSystray = useSystray;
			if (Global::tray != 0L) {
				if (Settings::useSystray())
					Global::tray->show();
				else {
					Global::tray->hide();
					Global::mainContainer->show();
				}
			}
			if (Global::mainContainer)
				Global::mainContainer->m_actHideWindow->setEnabled(useSystray);
		}
	}
	static void setShowIconInSystray(bool show)
	{
		if (s_showIconInSystray != show) {
			s_showIconInSystray = show;
			if (Global::tray != 0L && Settings::useSystray())
				Global::tray->updateToolTip();
		}
	}
	static void setBigNotes(bool big);
	static inline void setExportTextTags(bool yes)              { s_exportTextTags       = yes;         }
	static inline void setUseGnuPGAgent(bool yes)               { s_useGnuPGAgent        = yes;         }
	static inline void setPlayAnimations(bool play)             { s_playAnimations       = play;        }
	static inline void setBlinkedFilter(bool blinked)           { s_blinkedFilter        = blinked;     }
	static inline void setStartDocked(bool docked)              { s_startDocked          = docked;      }
	static inline void setMiddleAction(int action)              { s_middleAction         = action;      }
	static inline void setGroupOnInsertionLine(bool yes)        { s_groupOnInsertionLine = yes;         }
	static inline void setSpellCheckTextNotes(bool yes)         { s_spellCheckTextNotes  = yes;         }
	static inline void setHideOnMouseOut(bool hide)             { s_hideOnMouseOut       = hide;        }
	static inline void setTimeToHideOnMouseOut(int time)        { s_timeToHideOnMouseOut = time;        }
	static inline void setShowOnMouseIn(bool show)              { s_showOnMouseIn        = show;        }
	static inline void setTimeToShowOnMouseIn(int time)         { s_timeToShowOnMouseIn  = time;        }
	static inline void setBasketTreeWidth(int width)            { s_basketTreeWidth      = width;       }
	static inline void setUsePassivePopup(bool enable)          { s_usePassivePopup      = enable;      }
	static inline void setMainWindowPosition(const QPoint &pos) { s_mainWindowPosition   = pos;         }
	static inline void setMainWindowSize(const QSize &size)     { s_mainWindowSize       = size;        }
	static inline void setShowEmptyBasketInfo(bool show)        { s_showEmptyBasketInfo  = show;        }
	// Programs :
	static inline void setIsTextUseProg(bool useProg)           { s_textUseProg          = useProg;     }
	static inline void setIsHtmlUseProg(bool useProg)           { s_htmlUseProg          = useProg;     }
	static inline void setIsImageUseProg(bool useProg)          { s_imageUseProg         = useProg;     }
	static inline void setIsAnimationUseProg(bool useProg)      { s_animationUseProg     = useProg;     }
	static inline void setIsSoundUseProg(bool useProg)          { s_soundUseProg         = useProg;     }
	static inline void setTextProg(const QString &prog)         { s_textProg             = prog;        }
	static inline void setHtmlProg(const QString &prog)         { s_htmlProg             = prog;        }
	static inline void setImageProg(const QString &prog)        { s_imageProg            = prog;        }
	static inline void setAnimationProg(const QString &prog)    { s_animationProg        = prog;        }
	static inline void setSoundProg(const QString &prog)        { s_soundProg            = prog;        }
	// Insert Note Default Values :
	static inline void setDefImageX(int val)                    { s_defImageX            = val;         }
	static inline void setDefImageY(int val)                    { s_defImageY            = val;         }
	static inline void setDefIconSize(int val)                  { s_defIconSize          = val;         }
	// Note Addition
	static inline void setNewNotesPlace(int val)                { s_newNotesPlace        = val;         }
	static inline void setViewTextFileContent(bool view)        { s_viewTextFileContent  = view;        }
	static inline void setViewHtmlFileContent(bool view)        { s_viewHtmlFileContent  = view;        }
	static inline void setViewImageFileContent(bool view)       { s_viewImageFileContent = view;        }
	static inline void setViewSoundFileContent(bool view)       { s_viewSoundFileContent = view;        }
  public:
	/* Save and load config */
	static void loadConfig();
	static void saveConfig();
  protected:
	static void loadLinkLook(LinkLook *look, const QString &name, const LinkLook &defaultLook);
	static void saveLinkLook(LinkLook *look, const QString &name);
};

/** Provide a dialog for modify application settings
  * @author S�astien Laot
  */
class SettingsDialog : public KDialogBase
{
  Q_OBJECT
  public:
	SettingsDialog(QWidget *parent = 0);
	~SettingsDialog();
  protected slots:
	void visualize();
	void slotDefault();
	void slotApply();
	void slotOk();
  private:
	// Main window
	QComboBox           *m_treeOnLeft;
	QComboBox           *m_filterOnTop;
	QCheckBox           *m_playAnimations;
	QCheckBox           *m_showNotesToolTip;
	QCheckBox           *m_bigNotes;
	QCheckBox           *m_exportTextTags;
	QCheckBox           *m_useGnuPGAgent;
	QCheckBox           *m_usePassivePopup;
	QComboBox           *m_middleAction;
	QCheckBox           *m_groupOnInsertionLine;
	QWidget             *m_groupOnInsertionLineWidget;
	// System Tray Icon
	QCheckBox           *m_useSystray;
	QCheckBox           *m_showIconInSystray;
	QWidget             *m_systray;
	QCheckBox           *m_hideOnMouseOut;
	KIntNumInput        *m_timeToHideOnMouseOut;
	QCheckBox           *m_showOnMouseIn;
	KIntNumInput        *m_timeToShowOnMouseIn;
	// Notes Default Values
	KIntNumInput        *m_imgSizeX;
	KIntNumInput        *m_imgSizeY;
	QPushButton         *m_pushVisualize;
	// Links
	LinkLookEditWidget  *m_soundLook;
	LinkLookEditWidget  *m_fileLook;
	LinkLookEditWidget  *m_localLinkLook;
	LinkLookEditWidget  *m_networkLinkLook;
	LinkLookEditWidget  *m_launcherLook;
	// Programs
	QCheckBox           *m_textUseProg;
	QCheckBox           *m_htmlUseProg;
	QCheckBox           *m_imageUseProg;
	QCheckBox           *m_animationUseProg;
	QCheckBox           *m_soundUseProg;
	RunCommandRequester *m_textProg;
	RunCommandRequester *m_htmlProg;
	RunCommandRequester *m_imageProg;
	RunCommandRequester *m_animationProg;
	RunCommandRequester *m_soundProg;
	// Note Addition
	QComboBox           *m_newNotesPlace;
	QCheckBox           *m_viewTextFileContent;
	QCheckBox           *m_viewHtmlFileContent;
	QCheckBox           *m_viewImageFileContent;
	QCheckBox           *m_viewSoundFileContent;
};

#endif // SETTINGS_H
