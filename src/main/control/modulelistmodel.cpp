#include "modulelistmodel.h"

#include <QAbstractItemModel>
#include <QMimeData>

ModuleListModel::ModuleListModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

int ModuleListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return m_modules.size();
}

Qt::ItemFlags ModuleListModel::flags(const QModelIndex& index) const
{
    if (index.isValid()) {
        return (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
    }

    return Qt::NoItemFlags;
}

QVariant ModuleListModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return m_modules.value(index.row());
    }

    if (role == Qt::UserRole) {
        return m_modules.value(index.row());
    }

    return QVariant();
}

QMimeData* ModuleListModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            stream << qVariantValue<QString>(data(index, Qt::UserRole));
        }
    }

    mimeData->setData("application/x-synthpro", encodedData);
    return mimeData;
}

QStringList ModuleListModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-synthpro";
    return types;
}

void ModuleListModel::addModule(const QString& moduleName)
{
    m_modules.append(moduleName);
}
