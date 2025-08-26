#ifndef SCHEMAMODEL_HPP
#define SCHEMAMODEL_HPP

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

class SchemaItem;

class SchemaModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum Column {
        NameColumn,
        ModeColumn,
        ValueColumn,
    };
    enum Data {
        AntecedentType = Qt::UserRole,
        Note,
        Modes,
        IsLeftHand,
        IsRightHand
    };

public:
    SchemaModel(SchemaItem *schema, QObject *parent);
    ~SchemaModel() override = default;

    void beforeSchemaChange();
    void afterSchemaChange();

public: // QAbstractItemModel interface
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

private:
    SchemaItem *getItem(QModelIndex const &index) const;

private:
    SchemaItem *m_schema;
};

class SchemaProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum HandFilter {BothHands, LeftHand, RightHand};
public:
    explicit SchemaProxyModel(QObject *parent = nullptr);

    void setHandFilter(int hand);

private:
    HandFilter m_hand;

protected:
    bool filterAcceptsRow(int sourceRow, QModelIndex const &sourceParent) const override;
};

#endif // SCHEMAMODEL_HPP
