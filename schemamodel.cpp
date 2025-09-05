#include "schemamodel.hpp"
#include "schema.hpp"
#include <QFont>

SchemaModel::SchemaModel(SchemaItem *schema, QObject *parent)
    : QAbstractItemModel{parent},
      m_schema{schema}
{

}

void SchemaModel::beforeSchemaChange()
{
    beginResetModel();
}

void SchemaModel::afterSchemaChange()
{
    endResetModel();
}

QModelIndex SchemaModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return {};

    auto *parentItem = getItem(parent);
    if (!parentItem)
        return {};

    if (auto *childItem = parentItem->child(row))
        return createIndex(row, column, childItem);

    return {};
}

QModelIndex SchemaModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return {};

    auto *childItem = getItem(index);
    auto *parentItem = childItem ? childItem->parent() : nullptr;

    return parentItem != m_schema && parentItem != nullptr
            ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

int SchemaModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    auto const *parentItem = getItem(parent);
    auto const *schemaItem = getItem({});

    return parentItem ? parentItem->childCount(schemaItem->itemType()) : 0;
}

int SchemaModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}

bool SchemaModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto *item = getItem(index);
    bool result{false};

    switch (role) {
        case Qt::EditRole:
            switch (index.column()) {
                case ModeColumn:
                    result = item->setMode(value.toInt());
                    break;
                case ValueColumn:
                    result = item->setValue(value.toString());
                    break;
            }
            break;
        case Note:
            item->setAntecedentNote(value.toString());
            break;
    }

    if (result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    return result;
}

QVariant SchemaModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    auto const *item = static_cast<SchemaItem const *>(index.internalPointer());

    switch (role) {
        case Qt::DisplayRole:
            switch (index.column()) {
                case NameColumn: return item->name();
                case ModeColumn: return item->modeName();
                case ValueColumn:
                    return item->kind() == SchemaItem::Kind::Morph && item->mode() == static_cast<int>(Mode::SchemaName)
                            ? static_cast<Schema*>(m_schema)->fullName()
                            : item->value().replace(" ", "·");
            }
            break;
        case Qt::EditRole:
            switch (index.column()) {
                case ModeColumn: return item->mode();
                case ValueColumn: return item->value();
            }
            break;
        case Qt::FontRole: {
            if (item->kind() == SchemaItem::Kind::Antecedent) {
                QFont f;
                f.setPointSize(16);
                return f;
            } else if (item->kind() == SchemaItem::Kind::Layer) {
                QFont f;
                f.setPointSize(14);
                return f;
            } else {
                QFont f;
                f.setPointSize(10);
                return f;
            }
            break;
        }
        case AntecedentType:
            return item->antecedentType();
        case Note:
            return item->antecedentNote();
        case Modes:
            return item->modes();
        case IsLeftHand:
            return item->isLeft();
        case IsRightHand:
            return item->isRight();
        case MorphType:
            return item->morphType();
    }

    return {};
}

QVariant SchemaModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return {};

    switch (role) {
        case Qt::DisplayRole:
            switch (section) {
                case NameColumn: return "Name";
                case ModeColumn: return "Mode";
                case ValueColumn: return "Value";
            }
            break;
    }

    return {};
}

Qt::ItemFlags SchemaModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    auto flags = QAbstractItemModel::flags(index);

    auto const *item = getItem(index);
    if ((index.column() == ValueColumn || index.column() == ModeColumn) && item->isEditable())
        flags |= Qt::ItemIsEditable;

    return flags;
}

SchemaItem *SchemaModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (auto *item = static_cast<SchemaItem*>(index.internalPointer()))
            return item;
    }

    return m_schema;
}

SchemaProxyModel::SchemaProxyModel(QObject *parent)
    : QSortFilterProxyModel{parent},
      m_hand{BothHands},
      m_morphType{-1}
{
    setRecursiveFilteringEnabled(true);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

void SchemaProxyModel::setHandFilter(int hand)
{
    if (m_hand == static_cast<HandFilter>(hand))
        return;

    m_hand = static_cast<HandFilter>(hand);
    invalidateFilter();
}

void SchemaProxyModel::setMorphType(int morphType)
{
    if (m_morphType == morphType)
        return;

    m_morphType = morphType;
    invalidateFilter();
}

bool SchemaProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    auto const sourceIndex = sourceModel()->index(sourceRow, SchemaModel::ValueColumn, sourceParent);

    if (m_hand == BothHands && m_morphType == -1 && filterRegularExpression().pattern().isEmpty())
        return true;

    bool handsFilter = (m_hand == BothHands)
                    || (m_hand == LeftHand && sourceIndex.data(SchemaModel::IsLeftHand).toBool())
                    || (m_hand == RightHand && sourceIndex.data(SchemaModel::IsRightHand).toBool());

    bool morphTypeFilter = (m_morphType == -1)
                        || (m_morphType == sourceIndex.data(SchemaModel::MorphType).toInt());

    return handsFilter && morphTypeFilter && sourceIndex.data(Qt::EditRole).toString().contains(filterRegularExpression());
}
