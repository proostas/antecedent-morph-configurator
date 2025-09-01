#ifndef CODEGENERATOR_HPP
#define CODEGENERATOR_HPP

#include <QString>
#include <QTextStream>

class Schema;

class CodeGenerator
{
public:
    enum Firmware {ZMKFirmware, QMKFirmware};
public:
    CodeGenerator(Schema *schema, Firmware firmware);
    virtual ~CodeGenerator() = default;

    virtual std::pair<bool,QString> verify() = 0;
    virtual std::pair<bool,QString> prepare() = 0;
    virtual void generate(QTextStream &out) = 0;

protected:
    Schema *m_schema;
    Firmware m_firmware;
};

#endif // CODEGENERATOR_HPP
