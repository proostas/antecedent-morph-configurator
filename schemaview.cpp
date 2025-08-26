#include "schemaview.hpp"
#include <QAction>
#include <QMenu>
#include <QContextMenuEvent>
#include "lineedit.hpp"
#include <QComboBox>
#include "schemamodel.hpp"

SchemaView::SchemaView(QWidget *parent)
    : QTreeView{parent},
      m_expandAllAction{new QAction{"Expand All", this}},
      m_collapseAllAction{new QAction{"Collapse All", this}},
      m_expandToMorphsAction{new QAction{"Expand to Morphs", this}},
      m_expandThisToMorphsAction{new QAction{"Expand this to Morphs", this}}
{
    setItemDelegateForColumn(SchemaModel::ModeColumn, new ModeDelegate);
    setItemDelegateForColumn(SchemaModel::ValueColumn, new ValueDelegate);
    m_collapseAllAction->setShortcut(QKeySequence{"Ctrl+C"});
    m_collapseAllAction->setShortcutContext(Qt::WindowShortcut);
    addAction(m_collapseAllAction);
    m_expandToMorphsAction->setShortcut(QKeySequence{"Ctrl+M"});
    m_expandToMorphsAction->setShortcutContext(Qt::WindowShortcut);
    addAction(m_expandToMorphsAction);
    m_expandThisToMorphsAction->setShortcut(QKeySequence{"Ctrl+E"});
    m_expandThisToMorphsAction->setShortcutContext(Qt::WindowShortcut);
    addAction(m_expandThisToMorphsAction);

    connect(m_expandAllAction, &QAction::triggered, this, &SchemaView::expandAll);
    connect(m_collapseAllAction, &QAction::triggered, this, &SchemaView::collapseAll);
    connect(m_expandToMorphsAction, &QAction::triggered, this, [this](){ expandToDepth(1); });
    connect(m_expandThisToMorphsAction, &QAction::triggered, this, [this](){ expandRecursively(currentIndex(), 1); });
}

void SchemaView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;
    menu.addAction(m_expandThisToMorphsAction);
    menu.addAction(m_expandAllAction);
    menu.addAction(m_collapseAllAction);
    menu.addAction(m_expandToMorphsAction);
    menu.exec(event->globalPos());
}

ValueDelegate::ValueDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

QWidget *ValueDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto *editor = new LineEdit{parent};
    editor->setFrame(false);

    return editor;
}

void ValueDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.data(Qt::EditRole).toString();
    auto *lineEdit = static_cast<LineEdit*>(editor);
    lineEdit->setText(value);
    // a hack to deselect selected by default text
    QObject hack;
    connect(&hack, &QObject::destroyed, lineEdit, [lineEdit](){
       lineEdit->deselect();
    }, Qt::QueuedConnection);
}

void ValueDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto *lineEdit = static_cast<LineEdit*>(editor);
    model->setData(index, lineEdit->text(), Qt::EditRole);
}

void ValueDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
    static_cast<LineEdit*>(editor)->end(false);
}

ModeDelegate::ModeDelegate(QObject *parent)
    : QStyledItemDelegate{parent}
{

}

QWidget *ModeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);
    auto *editor = new QComboBox{parent};
    editor->setFrame(false);
    editor->addItems(index.data(SchemaModel::Modes).toStringList());
    return editor;
}

void ModeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.data(Qt::EditRole).toInt();
    auto *comboBox = static_cast<QComboBox*>(editor);
    comboBox->setCurrentIndex(value);
}

void ModeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto *comboBox = static_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}

void ModeDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);

    editor->setGeometry(option.rect);
}
