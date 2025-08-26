#ifndef SCHEMAVIEW_HPP
#define SCHEMAVIEW_HPP

#include <QTreeView>
#include <QStyledItemDelegate>

class SchemaView : public QTreeView
{
    Q_OBJECT
public:
    explicit SchemaView(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QAction *m_expandAllAction;
    QAction *m_collapseAllAction;
    QAction *m_expandToMorphsAction;
    QAction *m_expandThisToMorphsAction;
};

class ModeDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ModeDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ValueDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    ValueDelegate(QObject *parent = nullptr);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SCHEMAVIEW_HPP
