#pragma once

#include <QtWidgets/QToolBar>
#include <QtCore/QEvent>
#include <QtGui/QWindow>
#include <QtWidgets/QToolButton>
#include <QtWidgets/qwidget.h>
#include <QtWidgets/QGraphicsBlurEffect>
#include <QtGui/QPainter>


class HeadBar : public QToolBar
{
    Q_OBJECT
        QPoint m_dragStartPos;
    bool m_dragging = false;
public:
  HeadBar(QWidget *parent = nullptr);

  // give the caller a handle of the default action.

  QAction * addToolBtn(QAction *action);

  bool event(QEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *p_event) override;
  void dropEvent(QDropEvent *event) override;


signals:
  void request_move_window();
  void request_window_resize(Qt::Edges edge);
};
