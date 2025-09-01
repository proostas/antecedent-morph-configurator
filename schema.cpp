#include "schema.hpp"
#include <QJsonObject>
#include <QJsonArray>

SchemaItem::SchemaItem(SchemaItem *parent)
    : m_parent{parent}
{

}

int SchemaItem::row() const
{
    if (!m_parent)
        return 0;

    return m_parent->rowOf(this);
}

SchemaItem *SchemaItem::parent()
{
    return m_parent;
}

QStringList SchemaItem::modes() const
{
    return {};
}

bool SchemaItem::setMode(int mode)
{
    Q_UNUSED(mode)
    return false;
}

int SchemaItem::mode() const
{
    return 0;
}

QString SchemaItem::modeName() const
{
    return {};
}

QString SchemaItem::value() const
{
    return {};
}

bool SchemaItem::isEditable() const
{
    return false;
}

bool SchemaItem::setValue(const QString &value)
{
    Q_UNUSED(value)
    return false;
}

bool SchemaItem::isLeft() const
{
    return !isRight();
}

bool SchemaItem::isRight() const
{
    return m_parent->isRight();
}

int SchemaItem::antecedentType() const
{
    return m_parent->antecedentType();
}

bool SchemaItem::setAntecedentNote(const QString &note)
{
    return m_parent->setAntecedentNote(note);
}

QString SchemaItem::antecedentNote() const
{
    return m_parent->antecedentNote();
}

Modifier SchemaItem::pressedModifier() const
{
    return NoModifier;
}

Schema::Schema(Type type, SchemaItem *parent)
    : SchemaItem{parent},
      m_filePath{},
      m_name{},
      m_version{},
      m_type{type},
      m_prefix{},
      m_antecedents{},
      m_changed{false}
{
    m_antecedents.reserve(Antecedent::Space+1);
    for (int type = Antecedent::A; type <= Antecedent::Space; ++type)
        m_antecedents.push_back(std::make_unique<Antecedent>(static_cast<Antecedent::Type>(type), this));
}

bool Schema::isNew() const
{
    return m_filePath.isEmpty();
}

bool Schema::setFilePath(QString filePath)
{
    if (m_filePath == filePath)
        return false;

    m_filePath = filePath;
    return true;
}

QString Schema::filePath() const
{
    return m_filePath;
}

QString Schema::suggestedFileName() const
{
    return QString{"Antecedent Morph %1 Schema %2.amconf"}.arg(m_name).arg(m_version);
}

bool Schema::fromJson(const QJsonDocument &jsonDoc)
{
    if (!jsonDoc.isObject())
        return false;

    auto schema = jsonDoc.object();
    m_name = schema["name"].toString();
    m_version = schema["version"].toString();
    m_type = static_cast<Type>(schema["type"].toInt());
    m_prefix = schema["prefix"].toString();
    m_changed = false;

    auto antecedents = schema["antecedents"].toObject();
    for (auto &a : m_antecedents) {
        auto const key = QString::number(a->type());
        if (antecedents.contains(key))
            a->fromJson(antecedents[key].toObject());
    }

    return true;
}

QJsonDocument Schema::toJson()
{
    QJsonObject schema;
    schema["format"] = 1;
    schema["name"] = m_name;
    schema["version"] = m_version;
    schema["type"] = m_type;
    schema["prefix"] = m_prefix;
    QJsonObject antecedents;
    for (auto const &a : m_antecedents) {
        antecedents[QString::number(a->type())] = a->toJson(m_type);
    }
    schema["antecedents"] = antecedents;

    return QJsonDocument{schema};
}

void Schema::clear()
{
    m_filePath.clear();
    m_name.clear();
    m_version.clear();
    m_type = Flat;
    m_prefix.clear();
    m_changed = false;
    std::for_each(m_antecedents.cbegin(), m_antecedents.cend(),
                  [](std::unique_ptr<Antecedent> const &a) { a->clear(); });
}

bool Schema::setName(const QString &name)
{
    if (m_name == name)
        return false;

    m_name = name;
    m_changed = true;
    return true;
}

QString Schema::fullName() const
{
    return QString{"%1 %2"}.arg(m_name).arg(m_version);
}

bool Schema::setVersion(const QString &version)
{
    if (m_version == version)
        return false;

    m_version = version;
    m_changed = true;
    return true;
}

QString Schema::version() const
{
    return m_version;
}

bool Schema::setType(Type type)
{
    if (m_type == type)
        return false;

    m_type = type;
    m_changed = true;
    return true;
}

bool Schema::setPrefix(const QString &prefix)
{
    if (m_prefix == prefix)
        return false;

    m_prefix = prefix;
    m_changed = true;
    return true;
}

QString Schema::prefix() const
{
    return m_prefix;
}

bool Schema::isEmpty(LayerType layerType, MorphType morphType) const
{
    for (auto const &a: m_antecedents) {
        if (!a->isEmpty(layerType, morphType))
            return false;
    }

    return true;
}

bool Schema::isEmpty(LayerType layerType, MorphType morphType, ModType modType) const
{
    for (auto const &a: m_antecedents) {
        if (!a->isEmpty(layerType, morphType, modType))
            return false;
    }

    return true;
}

SchemaItem::Kind Schema::kind() const
{
    return SchemaItem::Kind::Schema;
}

Schema::Type Schema::type() const
{
    return m_type;
}

SchemaItem *Schema::child(int row)
{
    return m_antecedents[row].get();
}

int Schema::childCount(int schemaType) const
{
    Q_UNUSED(schemaType)
    return int(m_antecedents.size());
}

int Schema::itemType() const
{
    return m_type;
}

QString Schema::name() const
{
    return m_name;
}

bool Schema::isRight() const
{
    return false;
}

bool Schema::isChanged() const
{
    if (m_changed)
        return true;

    for (auto const &a : m_antecedents)
        if (a->isChanged())
            return true;

    return false;
}

void Schema::clearChanged()
{
    m_changed = false;
    for (auto &a : m_antecedents)
        a->clearChanged();
}

int Schema::antecedentType() const
{
    return -1;
}

int Schema::rowOf(SchemaItem const *me) const
{
    auto const it = std::find_if(m_antecedents.cbegin(), m_antecedents.cend(),
                        [me](std::unique_ptr<Antecedent> const &item) {
                            return item.get() == me;
                        });
    if (it != m_antecedents.cend())
        return std::distance(m_antecedents.cbegin(), it);

    Q_ASSERT(false && "Should not happen");
    return -1;
}

Antecedent::Antecedent(Type type, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_layers{},
      m_note{},
      m_changed{false}
{
    m_layers.push_back(std::make_unique<Layer>(LayerType::Base, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Mouse, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Navigation, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Media, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Function, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Number, this));
    m_layers.push_back(std::make_unique<Layer>(LayerType::Symbol, this));
}

bool Antecedent::fromJson(const QJsonObject &json)
{
    m_type = static_cast<Type>(json["type"].toInt());
    m_note = json["note"].toVariant().toString();
    m_changed = false;

    int i{0};
    auto const layers = json["layers"].toArray();
    for (auto const &l: layers) {
        m_layers[i]->fromJson(l.toObject());
        ++i;
    }
    return true;
}

QJsonObject Antecedent::toJson(int schemaType) const
{
    QJsonArray layers;
    for (int i = 0; i < childCount(schemaType); ++i) {
        layers << m_layers[i]->toJson();
    }
    QJsonObject antecedent;
    antecedent["type"] = m_type;
    antecedent["note"] = m_note;
    antecedent["layers"] = layers;
    return antecedent;
}

void Antecedent::clear()
{
    m_note.clear();
    m_changed = false;
    std::for_each(m_layers.cbegin(), m_layers.cend(),
                  [](std::unique_ptr<Layer> const &l) { l->clear(); });
}

bool Antecedent::isEmpty(LayerType layerType, MorphType morphType) const
{
    return m_layers[static_cast<int>(layerType)]->isEmpty(morphType);
}

bool Antecedent::isEmpty(LayerType layerType, MorphType morphType, ModType modType) const
{
    return m_layers[static_cast<int>(layerType)]->isEmpty(morphType, modType);
}

Morph *Antecedent::getMorph(LayerType layerType, MorphType morphType) const
{
    return m_layers[static_cast<int>(layerType)]->getMorph(morphType);
}

Mod *Antecedent::getMod(LayerType layerType, MorphType morphType, ModType modType) const
{
    return m_layers[static_cast<int>(layerType)]->getMod(morphType, modType);
}

SchemaItem::Kind Antecedent::kind() const
{
    return SchemaItem::Kind::Antecedent;
}

Antecedent::Type Antecedent::type() const
{
    return m_type;
}

SchemaItem *Antecedent::child(int row)
{
    return m_layers[row].get();
}

int Antecedent::childCount(int schemaType) const
{
    if (schemaType == Schema::Flat)
        return 1;

    return int(m_layers.size());
}

int Antecedent::itemType() const
{
    return m_type;
}

QString Antecedent::name() const
{
    return symbol(m_type);
}

bool Antecedent::isRight() const
{
    switch (m_type) {
        case J:
        case L:
        case U:
        case Y:
        case Quote:
        case M:
        case N:
        case E:
        case I:
        case O:
        case K:
        case H:
        case Comma:
        case Dot:
        case Slash:
        case Space:
            return true;
        default:
            return false;
    }
}

bool Antecedent::isChanged() const
{
    if (m_changed)
        return true;

    for (auto const &l : m_layers)
        if (l->isChanged())
            return true;

    return false;
}

void Antecedent::clearChanged()
{
    m_changed = false;
    for (auto &l : m_layers)
        l->clearChanged();
}

int Antecedent::antecedentType() const
{
    return m_type;
}

bool Antecedent::setAntecedentNote(const QString &note)
{
    if (m_note == note)
        return false;

    m_note = note;
    m_changed = true;
    return true;
}

QString Antecedent::antecedentNote() const
{
    return m_note;
}

QString Antecedent::symbol() const
{
    return symbol(m_type);
}

QString Antecedent::zmkCode() const
{
    return ZMKCode(m_type);
}

QString Antecedent::symbol(Type type)
{
    switch (type) {
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "J";
        case K: return "K";
        case L: return "L";
        case M: return "M";
        case N: return "N";
        case O: return "O";
        case P: return "P";
        case Q: return "Q";
        case R: return "R";
        case S: return "S";
        case T: return "T";
        case U: return "U";
        case V: return "V";
        case W: return "W";
        case X: return "X";
        case Y: return "Y";
        case Z: return "Z";
        case N0: return "0";
        case N1: return "1";
        case N2: return "2";
        case N3: return "3";
        case N4: return "4";
        case N5: return "5";
        case N6: return "6";
        case N7: return "7";
        case N8: return "8";
        case N9: return "9";
        case Comma: return ",";
        case Dot: return ".";
        case Quote: return "'";
        case Slash: return "/";
        case LBrace: return "{";
        case RBrace: return "}";
        case LParen: return "(";
        case RParen: return ")";
        case Star: return "*";
        case Colon: return ":";
        case Dollar: return "$";
        case Percent: return "%";
        case Caret: return "^";
        case Plus: return "+";
        case Tilda: return "~";
        case Exclamation: return "!";
        case At: return "@";
        case Hash: return "#";
        case Pipe: return "|";
        case Ampersand: return "&";
        case Underscore: return "_";
        case LBracket: return "[";
        case RBracket: return "]";
        case Semicolon: return ";";
        case Grave: return "`";
        case Equal: return "=";
        case Backslash: return "\\";
        case Minus: return "-";
        case Space: return " ";
    }
    assert(false && "Should not happen");
    return {};
}

QString Antecedent::ZMKCode(Type type)
{
    switch (type) {
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "J";
        case K: return "K";
        case L: return "L";
        case M: return "M";
        case N: return "N";
        case O: return "O";
        case P: return "P";
        case Q: return "Q";
        case R: return "R";
        case S: return "S";
        case T: return "T";
        case U: return "U";
        case V: return "V";
        case W: return "W";
        case X: return "X";
        case Y: return "Y";
        case Z: return "Z";
        case N0: return "N0";
        case N1: return "N1";
        case N2: return "N2";
        case N3: return "N3";
        case N4: return "N4";
        case N5: return "N5";
        case N6: return "N6";
        case N7: return "N7";
        case N8: return "N8";
        case N9: return "N9";
        case Comma: return "COMMA";
        case Dot: return "DOT";
        case Quote: return "SQT";
        case Slash: return "FSLH";
        case LBrace: return "LBRC";
        case RBrace: return "RBRC";
        case LParen: return "LPAR";
        case RParen: return "RPAR";
        case Star: return "STAR";
        case Colon: return "COLON";
        case Dollar: return "DLLR";
        case Percent: return "PRCNT";
        case Caret: return "CARET";
        case Plus: return "PLUS";
        case Tilda: return "TILDE";
        case Exclamation: return "EXCL";
        case At: return "AT";
        case Hash: return "HASH";
        case Pipe: return "PIPE";
        case Ampersand: return "AMPS";
        case Underscore: return "UNDER";
        case LBracket: return "LBKT";
        case RBracket: return "RBKT";
        case Semicolon: return "SEMI";
        case Grave: return "GRAVE";
        case Equal: return "EQUAL";
        case Backslash: return "BSLH";
        case Minus: return "MINUS";
        case Space: return "SPACE";
    }
    assert(false && "Should not happen");
    return {};
}

QString Antecedent::QMKCode(Type type)
{
    switch (type) {
        case A: return "KC_A";
        case B: return "KC_B";
        case C: return "KC_C";
        case D: return "KC_D";
        case E: return "KC_E";
        case F: return "KC_F";
        case G: return "KC_G";
        case H: return "KC_H";
        case I: return "KC_I";
        case J: return "KC_J";
        case K: return "KC_K";
        case L: return "KC_L";
        case M: return "KC_M";
        case N: return "KC_N";
        case O: return "KC_O";
        case P: return "KC_P";
        case Q: return "KC_Q";
        case R: return "KC_R";
        case S: return "KC_S";
        case T: return "KC_T";
        case U: return "KC_U";
        case V: return "KC_V";
        case W: return "KC_W";
        case X: return "KC_X";
        case Y: return "KC_Y";
        case Z: return "KC_Z";
        case N0: return "KC_0";
        case N1: return "KC_1";
        case N2: return "KC_2";
        case N3: return "KC_3";
        case N4: return "KC_4";
        case N5: return "KC_5";
        case N6: return "KC_6";
        case N7: return "KC_7";
        case N8: return "KC_8";
        case N9: return "KC_9";
        case Comma: return "KC_COMM";
        case Dot: return "KC_DOT";
        case Quote: return "KC_QUOT";
        case Slash: return "KC_SLSH";
        case LBrace: return "KC_LCBR";
        case RBrace: return "KC_RCBR";
        case LParen: return "KC_LPRN";
        case RParen: return "KC_RPRN";
        case Star: return "KC_ASTR";
        case Colon: return "KC_COLN";
        case Dollar: return "KC_DLR";
        case Percent: return "KC_PERC";
        case Caret: return "KC_CIRC";
        case Plus: return "KC_PLUS";
        case Tilda: return "KC_TILD";
        case Exclamation: return "KC_EXLM";
        case At: return "KC_AT";
        case Hash: return "KC_HASH";
        case Pipe: return "KC_PIPE";
        case Ampersand: return "KC_AMPR";
        case Underscore: return "KC_UNDS";
        case LBracket: return "KC_LBRC";
        case RBracket: return "KC_RBRC";
        case Semicolon: return "KC_SCLN";
        case Grave: return "KC_GRV";
        case Equal: return "KC_EQL";
        case Backslash: return "KC_BSLS";
        case Minus: return "KC_MINS";
        case Space: return "KC_SPC";
    }
    assert(false && "Should not happen");
    return {};
}

int Antecedent::rowOf(const SchemaItem *me) const
{
    auto const it = std::find_if(m_layers.cbegin(), m_layers.cend(),
                        [me](std::unique_ptr<Layer> const &item) {
                            return item.get() == me;
                        });
    if (it != m_layers.cend())
        return std::distance(m_layers.cbegin(), it);

    Q_ASSERT(false && "Should not happen");
    return -1;
}

Layer::Layer(LayerType type, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_morphs{}
{
    if (type == LayerType::Base || type == LayerType::Mouse || type == LayerType::Navigation || type == LayerType::Media) {
        m_morphs.push_back(std::make_unique<Morph>(MorphType::NorthEast, Mode::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(MorphType::East, Mode::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(MorphType::SouthEast, Mode::Text, this));
    }
    if (type == LayerType::Base || type == LayerType::Symbol || type == LayerType::Number || type == LayerType::Function) {
        m_morphs.push_back(std::make_unique<Morph>(MorphType::NorthWest, Mode::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(MorphType::West, Mode::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(MorphType::SouthWest, Mode::Text, this));
    }
}

bool Layer::fromJson(const QJsonObject &json)
{
    m_type = static_cast<LayerType>(json["type"].toInt());
    int i{0};
    auto const morphs = json["morphs"].toArray();
    for (auto const &m: morphs) {
        m_morphs[i]->fromJson(m.toObject());
        ++i;
    }
    return true;
}

QJsonObject Layer::toJson() const
{
    QJsonArray morphs;
    for (auto const &m : m_morphs) {
        morphs << m->toJson();
    }
    QJsonObject layer;
    layer["type"] = int(m_type);
    layer["morphs"] = morphs;
    return layer;
}

void Layer::clear()
{
    std::for_each(m_morphs.cbegin(), m_morphs.cend(),
                  [](std::unique_ptr<Morph> const &m) { m->clear(); });
}

bool Layer::isEmpty(MorphType morphType) const
{
    int idx = int(morphType);
    if (m_type == LayerType::Symbol
      || m_type == LayerType::Number
      || m_type == LayerType::Function)
    {
        idx -= 3;
    }
    return m_morphs[idx]->isEmpty();
}

bool Layer::isEmpty(MorphType morphType, ModType modType) const
{
    int idx = int(morphType);
    if (m_type == LayerType::Symbol
      || m_type == LayerType::Number
      || m_type == LayerType::Function)
    {
        idx -= 3;
    }
    return m_morphs[idx]->isEmpty(modType);
}

Morph *Layer::getMorph(MorphType morphType) const
{
    int idx = int(morphType);
    if (m_type == LayerType::Symbol
      || m_type == LayerType::Number
      || m_type == LayerType::Function)
    {
        idx -= 3;
    }
    return m_morphs[idx].get();
}

Mod *Layer::getMod(MorphType morphType, ModType modType) const
{
    int idx = int(morphType);
    if (m_type == LayerType::Symbol
      || m_type == LayerType::Number
      || m_type == LayerType::Function)
    {
        idx -= 3;
    }
    return m_morphs[idx]->getMod(modType);
}

SchemaItem::Kind Layer::kind() const
{
    return SchemaItem::Kind::Layer;
}

SchemaItem *Layer::child(int row)
{
    return m_morphs[row].get();
}

int Layer::childCount(int schemaType) const
{
    Q_UNUSED(schemaType);
    return int(m_morphs.size());
}

int Layer::itemType() const
{
    return int(m_type);
}

QString Layer::name() const
{
    switch (m_type) {
        case LayerType::Base: return "Base";
        case LayerType::Mouse: return "Mouse";
        case LayerType::Navigation: return "Navigation";
        case LayerType::Media: return "Media";
        case LayerType::Function: return "Function";
        case LayerType::Number: return "Number";
        case LayerType::Symbol: return "Symbol";
    }

    assert(false && "Should not happen");
    return {};
}

bool Layer::isChanged() const
{
    for (auto const &m : m_morphs)
        if (m->isChanged())
            return true;

    return false;
}

void Layer::clearChanged()
{
    for (auto &m : m_morphs)
        m->clearChanged();
}

int Layer::rowOf(const SchemaItem *me) const
{
    auto const it = std::find_if(m_morphs.cbegin(), m_morphs.cend(),
                        [me](std::unique_ptr<Morph> const &item) {
                            return item.get() == me;
                        });
    if (it != m_morphs.cend())
        return std::distance(m_morphs.cbegin(), it);

    Q_ASSERT(false && "Should not happen");
    return -1;
}

Morph::Morph(MorphType type, Mode mode, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_mode{mode},
      m_mods{},
      m_value{},
      m_changed{false}
{
    m_mods.push_back(std::make_unique<Mod>(ModType::Control, Mode::Text, this));
    m_mods.push_back(std::make_unique<Mod>(ModType::Alt, Mode::Text, this));
    m_mods.push_back(std::make_unique<Mod>(ModType::GUI, Mode::Text, this));
}

bool Morph::fromJson(const QJsonObject &json)
{
    m_type = static_cast<MorphType>(json["type"].toInt());
    m_mode = static_cast<Mode>(json["mode"].toInt());
    m_value = json["value"].toString();
    m_changed = false;

    int i{0};
    auto const mods = json["mods"].toArray();
    for (auto const &m: mods) {
        m_mods[i]->fromJson(m.toObject());
        ++i;
    }
    return true;
}

QJsonObject Morph::toJson() const
{
    QJsonArray mods;
    for (auto const &m: m_mods) {
        mods << m->toJson();
    }
    QJsonObject morph;
    morph["type"] = static_cast<int>(m_type);
    morph["mode"] = static_cast<int>(m_mode);
    morph["value"] = m_value;
    morph["mods"] = mods;
    return morph;
}

void Morph::clear()
{
    m_mode = Mode::Text;
    m_value.clear();
    m_changed = false;
    std::for_each(m_mods.cbegin(), m_mods.cend(),
                  [](std::unique_ptr<Mod> const &m) { m->clear(); });
}

bool Morph::isEmpty() const
{
    if (m_mode == Mode::SchemaName)
        return false;

    return m_value.isEmpty();
}

bool Morph::isEmpty(ModType modType) const
{
    return m_mods[static_cast<int>(modType)]->isEmpty();
}

bool Morph::isValid() const
{
    if (m_mode == Mode::SchemaName)
        return true;

    return m_value.size() != 1;
}

Mod *Morph::getMod(ModType modType) const
{
    return m_mods[static_cast<int>(modType)].get();
}

bool Morph::isSingleLettered(QString const &symbol) const
{
    return m_mode == Mode::Text && m_value.length() == 2 && m_value.first(1).toLower() == symbol.toLower();
}

SchemaItem::Kind Morph::kind() const
{
    return SchemaItem::Kind::Morph;
}

SchemaItem *Morph::child(int row)
{
    return m_mods[row].get();
}

int Morph::childCount(int schemaType) const
{
    Q_UNUSED(schemaType)
    return int(m_mods.size());
}

int Morph::itemType() const
{
    return static_cast<int>(m_type);
}

QString Morph::name() const
{
    switch (m_type) {
        case MorphType::NorthEast: return "↗️";
        case MorphType::East: return "➡️";
        case MorphType::SouthEast: return "↘️";
        case MorphType::NorthWest: return "↖️";
        case MorphType::West: return "⬅️";
        case MorphType::SouthWest: return "↙️";
    }
    assert(false && "Should not happen");
    return {};
}

QStringList Morph::modes() const
{
    return {"Text", "Macro", "SchName"};
}

bool Morph::setMode(int mode)
{
    if (m_mode == static_cast<Mode>(mode))
        return false;

    m_mode = static_cast<Mode>(mode);
    m_changed = true;
    return true;
}

int Morph::mode() const
{
    return static_cast<int>(m_mode);
}

QString Morph::modeName() const
{
    switch (m_mode) {
        case Mode::Text: return "Text";
        case Mode::MacroName: return "Macro";
        case Mode::SchemaName: return "SchName";
    }
    assert(false && "Should not happen");
    return {};
}

QString Morph::value() const
{
    return m_value;
}

bool Morph::isEditable() const
{
    return true;
}

bool Morph::setValue(const QString &value)
{
    if (m_value == value)
        return false;

    m_value = value;
    m_changed = true;
    return true;
}

bool Morph::isChanged() const
{
    if (m_changed)
        return true;

    for (auto const &m : m_mods)
        if (m->isChanged())
            return true;

    return false;
}

void Morph::clearChanged()
{
    m_changed = false;
    for (auto &m : m_mods)
        m->clearChanged();
}

int Morph::rowOf(const SchemaItem *me) const
{
    auto const it = std::find_if(m_mods.cbegin(), m_mods.cend(),
                        [me](std::unique_ptr<Mod> const &item) {
                            return item.get() == me;
                        });
    if (it != m_mods.cend())
        return std::distance(m_mods.cbegin(), it);

    Q_ASSERT(false && "Should not happen");
    return -1;
}

Mod::Mod(ModType type, Mode mode, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_mode{mode},
      m_value{},
      m_changed{false}
{

}

bool Mod::fromJson(const QJsonObject &json)
{
    m_type = static_cast<ModType>(json["type"].toInt());
    m_mode = static_cast<Mode>(json["mode"].toInt());
    m_value = json["value"].toString();
    m_changed = false;
    return true;
}

QJsonObject Mod::toJson() const
{
    QJsonObject mod;
    mod["type"] = static_cast<int>(m_type);
    mod["mode"] = static_cast<int>(m_mode);
    mod["value"] = m_value;
    return mod;
}

void Mod::clear()
{
    m_mode = Mode::Text;
    m_value.clear();
    m_changed = false;
}

bool Mod::isEmpty() const
{
    if (m_mode == Mode::SchemaName)
        return false;

    return m_value.isEmpty();
}

bool Mod::isValid() const
{
    if (m_mode == Mode::SchemaName)
        return true;

    return m_value.size() != 1;
}

bool Mod::isSingleLettered(const QString &symbol) const
{
    return m_mode == Mode::Text && m_value.length() == 2 && m_value.first(1).toLower() == symbol.toLower();
}

SchemaItem::Kind Mod::kind() const
{
    return SchemaItem::Kind::Mod;
}

SchemaItem *Mod::child(int row)
{
    return nullptr;
}

int Mod::childCount(int schemaType) const
{
    Q_UNUSED(schemaType);
    return 0;
}

int Mod::itemType() const
{
    return static_cast<int>(m_type);
}

QString Mod::name() const
{
    switch (m_type) {
        case ModType::Control: return "Ctrl";
        case ModType::Alt: return "Alt";
        case ModType::GUI: return "GUI";
    }
    assert(false && "Should not happen");
    return {};
}

QStringList Mod::modes() const
{
    return m_parent->modes();
}

bool Mod::setMode(int mode)
{
    if (m_mode == static_cast<Mode>(mode))
        return false;

    m_mode = static_cast<Mode>(mode);
    m_changed = true;
    return true;
}

int Mod::mode() const
{
    return static_cast<int>(m_mode);
}

QString Mod::modeName() const
{
    switch (m_mode) {
        case Mode::Text: return "Text";
        case Mode::MacroName: return "Macro";
        case Mode::SchemaName: return "SchName";
    }
    assert(false && "Should not happen");
    return {};
}

QString Mod::value() const
{
    return m_value;
}

bool Mod::isEditable() const
{
    return true;
}

bool Mod::setValue(const QString &value)
{
    if (m_value == value)
        return false;

    m_value = value;
    m_changed = true;
    return true;
}

bool Mod::isChanged() const
{
    return m_changed;
}

void Mod::clearChanged()
{
    m_changed = false;
}

Modifier Mod::pressedModifier() const
{
    switch (static_cast<MorphType>(m_parent->itemType())) {
        case MorphType::NorthEast:
        case MorphType::East:
        case MorphType::SouthEast:
            switch (m_type) {
                case ModType::Control:
                    return LCTRL;
                case ModType::Alt:
                    return LALT;
                case ModType::GUI:
                    return LGUI;
            }
            break;
        case MorphType::NorthWest:
        case MorphType::West:
        case MorphType::SouthWest:
            switch (m_type) {
                case ModType::Control:
                    return RCTRL;
                case ModType::Alt:
                    return RALT;
                case ModType::GUI:
                    return RGUI;
            }
            break;
    }
    assert(false && "Should not happen");
    return NoModifier;
}

int Mod::rowOf(const SchemaItem *me) const
{
    assert(false && "Should not happen");
    return -1;
}
