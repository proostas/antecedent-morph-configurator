#ifndef SCHEMAPROPERTIESDIALOG_HPP
#define SCHEMAPROPERTIESDIALOG_HPP

#include <QDialog>

class QLineEdit;
class QComboBox;
class QSpacerItem;
class QDialogButtonBox;

class SchemaPropertiesDialog : public QDialog
{
    Q_OBJECT
public:
    explicit SchemaPropertiesDialog(QWidget *parent);
    void setName(QString const &name);
    QString name() const;
    void setVersion(QString const &version);
    QString version() const;
    void setType(int type);
    int type();
    void setPrefix(QString const &prefix);
    QString prefix() const;

private:
    QLineEdit *m_name;
    QLineEdit *m_version;
    QComboBox *m_type;
    QLineEdit *m_prefix;
    QSpacerItem *m_spacer;
    QDialogButtonBox *m_buttonBox;
};

#endif // SCHEMAPROPERTIESDIALOG_HPP
