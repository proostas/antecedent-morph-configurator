#include "qmkcodegenerator.hpp"


QmkCodeGenerator::QmkCodeGenerator(Schema *schema)
    : CodeGenerator{schema, CodeGenerator::QMKFirmware}
{

}

std::pair<bool, QString> QmkCodeGenerator::verify()
{
    return {false, {}};
}

std::pair<bool, QString> QmkCodeGenerator::prepare()
{
    return {false, {}};
}

void QmkCodeGenerator::generate(QTextStream &out)
{

}
