

#ifndef LIKEBACKBAR_H
#define LIKEBACKBAR_H

#include <QTimer>
#include <QWidget>

#include "likeback.h"

#include "ui_likebackbar.h"



class LikeBackBar : public QWidget, private Ui::LikeBackBar
{
  Q_OBJECT

  public:
             LikeBackBar( LikeBack *likeBack );
            ~LikeBackBar();

  public slots:
    void     startTimer();
    void     stopTimer();

  private slots:
    void     autoMove();
    void     likeClicked();
    void     dislikeClicked();
    void     bugClicked();
    void     featureClicked();

  private:
    LikeBack *m_likeBack;
    QTimer    m_timer;
};

#endif
