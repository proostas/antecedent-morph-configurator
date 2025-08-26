#ifndef LINEEDIT_HPP
#define LINEEDIT_HPP

#include <QLineEdit>

class LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QAction *m_insertReturnAction;
    QAction *m_insertLeftAction;
};

#endif // LINEEDIT_HPP
