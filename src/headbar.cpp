#include "headbar.h"
#include <QtGui/QMouseEvent>
#include <QtCore/QMimeData>
#include "log.h"
#include "fn.h"
HeadBar::HeadBar(QWidget *parent)
  : QToolBar(parent)
{
  setMovable(false);
  setFloatable(false);

  setAcceptDrops(true);

  setContextMenuPolicy(Qt::PreventContextMenu);
  setStyleSheet(
                "QToolBar { background-color: rgb(255, 255, 255); }"
  );


}

bool HeadBar::event(QEvent *event){
  if (event->type() ==  QEvent::MouseButtonPress) {
    emit request_move_window();
  }

  return QToolBar::event(event);
}

void HeadBar::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void HeadBar::dragMoveEvent(QDragMoveEvent *p_event) {
  p_event->acceptProposedAction();
}

void HeadBar::dropEvent(QDropEvent *event)
{
    // format string: iconname;text

    auto dataStrings = event->mimeData()->text().split(";");
    auto act_icon = dataStrings.first();
    auto act_name = dataStrings.at(1);

    auto tempBtn = new QToolButton;

    tempBtn->setDefaultAction(new QAction(QIcon::fromTheme(act_icon),act_name));

    if (act_name == "kde"){
        connect(tempBtn->defaultAction(),&QAction::triggered,
                [=,this]{
                  nlog::Info(FI,LI,"%s : kde pressed",FN);
                });
    } else if (act_name == "okular") {
      connect(tempBtn->defaultAction(),&QAction::triggered,
              [=,this]{
                nlog::Info(FI,LI,"%s : okular pressed",FN);
              });
    }


    addWidget(tempBtn);

    event->accept();
}

QAction * HeadBar::addToolBtn(QAction * action){
  auto tempBtn= new QToolButton();

  tempBtn->setDefaultAction(action);
  tempBtn->defaultAction()->setText(action->text());

  this->addWidget(tempBtn);

  return tempBtn->defaultAction();
}
