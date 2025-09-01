#ifndef ZMKCODEGENERATOR_HPP
#define ZMKCODEGENERATOR_HPP

#include "codegenerator.hpp"
#include "schema.hpp"
#include <QString>

class ZmkCodeGenerator : public CodeGenerator
{
public:
    ZmkCodeGenerator(Schema *schema);
    ~ZmkCodeGenerator() override = default;

    std::pair<bool, QString> verify() override;
    std::pair<bool, QString> prepare() override;
    void generate(QTextStream &out) override;

    static QString zmkKeycode(QString const& c);
private:
    void generateCommentary(QTextStream &out);
    void generateModMorphs(QTextStream &out);
    void generateBehaviors(QTextStream &out);
    void generateBaseBehaviors(QTextStream &out);
    void generateDeepBehaviors(QTextStream &out);
    void generateMacros(QTextStream &out);

    void generateBaseModMorphs(QTextStream &out);
    void generateDeepModMorphs(QTextStream &out);

private:
    QString buildBinding(bool isSingleLettered, QString const &macroLabel, const QString &value) const;
    QString buildMacroLabel(QString const &value, QHash<QString, bool> &usedLabels) const;
    QString buildMacro(QString const &symbol, QString const &label, const QString &value) const;
    QString buildBehavior(LayerType layerType, MorphType morphType, ModType modType,
                          QString const &bindings, QString const &antecedents) const;
    QString buildBehavior(LayerType layerType, MorphType morphType,
                          QString const &bindings, QString const &antecedents, QString const &postfix = {}) const;
    QString buildBehavior(QString const &nodeLabel, QString const &nodeName, QString const &label,
                          QString const &bindings, QString const &antecedents) const;

private:
    struct MacroParams {
        MacroParams(QString const &label, QString const &symbol, SchemaItem *item)
            : label{label}, symbol{symbol}, item{item}
        { }
        QString label;
        QString symbol;
        SchemaItem *item;
    };
    std::unordered_map<SchemaItem*, std::unique_ptr<MacroParams>> m_macros;
    std::vector<MacroParams*> m_orderedMacros;
};

#endif // ZMKCODEGENERATOR_HPP
