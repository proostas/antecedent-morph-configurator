#include "lineedit.hpp"
#include <QContextMenuEvent>
#include <QMenu>

LineEdit::LineEdit(QWidget *parent)
    : QLineEdit{parent},
      m_insertReturnAction{new QAction{"Insert ⏎", this}},
      m_insertLeftAction{new QAction{"Insert ←", this}}
{
    m_insertReturnAction->setShortcut(QKeySequence{"Ctrl+R"});
    addAction(m_insertReturnAction);
    m_insertLeftAction->setShortcut(QKeySequence{"Ctrl+L"});
    addAction(m_insertLeftAction);
    connect(m_insertReturnAction, &QAction::triggered, this, [=](){
        insert("⏎");
    });
    connect(m_insertLeftAction, &QAction::triggered, this, [this](){
        insert("←");
    });
}

void LineEdit::contextMenuEvent(QContextMenuEvent *event)
{
    auto *menu = createStandardContextMenu();
    menu->addSeparator();
    menu->addAction(m_insertReturnAction);
    menu->addAction(m_insertLeftAction);
    menu->exec(event->globalPos());
}
