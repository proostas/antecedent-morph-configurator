#include "codegeneratordialog.hpp"

#include <QFormLayout>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QPlainTextEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>
#include "zmkcodegenerator.hpp"
#include "qmkcodegenerator.hpp"
#include <QFile>

CodeGeneratorDialog::CodeGeneratorDialog(Schema *schema, CodeGenerator::Firmware firmware, QWidget *parent)
    : QDialog{parent},
      m_schema{schema},
      m_firmware{firmware},
      m_layout{new QFormLayout{this}},
      m_outputLayout{new QHBoxLayout},
      m_outputPath{new QLineEdit{this}},
      m_outputPathSelector{new QToolButton{this}},
      m_log{new QPlainTextEdit{this}},
      m_buttonBox{new QDialogButtonBox{this}}
{
    setWindowTitle(QString{"%1 Code Generator"}.arg(m_firmware == CodeGenerator::ZMKFirmware ? "ZMK" : "QMK"));
    resize(600, 500);

    if (m_firmware == CodeGenerator::ZMKFirmware)
        m_generator = std::make_unique<ZmkCodeGenerator>(m_schema);
    else
        m_generator = std::make_unique<QmkCodeGenerator>(m_schema);

    m_outputLayout->addWidget(m_outputPath);
    m_outputPathSelector->setText("...");
    connect(m_outputPathSelector, &QToolButton::clicked, this, [this](){
        QString filePath = QFileDialog::getSaveFileName(
            this, "Destination File Path",
            QStandardPaths::standardLocations(QStandardPaths::HomeLocation).value(0),
            "All Files (*)");
        if (filePath.isEmpty())
            return;

        m_outputPath->setText(filePath);
    });
    m_outputLayout->addWidget(m_outputPathSelector);
    m_layout->addRow("Output: ", m_outputLayout);

    m_log->setReadOnly(true);
    m_layout->setWidget(1, QFormLayout::SpanningRole, m_log);

    m_buttonBox->setStandardButtons(QDialogButtonBox::StandardButton::Save|QDialogButtonBox::StandardButton::Close);
    m_buttonBox->button(QDialogButtonBox::StandardButton::Save)->setText("Generate");
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &CodeGeneratorDialog::generate);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &CodeGeneratorDialog::reject);
    m_layout->setWidget(2, QFormLayout::SpanningRole, m_buttonBox);
}

void CodeGeneratorDialog::generate()
{
    m_log->clear();
    if (m_outputPath->text().isEmpty()) {
        m_log->appendPlainText("Select the destination to write to");
        return;
    }

    auto verifyResult = m_generator->verify();
    if (!verifyResult.first) {
        m_log->appendPlainText(QString{"Verify failed at %1"}.arg(verifyResult.second));
        return;
    } else {
        m_log->appendPlainText("Verify OK");
    }

    auto prepareResult = m_generator->prepare();
    if (!prepareResult.first) {
        m_log->appendPlainText(QString{"Prepare failed at %1"}.arg(verifyResult.second));
        return;
    } else {
        m_log->appendPlainText("Prepare OK");
    }

    QFile file{m_outputPath->text()};
    if (!file.open(QFile::WriteOnly|QFile::Truncate)) {
        m_log->appendPlainText(QString{"Failed to open file: %1"}.arg(file.errorString()));
        return;
    }

    QTextStream out{&file};
    m_generator->generate(out);
}
