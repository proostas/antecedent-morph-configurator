#ifndef CODEGENERATORDIALOG_HPP
#define CODEGENERATORDIALOG_HPP

#include <QDialog>
#include "codegenerator.hpp"
#include "schema.hpp"

class QFormLayout;
class QComboBox;
class QHBoxLayout;
class QLineEdit;
class QToolButton;
class QPlainTextEdit;
class QDialogButtonBox;

class CodeGeneratorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CodeGeneratorDialog(Schema *schema, CodeGenerator::Firmware firmware, QWidget *parent);
    ~CodeGeneratorDialog() override = default;

    void generate();

private:
    Schema *m_schema;
    CodeGenerator::Firmware m_firmware;
    std::unique_ptr<CodeGenerator> m_generator;
    QFormLayout *m_layout;
    QHBoxLayout *m_outputLayout;
    QLineEdit *m_outputPath;
    QToolButton *m_outputPathSelector;
    QPlainTextEdit *m_log;
    QDialogButtonBox *m_buttonBox;
};

#endif // CODEGENERATORDIALOG_HPP
