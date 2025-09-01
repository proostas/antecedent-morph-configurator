#ifndef QMKCODEGENERATOR_HPP
#define QMKCODEGENERATOR_HPP

#include "codegenerator.hpp"

class QmkCodeGenerator : public CodeGenerator
{
public:
    QmkCodeGenerator(Schema *schema);
    ~QmkCodeGenerator() override = default;

    std::pair<bool,QString> verify() override;
    std::pair<bool,QString> prepare() override;
    void generate(QTextStream &out) override;
};

#endif // QMKCODEGENERATOR_HPP
