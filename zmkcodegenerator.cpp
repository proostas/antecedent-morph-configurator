#include "zmkcodegenerator.hpp"
#include "schema.hpp"

ZmkCodeGenerator::ZmkCodeGenerator(Schema *schema)
    : CodeGenerator{schema, CodeGenerator::ZMKFirmware}
{

}

std::pair<bool, QString> ZmkCodeGenerator::verify()
{
    for (auto const &a: m_schema->m_antecedents) {
        for (auto const &l: a->m_layers) {
            for (auto const &m: l->m_morphs) {
                if (!m->isEmpty() && !m->isValid())
                    return {false, QString{"[%1.%2.%3] Invalid value: '%4'"}
                        .arg(a->name())
                        .arg(l->name())
                        .arg(m->name())
                        .arg(m->value())};
                for (auto i = m->m_value.cbegin(); i != m->m_value.cend(); i++) {
                    if (zmkKeycode(*i).isNull())
                        return {false, QString{"[%1.%2.%3] Invalid symbol: %4"}
                            .arg(a->name())
                            .arg(l->name())
                            .arg(m->name())
                            .arg(*i)};
                }
                for (auto const &md: m->m_mods) {
                    if (!md->isEmpty() && !md->isValid())
                        return {false, QString{"[%1.%2.%3.%4] Invalid value: '%5'"}
                            .arg(a->name())
                            .arg(l->name())
                            .arg(m->name())
                            .arg(md->name())
                            .arg(md->value())};
                    for (auto i = md->m_value.cbegin(); i != md->m_value.cend(); i++) {
                        if (zmkKeycode(*i).isNull())
                            return {false, QString{"[%1.%2.%3.%4] Invalid symbol: %5"}
                                .arg(a->name())
                                .arg(l->name())
                                .arg(m->name())
                                .arg(md->name())
                                .arg(*i)};
                    }
                }
            }
        }
    }
    return {true, {}};
}

std::pair<bool, QString> ZmkCodeGenerator::prepare()
{
    m_orderedMacros.clear();
    m_macros.clear();

    QHash<QString,bool> usedMacroLabels;

    for (auto const &a: m_schema->m_antecedents) {
        for (auto const &l: a->m_layers) {
            for (auto const &m: l->m_morphs) {
                if (!m->isEmpty() && !m->isSingleLettered(a->symbol()) && static_cast<Mode>(m->mode()) != Mode::MacroName) {
                    QString macroLabel = buildMacroLabel(
                                (m->mode() == int(Mode::SchemaName) ? m_schema->fullName() : m->value()),
                                usedMacroLabels);
                    usedMacroLabels[macroLabel] = true;
                    auto macroParams = std::make_unique<MacroParams>(macroLabel, a->symbol(), m.get());
                    m_orderedMacros.push_back(macroParams.get());
                    m_macros[m.get()] = std::move(macroParams);
                }
                for (auto const &md: m->m_mods) {
                    if (!md->isEmpty() && !md->isSingleLettered(a->symbol()) && static_cast<Mode>(m->mode()) != Mode::MacroName) {
                        QString macroLabel = buildMacroLabel(
                                    (md->mode() == int(Mode::SchemaName) ? m_schema->fullName() : md->value()),
                                    usedMacroLabels);
                        usedMacroLabels[macroLabel] = true;
                        auto macroParams = std::make_unique<MacroParams>(macroLabel, a->symbol(), md.get());
                        m_orderedMacros.push_back(macroParams.get());
                        m_macros[md.get()] = std::move(macroParams);
                    }
                }
            }
        }
    }

    return {true, {}};
}

void ZmkCodeGenerator::generate(QTextStream &out)
{
    generateCommentary(out);
    out << "/ {\n";
    out << QString{}.fill(' ', 4) << "behaviors {\n";
    generateModMorphs(out);
    generateBehaviors(out);
    out << QString{}.fill(' ', 4) << "};\n";
    generateMacros(out);
    out << "};\n";
}

QString ZmkCodeGenerator::zmkKeycode(const QString &c)
{
    static QHash<QString,QString> map{
        {"a",  "A"},
        {"b",  "B"},
        {"c",  "C"},
        {"d",  "D"},
        {"e",  "E"},
        {"f",  "F"},
        {"g",  "G"},
        {"h",  "H"},
        {"i",  "I"},
        {"j",  "J"},
        {"k",  "K"},
        {"l",  "L"},
        {"m",  "M"},
        {"n",  "N"},
        {"o",  "O"},
        {"p",  "P"},
        {"q",  "Q"},
        {"r",  "R"},
        {"s",  "S"},
        {"t",  "T"},
        {"u",  "U"},
        {"v",  "V"},
        {"w",  "W"},
        {"x",  "X"},
        {"y",  "Y"},
        {"z",  "Z"},

        {"A",  "LS(A)"},
        {"B",  "LS(B)"},
        {"C",  "LS(C)"},
        {"D",  "LS(D)"},
        {"E",  "LS(E)"},
        {"F",  "LS(F)"},
        {"G",  "LS(G)"},
        {"H",  "LS(H)"},
        {"I",  "LS(I)"},
        {"J",  "LS(J)"},
        {"K",  "LS(K)"},
        {"L",  "LS(L)"},
        {"M",  "LS(M)"},
        {"N",  "LS(N)"},
        {"O",  "LS(O)"},
        {"P",  "LS(P)"},
        {"Q",  "LS(Q)"},
        {"R",  "LS(R)"},
        {"S",  "LS(S)"},
        {"T",  "LS(T)"},
        {"U",  "LS(U)"},
        {"V",  "LS(V)"},
        {"W",  "LS(W)"},
        {"X",  "LS(X)"},
        {"Y",  "LS(Y)"},
        {"Z",  "LS(Z)"},

        {"0",  "N0"},
        {"1",  "N1"},
        {"2",  "N2"},
        {"3",  "N3"},
        {"4",  "N4"},
        {"5",  "N5"},
        {"6",  "N6"},
        {"7",  "N7"},
        {"8",  "N8"},
        {"9",  "N9"},

        {" ",  "SPACE"},
        {"!",  "EXCL"},
        {"@",  "AT"},
        {"#",  "HASH"},
        {"$",  "DLLR"},
        {"%",  "PRCNT"},
        {"^",  "CARET"},
        {"&",  "AMPS"},
        {"*",  "STAR"},
        {"(",  "LPAR"},
        {")",  "RPAR"},
        {"=",  "EQUAL"},
        {"+",  "PLUS"},
        {"-",  "MINUS"},
        {"_",  "UNDER"},
        {"/",  "FSLH"},
        {"?",  "QMARK"},
        {"\\", "BSLH"},
        {"|",  "PIPE"},
        {";", "SEMI"},
        {":",  "COLON"},
        {"'",  "APOS"},
        {"‘",  "APOS"},
        {"’",  "APOS"},
        {"\"", "DQT"},
        {"“",  "DQT"},
        {"”",  "DQT"},
        {",",  "COMMA"},
        {".",  "DOT"},
        {">",  "GT"},
        {"<",  "LT"},
        {"[",  "LBKT"},
        {"]",  "RBKT"},
        {"{",  "LBRC"},
        {"}",  "RBRC"},
        {"`",  "GRAVE"},
        {"~",  "TILDE"},

        {"←",  "LEFT"},
        {"⏎", "RET"},

        {"é",  "E"}
    };

    return map.value(c, QString{});
}

void ZmkCodeGenerator::generateCommentary(QTextStream &out)
{
    out << QString("// %1 schema version %2\n").arg(m_schema->name(), m_schema->version());
    out << "// Automatically generated by Antecedent Morph Configurator\n\n";
}

void ZmkCodeGenerator::generateModMorphs(QTextStream &out)
{
    out << QString{}.fill(' ', 8) << "// Mod-Morphs\n";
    generateBaseModMorphs(out);
    if (m_schema->type() == Schema::Deep)
        generateDeepModMorphs(out);
}

void ZmkCodeGenerator::generateBehaviors(QTextStream &out)
{
    out << QString{}.fill(' ', 8) << "// Antecedent Morphs\n";
    generateBaseBehaviors(out);
    if (m_schema->type() == Schema::Deep)
        generateDeepBehaviors(out);
}

void ZmkCodeGenerator::generateBaseBehaviors(QTextStream &out)
{
    for (int morphType = int(MorphType::NorthEast); morphType <= int(MorphType::SouthWest); ++morphType) {
        QStringList bindings{};
        QStringList antecedents{};
        for (auto const &a: m_schema->m_antecedents) {
            auto *morph = a->getMorph(LayerType::Base, static_cast<MorphType>(morphType));
            if (morph->isEmpty())
                continue;

            auto const macroParams = m_macros.find(morph);
            bindings << (static_cast<Mode>(morph->mode()) == Mode::MacroName
                        ? QString{"<&amstdm_%1>"}.arg(morph->value())
                        : buildBinding(morph->isSingleLettered(a->symbol()),
                                       (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                       morph->value()));
            antecedents << a->zmkCode();
        }
        out << buildBehavior(LayerType::Base, static_cast<MorphType>(morphType),
                             bindings.join(", "), antecedents.join(" "));
        for (int modType = int(ModType::Control); modType <= int(ModType::GUI); ++modType) {
            QStringList bindings{};
            QStringList antecedents{};
            for (auto const &a: m_schema->m_antecedents) {
                auto *mod = a->getMod(LayerType::Base, static_cast<MorphType>(morphType), static_cast<ModType>(modType));
                if (mod->isEmpty())
                    continue;

                auto const macroParams = m_macros.find(mod);
                bindings << (static_cast<Mode>(mod->mode()) == Mode::MacroName
                             ? QString{"<&amstdm_%1>"}.arg(mod->value())
                             : buildBinding(mod->isSingleLettered(a->symbol()),
                                         (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                         mod->value()));
                antecedents << a->zmkCode();
            }
            out << buildBehavior(LayerType::Base, static_cast<MorphType>(morphType), static_cast<ModType>(modType),
                                 bindings.join(", "), antecedents.join(" "));
        }
    }
}

void ZmkCodeGenerator::generateDeepBehaviors(QTextStream &out)
{
    for (int layerType = int(LayerType::Mouse); layerType <= int(LayerType::Media); ++layerType) {
        for (int morphType = int(MorphType::NorthEast); morphType <= int(MorphType::SouthEast); ++morphType) {
            QStringList bindings{};
            QStringList antecedents{};
            for (auto const &a: m_schema->m_antecedents) {
                auto *morph = a->getMorph(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType));
                if (morph->isEmpty())
                    continue;

                auto const macroParams = m_macros.find(morph);
                bindings << (static_cast<Mode>(morph->mode()) == Mode::MacroName
                            ? QString{"<&amstdm_%1>"}.arg(morph->value())
                            : buildBinding(morph->isSingleLettered(a->symbol()),
                                           (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                           morph->value()));
                antecedents << a->zmkCode();
            }
            out << buildBehavior(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType),
                                 bindings.join(", "), antecedents.join(" "));
            for (int modType = int(ModType::Control); modType <= int(ModType::GUI); ++modType) {
                QStringList bindings{};
                QStringList antecedents{};
                for (auto const &a: m_schema->m_antecedents) {
                    auto *mod = a->getMod(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType), static_cast<ModType>(modType));
                    if (mod->isEmpty())
                        continue;

                    auto const macroParams = m_macros.find(mod);
                    bindings << (static_cast<Mode>(mod->mode()) == Mode::MacroName
                                 ? QString{"<&amstdm_%1>"}.arg(mod->value())
                                 : buildBinding(mod->isSingleLettered(a->symbol()),
                                             (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                             mod->value()));
                    antecedents << a->zmkCode();
                }
                out << buildBehavior(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType), static_cast<ModType>(modType),
                                     bindings.join(", "), antecedents.join(" "));
            }
        }
    }
    for (int layerType = int(LayerType::Function); layerType <= int(LayerType::Symbol); ++layerType) {
        for (int morphType = int(MorphType::NorthWest); morphType <= int(MorphType::SouthWest); ++morphType) {
            QStringList bindings{};
            QStringList antecedents{};
            for (auto const &a: m_schema->m_antecedents) {
                auto *morph = a->getMorph(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType));
                if (morph->isEmpty())
                    continue;

                auto const macroParams = m_macros.find(morph);
                bindings << (static_cast<Mode>(morph->mode()) == Mode::MacroName
                            ? QString{"<&amstdm_%1>"}.arg(morph->value())
                            : buildBinding(morph->isSingleLettered(a->symbol()),
                                           (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                           morph->value()));
                antecedents << a->zmkCode();
            }
            out << buildBehavior(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType),
                                 bindings.join(", "), antecedents.join(" "));
            for (int modType = int(ModType::Control); modType <= int(ModType::GUI); ++modType) {
                QStringList bindings{};
                QStringList antecedents{};
                for (auto const &a: m_schema->m_antecedents) {
                    auto *mod = a->getMod(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType), static_cast<ModType>(modType));
                    if (mod->isEmpty())
                        continue;

                    auto const macroParams = m_macros.find(mod);
                    bindings << (static_cast<Mode>(mod->mode()) == Mode::MacroName
                                 ? QString{"<&amstdm_%1>"}.arg(mod->value())
                                 : buildBinding(mod->isSingleLettered(a->symbol()),
                                             (macroParams != m_macros.cend() ? macroParams->second->label : QString{}),
                                             mod->value()));
                    antecedents << a->zmkCode();
                }
                out << buildBehavior(static_cast<LayerType>(layerType), static_cast<MorphType>(morphType), static_cast<ModType>(modType),
                                     bindings.join(", "), antecedents.join(" "));
            }
        }
    }
}

void ZmkCodeGenerator::generateMacros(QTextStream &out)
{
    out << QString{}.fill(' ', 4) << "macros {\n";
    QString symbol{};
    for (auto *m: m_orderedMacros) {
        if (m->symbol != symbol) {
            symbol = m->symbol;
            out << QString{}.fill(' ', 8) << QString{"// '%1'\n"}.arg(symbol);
        }
        QString val{};
        switch (static_cast<Mode>(m->item->mode())) {
            case Mode::Text:
                val = m->item->value();
                break;
            case Mode::SchemaName:
                val = m_schema->fullName();
                break;
            case Mode::MacroName:
                assert(false && "Should not happen");
                break;
        }

        out << buildMacro(m->symbol, m->label, val, m->item->kind() == SchemaItem::Kind::Mod);
    }
    out << QString{}.fill(' ', 4) << "};\n";
}

void ZmkCodeGenerator::generateBaseModMorphs(QTextStream &out)
{
    static QString tmpl = R"TMPL(
        // Base layer
        // NE
        am%1neagm: am%1neagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nea>, <&am%1neg>;
            mods = <(MOD_LGUI)>;
        };
        am%1necagm: am%1necagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nec>, <&am%1neagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1nem: am%1nem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1ne>, <&am%1necagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // E
        am%1eagm: am%1eagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1ea>, <&am%1eg>;
            mods = <(MOD_LGUI)>;
        };
        am%1ecagm: am%1ecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1ec>, <&am%1eagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1em: am%1em {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1e>, <&am%1ecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // SE
        am%1seagm: am%1seagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1sea>, <&am%1seg>;
            mods = <(MOD_LGUI)>;
        };
        am%1secagm: am%1secagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1sec>, <&am%1seagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1sem: am%1sem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1se>, <&am%1secagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // NW
        am%1nwagm: am%1nwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nwa>, <&am%1nwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1nwcagm: am%1nwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nwc>, <&am%1nwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1nwm: am%1nwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nw>, <&am%1nwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // W
        am%1wagm: am%1wagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1wa>, <&am%1wg>;
            mods = <(MOD_RGUI)>;
        };
        am%1wcagm: am%1wcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1wc>, <&am%1wagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1wm: am%1wm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1w>, <&am%1wcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // SW
        am%1swagm: am%1swagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1swa>, <&am%1swg>;
            mods = <(MOD_RGUI)>;
        };
        am%1swcagm: am%1swcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1swc>, <&am%1swagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1swm: am%1swm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1sw>, <&am%1swcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };)TMPL";
    out << tmpl.sliced(1).arg(m_schema->prefix()) << "\n";
}

void ZmkCodeGenerator::generateDeepModMorphs(QTextStream &out)
{
    static QString tmpl = R"TMPL(
        // Mouse layer
        // NE
        am%1mosneagm: am%1mosneagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosnea>, <&am%1mosneg>;
            mods = <(MOD_LGUI)>;
        };
        am%1mosnecagm: am%1mosnecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosnec>, <&am%1mosneagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1mosnem: am%1mosnem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosne>, <&am%1mosnecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // E
        am%1moseagm: am%1moseagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosea>, <&am%1moseg>;
            mods = <(MOD_LGUI)>;
        };
        am%1mosecagm: am%1mosecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosec>, <&am%1moseagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1mosem: am%1mosem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mose>, <&am%1mosecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // SE
        am%1mosseagm: am%1mosseagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mossea>, <&am%1mosseg>;
            mods = <(MOD_LGUI)>;
        };
        am%1mossecagm: am%1mossecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mossec>, <&am%1mosseagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1mossem: am%1mossem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mosse>, <&am%1mossecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // Navigation layer
        // NE
        am%1navneagm: am%1navneagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navnea>, <&am%1navneg>;
            mods = <(MOD_LGUI)>;
        };
        am%1navnecagm: am%1navnecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navnec>, <&am%1navneagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1navnem: am%1navnem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navne>, <&am%1navnecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // E
        am%1naveagm: am%1naveagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navea>, <&am%1naveg>;
            mods = <(MOD_LGUI)>;
        };
        am%1navecagm: am%1navecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navec>, <&am%1naveagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1navem: am%1navem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1nave>, <&am%1navecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // SE
        am%1navseagm: am%1navseagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navsea>, <&am%1navseg>;
            mods = <(MOD_LGUI)>;
        };
        am%1navsecagm: am%1navsecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navsec>, <&am%1navseagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1navsem: am%1navsem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1navse>, <&am%1navsecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // Media layer
        // NE
        am%1medneagm: am%1medneagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mednea>, <&am%1medneg>;
            mods = <(MOD_LGUI)>;
        };
        am%1mednecagm: am%1mednecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mednec>, <&am%1medneagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1mednem: am%1mednem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medne>, <&am%1mednecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // E
        am%1medeagm: am%1medeagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medea>, <&am%1medeg>;
            mods = <(MOD_LGUI)>;
        };
        am%1medecagm: am%1medecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medec>, <&am%1medeagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1medem: am%1medem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1mede>, <&am%1medecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // SE
        am%1medseagm: am%1medseagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medsea>, <&am%1medseg>;
            mods = <(MOD_LGUI)>;
        };
        am%1medsecagm: am%1medsecagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medsec>, <&am%1medseagm>;
            mods = <(MOD_LALT|MOD_LGUI)>;
        };
        am%1medsem: am%1medsem {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1medse>, <&am%1medsecagm>;
            mods = <(MOD_LCTL|MOD_LALT|MOD_LGUI)>;
        };
        // Symbol layer
        // NW
        am%1symnwagm: am%1symnwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symnwa>, <&am%1symnwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1symnwcagm: am%1symnwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symnwc>, <&am%1symnwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1symnwm: am%1symnwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symnw>, <&am%1symnwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // W
        am%1symwagm: am%1symwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symwa>, <&am%1symwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1symwcagm: am%1symwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symwc>, <&am%1symwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1symwm: am%1symwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symw>, <&am%1symwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // SW
        am%1symswagm: am%1symswagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symswa>, <&am%1symswg>;
            mods = <(MOD_RGUI)>;
        };
        am%1symswcagm: am%1symswcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symswc>, <&am%1symswagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1symswm: am%1symswm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1symsw>, <&am%1symswcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // Number layer
        // NW
        am%1numnwagm: am%1numnwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numnwa>, <&am%1numnwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1numnwcagm: am%1numnwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numnwc>, <&am%1numnwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1numnwm: am%1numnwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numnw>, <&am%1numnwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // W
        am%1numwagm: am%1numwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numwa>, <&am%1numwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1numwcagm: am%1numwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numwc>, <&am%1numwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1numwm: am%1numwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numw>, <&am%1numwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // SW
        am%1numswagm: am%1numswagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numswa>, <&am%1numswg>;
            mods = <(MOD_RGUI)>;
        };
        am%1numswcagm: am%1numswcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numswc>, <&am%1numswagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1numswm: am%1numswm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1numsw>, <&am%1numswcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // Function layer
        // NW
        am%1funnwagm: am%1funnwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funnwa>, <&am%1funnwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1funnwcagm: am%1funnwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funnwc>, <&am%1funnwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1funnwm: am%1funnwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funnw>, <&am%1funnwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // W
        am%1funwagm: am%1funwagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funwa>, <&am%1funwg>;
            mods = <(MOD_RGUI)>;
        };
        am%1funwcagm: am%1funwcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funwc>, <&am%1funwagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1funwm: am%1funwm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funw>, <&am%1funwcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };
        // SW
        am%1funswagm: am%1funswagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funswa>, <&am%1funswg>;
            mods = <(MOD_RGUI)>;
        };
        am%1funswcagm: am%1funswcagm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funswc>, <&am%1funswagm>;
            mods = <(MOD_RALT|MOD_RGUI)>;
        };
        am%1funswm: am%1funswm {
            compatible = "zmk,behavior-mod-morph";
            #binding-cells = <0>;
            bindings = <&am%1funsw>, <&am%1funswcagm>;
            mods = <(MOD_RCTL|MOD_RALT|MOD_RGUI)>;
        };)TMPL";
    out << tmpl.sliced(1).arg(m_schema->prefix()) << "\n";
}

QString ZmkCodeGenerator::buildBinding(bool isSingleLettered, const QString &macroLabel, const QString &value) const
{
    if (isSingleLettered)
        return QString{"<&kp %1>"}.arg(zmkKeycode(value.sliced(1,1)));

    return QString{"<&am%1_%2>"}.arg(m_schema->prefix(), macroLabel);
}

QString ZmkCodeGenerator::buildMacroLabel(const QString &value, QHash<QString,bool> &usedLabels) const
{
    static QRegularExpression replaceRe{"[- ]"};
    static QRegularExpression squeezeRe{"(?<=([- ]))\\1"};
    static QRegularExpression clearRe{"[^a-z0-9 -]"};

    auto label = value.toLower().trimmed();
    label.replace(squeezeRe, "");
    label.replace(clearRe, "");
    label = label.trimmed();
    label.replace(replaceRe, "_");

    if (label.length() > 15)
        label = label.first(15);

    label = (label.isEmpty() ? "m" : label);

    int postfix{};
    while (usedLabels.contains(label + (postfix ? QString::number(postfix) : QString{}))) {
        postfix++;
    }

    return label + (postfix ? QString::number(postfix) : QString{});
}

QString ZmkCodeGenerator::buildMacro(const QString &symbol, const QString &label, const QString &value, bool ignoreMods) const
{
    static QString tmpl{R"TMPL(am%1_%2: am%1_%2 {
            compatible = "zmk,behavior-macro";
            #binding-cells = <0>;
            wait-ms = <U_ANTMORPH_MACRO_WAIT>;
            tap-ms = <U_ANTMORPH_MACRO_TAP>;
            // %3%4
            bindings = %5;
        };)TMPL"};

    QString out = tmpl;
    QString pre, firstOp;
    QString val = value;
    if (symbol.toLower() == val.first(1).toLower()) {
        pre = QString("(") + val.first(1).toLower() + ")";
        firstOp = "";
        val = val.sliced(1);
    } else {
        pre = QString("[") + symbol.toLower() + "]";
        firstOp = "&kp BSPC ";
    }

    QString releaseMods{};
    if (ignoreMods) {
        releaseMods += "<&macro_release";
        auto const mods = QStringList{} << "LCTRL" << "RCTRL" << "LALT" << "RALT" << "LGUI" << "RGUI";
        for (auto const &m: mods)
            releaseMods += QString{" &kp %1"}.arg(m);
        releaseMods += ">";
    }

    QString bindings;
    for (auto const &l: val.split(QString())) {
        if (l.isEmpty())
            continue;

        bindings += QString("&kp ") + zmkKeycode(l) + " ";
    }
    QString taps = QString{"<&macro_tap %1>"}.arg(firstOp + bindings.trimmed());
    QString sequence = (!releaseMods.isEmpty() ? releaseMods + ", " : "") + taps;

    return out.arg(m_schema->prefix(), label, pre, val, sequence).prepend(QString{}.fill(' ', 8)).append("\n");
}

QString ZmkCodeGenerator::buildBehavior(LayerType layerType, MorphType morphType, ModType modType,
                                        const QString &bindings, const QString &antecedents) const
{
    QString postfix{};
    switch (modType) {
        case ModType::Control:
            postfix = "c";
            break;
        case ModType::Alt:
            postfix = "a";
            break;
        case ModType::GUI:
            postfix = "g";
    }
    return buildBehavior(layerType, morphType, bindings, antecedents, postfix);
}

QString ZmkCodeGenerator::buildBehavior(LayerType layerType, MorphType morphType,
                                        const QString &bindings, const QString &antecedents, const QString &postfix) const
{
    QString nodeLabel = "am" + m_schema->prefix();
    QString nodeName = "am_" + m_schema->prefix() + "_";
    QString label = "AM_" + m_schema->prefix().toUpper() + "_";
    switch (layerType) {
        case LayerType::Base:
            break;
        case LayerType::Mouse:
            nodeLabel += "mos";
            nodeName += "mos_";
            label += "MOS_";
            break;
        case LayerType::Navigation:
            nodeLabel += "nav";
            nodeName += "nav_";
            label += "NAV_";
            break;
        case LayerType::Media:
            nodeLabel += "med";
            nodeName += "med_";
            label += "MED_";
            break;
        case LayerType::Function:
            nodeLabel += "fun";
            nodeName += "fun_";
            label += "FUN_";
            break;
        case LayerType::Number:
            nodeLabel += "num";
            nodeName += "num_";
            label += "NUM_";
            break;
        case LayerType::Symbol:
            nodeLabel += "sym";
            nodeName += "sym_";
            label += "SYM_";
            break;
    }
    switch (morphType) {
        case MorphType::NorthEast:
            nodeLabel += "ne";
            nodeName += "ne";
            label += "NE";
            break;
        case MorphType::East:
            nodeLabel += "e";
            nodeName += "e";
            label += "E";
            break;
        case MorphType::SouthEast:
            nodeLabel += "se";
            nodeName += "se";
            label += "SE";
            break;
        case MorphType::NorthWest:
            nodeLabel += "nw";
            nodeName += "nw";
            label += "NW";
            break;
        case MorphType::West:
            nodeLabel += "w";
            nodeName += "w";
            label += "W";
            break;
        case MorphType::SouthWest:
            nodeLabel += "sw";
            nodeName += "sw";
            label += "SW";
            break;
    }
    if (!postfix.isEmpty()) {
        nodeLabel += postfix;
        nodeName += "_" + postfix;
        label += "_" + postfix.toUpper();
    }

    return buildBehavior(nodeLabel, nodeName, label, bindings, antecedents);
}

QString ZmkCodeGenerator::buildBehavior(const QString &nodeLabel, const QString &nodeName, const QString &label,
                                        const QString &bindings, const QString &antecedents) const
{
    static QString tmpl = R"TMPL(
        %1: %2 {
            compatible = "zmk,behavior-antecedent-morph";
            label = "%3";
            #binding-cells = <0>;
            defaults = <&none>;
            bindings = %4;
            antecedents = <%5>;
            max-delay-ms = <U_ANTMORPH_DELAY>;
        };)TMPL";

    auto out = tmpl;
    out = out.sliced(1);
    return out.arg(nodeLabel, nodeName, label, (bindings.isEmpty() ? "<&none>" : bindings), (antecedents.isEmpty() ? "0x070100" : antecedents)).append("\n");
}
