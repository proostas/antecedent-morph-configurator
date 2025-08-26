#include "schema.hpp"
#include <QJsonObject>
#include <QJsonArray>

SchemaItem::SchemaItem(SchemaItem *parent)
    : m_parent{parent}
{

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
    m_layers.push_back(std::make_unique<Layer>(Layer::Base, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Mouse, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Navigation, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Media, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Function, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Number, this));
    m_layers.push_back(std::make_unique<Layer>(Layer::Symbol, this));
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
    switch (m_type) {
        case A: return "A";
        case B: return "B";
        case C: return "C";
        case D: return "D";
        case E: return "E";
        case F: return "F";
        case G: return "G";
        case H: return "H";
        case I: return "I";
        case J: return "G";
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

Layer::Layer(Type type, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_morphs{}
{
    if (type == Base || type == Mouse || type == Navigation || type == Media) {
        m_morphs.push_back(std::make_unique<Morph>(Morph::NorthEast, Morph::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(Morph::East, Morph::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(Morph::SouthEast, Morph::Text, this));
    }
    if (type == Base || type == Symbol || type == Number || type == Function) {
        m_morphs.push_back(std::make_unique<Morph>(Morph::SouthWest, Morph::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(Morph::West, Morph::Text, this));
        m_morphs.push_back(std::make_unique<Morph>(Morph::NorthWest, Morph::Text, this));
    }
}

bool Layer::fromJson(const QJsonObject &json)
{
    m_type = static_cast<Type>(json["type"].toInt());
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
    layer["type"] = m_type;
    layer["morphs"] = morphs;
    return layer;
}

void Layer::clear()
{
    std::for_each(m_morphs.cbegin(), m_morphs.cend(),
                  [](std::unique_ptr<Morph> const &m) { m->clear(); });
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
    return m_type;
}

QString Layer::name() const
{
    switch (m_type) {
        case Base: return "Base";
        case Mouse: return "Mouse";
        case Navigation: return "Navigation";
        case Media: return "Media";
        case Function: return "Function";
        case Number: return "Number";
        case Symbol: return "Symbol";
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

Morph::Morph(Type type, Mode mode, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_mode{mode},
      m_mods{},
      m_value{},
      m_changed{false}
{
    m_mods.push_back(std::make_unique<Mod>(Mod::Control, Mod::Replace, this));
    m_mods.push_back(std::make_unique<Mod>(Mod::Alt, Mod::Replace, this));
    m_mods.push_back(std::make_unique<Mod>(Mod::GUI, Mod::Replace, this));
}

bool Morph::fromJson(const QJsonObject &json)
{
    m_type = static_cast<Type>(json["type"].toInt());
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
    morph["type"] = m_type;
    morph["mode"] = m_mode;
    morph["value"] = m_value;
    morph["mods"] = mods;
    return morph;
}

void Morph::clear()
{
    m_mode = Text;
    m_value.clear();
    m_changed = false;
    std::for_each(m_mods.cbegin(), m_mods.cend(),
                  [](std::unique_ptr<Mod> const &m) { m->clear(); });
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
    return m_type;
}

QString Morph::name() const
{
    switch (m_type) {
        case NorthEast: return "NE";
        case East: return "E";
        case SouthEast: return "SE";
        case SouthWest: return "SW";
        case West: return "W";
        case NorthWest: return "NW";
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
    if (m_mode == mode)
        return false;

    m_mode = static_cast<Mode>(mode);
    m_changed = true;
    return true;
}

int Morph::mode() const
{
    return m_mode;
}

QString Morph::modeName() const
{
    switch (m_mode) {
        case Text: return "Text";
        case MacroName: return "Macro";
        case SchemaName: return "SchName";
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

Mod::Mod(Type type, Mode mode, SchemaItem *parent)
    : SchemaItem{parent},
      m_type{type},
      m_mode{mode},
      m_value{},
      m_changed{false}
{

}

bool Mod::fromJson(const QJsonObject &json)
{
    m_type = static_cast<Type>(json["type"].toInt());
    m_mode = static_cast<Mode>(json["mode"].toInt());
    m_value = json["value"].toString();
    m_changed = false;
    return true;
}

QJsonObject Mod::toJson() const
{
    QJsonObject mod;
    mod["type"] = m_type;
    mod["mode"] = m_mode;
    mod["value"] = m_value;
    return mod;
}

void Mod::clear()
{
    m_mode = Replace;
    m_value.clear();
    m_changed = false;
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
    return m_type;
}

QString Mod::name() const
{
    switch (m_type) {
        case Control: return "Ctrl";
        case Alt: return "Alt";
        case GUI: return "GUI";
    }
    assert(false && "Should not happen");
    return {};
}

QStringList Mod::modes() const
{
    return {"Replace", "Append"};
}

bool Mod::setMode(int mode)
{
    if (m_mode == mode)
        return false;

    m_mode = static_cast<Mode>(mode);
    m_changed = true;
    return true;
}

int Mod::mode() const
{
    return m_mode;
}

QString Mod::modeName() const
{
    switch (m_mode) {
        case Replace: return "Replace";
        case Append: return "Append";
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

int Mod::rowOf(const SchemaItem *me) const
{
    assert(false && "Should not happen");
    return -1;
}
