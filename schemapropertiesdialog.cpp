#include "schemapropertiesdialog.hpp"
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QDialogButtonBox>

SchemaPropertiesDialog::SchemaPropertiesDialog(QWidget *parent)
    : QDialog{parent},
      m_name{new QLineEdit},
      m_version{new QLineEdit},
      m_type{new QComboBox},
      m_prefix{new QLineEdit},
      m_spacer{new QSpacerItem{20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding}},
      m_buttonBox{new QDialogButtonBox{QDialogButtonBox::Ok|QDialogButtonBox::Cancel}}

{
    setWindowTitle("Schema Properties");
    resize(400, 200);
    auto *layout = new QFormLayout{this};
    layout->addRow("Name:", m_name);
    layout->addRow("Version:", m_version);
    m_type->addItems(QStringList{} << "Flat" << "Deep");
    layout->addRow("Type:", m_type);
    layout->addRow("Prefix:", m_prefix);
    layout->addItem(m_spacer);
    layout->addWidget(m_buttonBox);

    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &SchemaPropertiesDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &SchemaPropertiesDialog::reject);
}

void SchemaPropertiesDialog::setName(const QString &name)
{
    m_name->setText(name);
}

QString SchemaPropertiesDialog::name() const
{
    return m_name->text();
}

void SchemaPropertiesDialog::setVersion(const QString &version)
{
    m_version->setText(version);
}

QString SchemaPropertiesDialog::version() const
{
    return m_version->text();
}

void SchemaPropertiesDialog::setType(int type)
{
    m_type->setCurrentIndex(type);
}

int SchemaPropertiesDialog::type()
{
    return m_type->currentIndex();
}

void SchemaPropertiesDialog::setPrefix(const QString &prefix)
{
    m_prefix->setText(prefix);
}

QString SchemaPropertiesDialog::prefix() const
{
    return m_prefix->text();
}
