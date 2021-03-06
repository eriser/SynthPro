#include "modulelistmodel.h"

#include <QAbstractItemModel>
#include <QDebug>
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

    return m_moduleNames.size();
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

    if (role == Qt::ToolTipRole) {
        return QString(tr("%1 Module")).arg(m_moduleNames.value(index.row()));
    }

    if (role == Qt::DisplayRole) {
        return m_moduleNames.value(index.row());
    }

    if (role == Qt::DecorationRole) {
        return m_modulePixmaps.value(index.row());
    }

    if (role == Qt::UserRole) {
        return m_moduleTypes.value(index.row());
    }

    return QVariant();
}

QMimeData* ModuleListModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;

    if (indexes.size() != 1) {
        // If we have more than one item selected then…
        // Erm don't know what to do.
        return mimeData;
    }

    foreach (const QModelIndex& index, indexes) {
        if (index.isValid()) {
            encodedData = data(index, Qt::UserRole).toByteArray();
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

void ModuleListModel::addModule(const QString& moduleName, QtFactory::ModuleType moduleType)
{
    m_moduleNames.append(moduleName);
    m_moduleTypes.append(moduleType);

    // Find the pixmap file to append it in the pixmaps list.
    QString filename = moduleName;
    filename = filename.replace(" ", "-").toLower();
    filename += "-icon.png";

    m_modulePixmaps.append(QPixmap(":/src/resources/images/module/" + filename));
}
