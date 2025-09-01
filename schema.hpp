#ifndef SCHEMA_HPP
#define SCHEMA_HPP

#include <QString>
#include <QJsonDocument>

enum class LayerType {
    Base,
    Mouse,
    Navigation,
    Media,
    Function,
    Number,
    Symbol
};

enum class MorphType {
    NorthEast,
    East,
    SouthEast,
    NorthWest,
    West,
    SouthWest
};

enum class ModType {
    Control,
    Alt,
    GUI
};

enum class Mode {
    Text,
    MacroName,
    SchemaName
};

enum Modifier {
    NoModifier,
    LCTRL,
    RCTRL,
    LALT,
    RALT,
    LGUI,
    RGUI
 };

class Antecedent;

class SchemaItem
{
public:
    enum class Kind {Schema, Antecedent, Layer, Morph, Mod};
public:
    explicit SchemaItem(SchemaItem *parent = nullptr);
    virtual ~SchemaItem() = default;

    virtual Kind kind() const = 0;
    virtual SchemaItem *child(int row) = 0;
    virtual int childCount(int schemaType) const = 0;
    int row() const;
    SchemaItem *parent();

    virtual int itemType() const = 0;
    virtual QString name() const = 0;
    virtual QStringList modes() const;
    virtual bool setMode(int mode);
    virtual int mode() const;
    virtual QString modeName() const;
    virtual QString value() const;
    virtual bool isEditable() const;
    virtual bool setValue(QString const& value);

    bool isLeft() const;
    virtual bool isRight() const;

    virtual bool isChanged() const = 0;
    virtual void clearChanged() = 0;

    virtual int antecedentType() const;
    virtual bool setAntecedentNote(QString const &note);
    virtual QString antecedentNote() const;

    virtual Modifier pressedModifier() const;

protected:
    virtual int rowOf(SchemaItem const *me) const = 0;

protected:
    SchemaItem *m_parent;
};

class Schema : public SchemaItem
{
public:
    friend class ZmkCodeGenerator;
public:
    enum Type {Flat, Deep};
public:
    Schema(Type type, SchemaItem *parent = nullptr);
    ~Schema() override = default;

    bool isNew() const;
    bool setFilePath(QString filePath);
    QString filePath() const;
    QString suggestedFileName() const;

    bool fromJson(QJsonDocument const &jsonDoc);
    QJsonDocument toJson();
    void clear();

    bool setName(QString const &name);
    QString fullName() const;
    bool setVersion(QString const &version);
    QString version() const;
    bool setType(Type type);
    Type type() const;
    bool setPrefix(QString const &prefix);
    QString prefix() const;

    bool isEmpty(LayerType layerType, MorphType morphType) const;
    bool isEmpty(LayerType layerType, MorphType morphType, ModType modType) const;

public: // SchemaItem interface
    SchemaItem::Kind kind() const override;
    SchemaItem *child(int row) override;
    int childCount(int schemaType) const override;
    int itemType() const override;
    QString name() const override;

    bool isRight() const override;

    bool isChanged() const override;
    void clearChanged() override;

    int antecedentType() const override;

protected:
    int rowOf(SchemaItem const *me) const override;

private:
    QString m_filePath;
    QString m_name;
    QString m_version;
    Type m_type;
    QString m_prefix;
    std::vector<std::unique_ptr<Antecedent>> m_antecedents;
    bool m_changed;
};

class Layer;
class Morph;
class Mod;

class Antecedent : public SchemaItem
{
public:
    friend class ZmkCodeGenerator;
public:
    enum Type {
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
        Comma, Dot, Quote, Slash,
        LBrace, RBrace, LParen, RParen, Star,
        Colon, Dollar, Percent, Caret, Plus,
        Tilda, Exclamation, At, Hash, Pipe,
        Ampersand, Underscore,
        LBracket, RBracket, Semicolon, Grave, Equal, Backslash, Minus, Space
    };
public:
    explicit Antecedent(Type type, SchemaItem *parent);
    ~Antecedent() override = default;

    bool fromJson(QJsonObject const &json);
    QJsonObject toJson(int schemaType) const;
    void clear();

    Type type() const;

    bool isEmpty(LayerType layerType, MorphType morphType) const;
    bool isEmpty(LayerType layerType, MorphType morphType, ModType modType) const;

    Morph *getMorph(LayerType layerType, MorphType morphType) const;
    Mod *getMod(LayerType layerType, MorphType morphType, ModType modType) const;

public: // SchemaItem interface
    SchemaItem::Kind kind() const override;
    SchemaItem *child(int row) override;
    int childCount(int schemaType) const override;
    int itemType() const override;
    QString name() const override;

    bool isRight() const override;

    bool isChanged() const override;
    void clearChanged() override;

    int antecedentType() const override;
    bool setAntecedentNote(QString const &note) override;
    QString antecedentNote() const override;

    QString symbol() const;
    QString zmkCode() const;

public:
    static QString symbol(Type type);
    static QString ZMKCode(Type type);
    static QString QMKCode(Type type);

protected:
    int rowOf(SchemaItem const *me) const override;
private:
    Type m_type;
    std::vector<std::unique_ptr<Layer>> m_layers;
    QString m_note;
    bool m_changed;
};

class Layer : public SchemaItem
{
public:
    friend class ZmkCodeGenerator;
public:

public:
    explicit Layer(LayerType type, SchemaItem *parent);
    ~Layer() override = default;

    bool fromJson(QJsonObject const &json);
    QJsonObject toJson() const;
    void clear();

    bool isEmpty(MorphType morphType) const;
    bool isEmpty(MorphType morphType, ModType modType) const;

    Morph *getMorph(MorphType morphType) const;
    Mod *getMod(MorphType morphType, ModType modType) const;

public: // SchemaItem interface
    SchemaItem::Kind kind() const override;
    SchemaItem *child(int row) override;
    int childCount(int schemaType) const override;
    int itemType() const override;
    QString name() const override;

    bool isChanged() const override;
    void clearChanged() override;

protected:
    int rowOf(SchemaItem const *me) const override;

private:
    LayerType m_type;
    std::vector<std::unique_ptr<Morph>> m_morphs;
};

class Morph : public SchemaItem
{
public:
    friend class ZmkCodeGenerator;

public:
    explicit Morph(MorphType type, Mode mode, SchemaItem *parent);
    ~Morph() override = default;

    bool fromJson(QJsonObject const &json);
    QJsonObject toJson() const;
    void clear();

    bool isEmpty() const;
    bool isEmpty(ModType modType) const;
    bool isValid() const;

    Mod *getMod(ModType modType) const;

    bool isSingleLettered(const QString &symbol) const;

public: // SchemaItem interface
    SchemaItem::Kind kind() const override;
    SchemaItem *child(int row) override;
    int childCount(int schemaType) const override;
    int itemType() const override;
    QString name() const override;
    QStringList modes() const override;
    bool setMode(int mode) override;
    int mode() const override;
    QString modeName() const override;
    QString value() const override;
    bool isEditable() const override;
    bool setValue(QString const& value) override;

    bool isChanged() const override;
    void clearChanged() override;

protected:
    int rowOf(SchemaItem const *me) const override;

private:
    MorphType m_type;
    Mode m_mode;
    std::vector<std::unique_ptr<Mod>> m_mods;
    QString m_value;
    bool m_changed;
};

class Mod : public SchemaItem
{
public:
    friend class ZmkCodeGenerator;

public:
    explicit Mod(ModType type, Mode mode, SchemaItem *parent);
    ~Mod() override = default;

    bool fromJson(QJsonObject const &json);
    QJsonObject toJson() const;
    void clear();

    bool isEmpty() const;
    bool isValid() const;

    bool isSingleLettered(QString const &symbol) const;

public: // SchemaItem interface
    SchemaItem::Kind kind() const override;
    SchemaItem *child(int row) override;
    int childCount(int schemaType) const override;
    int itemType() const override;
    QString name() const override;
    QStringList modes() const override;
    bool setMode(int mode) override;
    int mode() const override;
    QString modeName() const override;
    QString value() const override;
    bool isEditable() const override;
    bool setValue(QString const& value) override;

    bool isChanged() const override;
    void clearChanged() override;

     Modifier pressedModifier() const override;

protected:
    int rowOf(SchemaItem const *me) const override;

private:
    ModType m_type;
    Mode m_mode;
    QString m_value;
    bool m_changed;
};

#endif // SCHEMA_HPP
