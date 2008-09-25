

#ifndef LIKEBACKDIALOG_H
#define LIKEBACKDIALOG_H

#include <KDialog>

#include <QButtonGroup>

#include "likeback.h"

#include "ui_likebackdialog.h"


class LikeBackDialog : public KDialog, private Ui::LikeBackDialog
{
  Q_OBJECT
  public:
  LikeBackDialog(LikeBack::Button reason, const QString &initialComment, const QString &windowPath, const QString &context, LikeBack *likeBack);
  ~LikeBackDialog();
  private:
  LikeBack     *m_likeBack;
  QString       m_windowPath;
  QString       m_context;
  QButtonGroup *m_typeGroup_;
  QString introductionText();
  private slots:
  void slotDefault();
  void slotOk();
  void commentChanged();
  void send();
  void requestFinished(int id, bool error);
};

#endif
