/*
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filetypesmodel.h"

#include "mimetypewriter.h"

#include <QMimeDatabase>
#include <QStandardPaths>

#include <algorithm>

FileTypesModel::FileTypesModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

int FileTypesModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return static_cast<int>(m_groups.size());
    }
    if (parent.internalPointer() || parent.column() != 0) { // a mimetype, or a column other than 0
        return 0;
    }
    return static_cast<int>(m_groups[parent.row()]->types.size());
}

int FileTypesModel::columnCount(const QModelIndex &) const
{
    return 1;
}

QModelIndex FileTypesModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column != 0 || row < 0 || !hasIndex(row, column, parent)) {
        return {};
    }
    if (!parent.isValid()) {
        return createIndex(row, 0, nullptr);
    }
    return createIndex(row, 0, m_groups[parent.row()].get());
}

QModelIndex FileTypesModel::parent(const QModelIndex &child) const
{
    const auto *group = static_cast<const Group *>(child.internalPointer());
    if (!child.isValid() || !group) {
        return {};
    }
    return createIndex(groupRow(group->name), 0, nullptr);
}

QVariant FileTypesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid)) {
        return {};
    }
    const auto group = static_cast<const Group *>(index.internalPointer());
    if (!group) {
        const auto &name = m_groups[index.row()]->name;
        switch (role) {
        case Qt::DisplayRole:
        case NameRole:
        case MajorTypeRole:
            return name;
        case IsGroupRole:
            return true;
        }
        return QString();
    }

    const MimeTypeData &type = *group->types[index.row()];
    switch (role) {
    case Qt::DisplayRole:
    case MinorTypeRole:
        return type.minorType();
    case Qt::DecorationRole:
    case IconRole:
        return type.icon();
    case NameRole:
        return type.name();
    case IsGroupRole:
        return false;
    case MajorTypeRole:
        return type.majorType();
    case CommentRole:
        return type.comment();
    case PatternsRole:
        return type.patterns();
    }
    return {};
}

QHash<int, QByteArray> FileTypesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {NameRole, "name"},
        {IsGroupRole, "isGroup"},
        {MajorTypeRole, "majorType"},
        {MinorTypeRole, "minorType"},
        {IconRole, "iconName"},
        {CommentRole, "comment"},
        {PatternsRole, "patterns"},
    };
}

template<typename Func>
void FileTypesModel::forEachType(Func &&func) const
{
    for (const auto &group : m_groups) {
        for (const auto &type : group->types) {
            func(*type);
        }
    }
}

int FileTypesModel::groupRow(const QString &name) const
{
    const auto it = std::ranges::find(m_groups, name, [](const auto &group) {
        return group->name;
    });
    return it == m_groups.end() ? -1 : static_cast<int>(it - m_groups.begin());
}

FileTypesModel::Location FileTypesModel::locate(const QString &name) const
{
    const int groupIndex = groupRow(name.section(QLatin1Char('/'), 0, 0));
    if (groupIndex < 0) {
        return {};
    }
    const auto &types = m_groups[groupIndex]->types;
    const auto it = std::ranges::find_if(types, [&name](const auto &type) {
        return type->name() == name;
    });
    if (it == types.end()) {
        return {};
    }
    return {groupIndex, static_cast<int>(it - types.begin())};
}

MimeTypeData *FileTypesModel::find(const QString &name) const
{
    const Location location = locate(name);
    return location.isValid() ? m_groups[location.group]->types[location.row].get() : nullptr;
}

QModelIndex FileTypesModel::indexOf(const QString &name) const
{
    const Location location = locate(name);
    return location.isValid() ? index(location.row, 0, index(location.group, 0)) : QModelIndex();
}

void FileTypesModel::emitChanged(const Location &location)
{
    const QModelIndex idx = index(location.row, 0, index(location.group, 0));
    Q_EMIT dataChanged(idx, idx);
}

void FileTypesModel::load()
{
    beginResetModel();
    m_groups.clear();
    m_removedList.clear();

    auto mimetypes = QMimeDatabase().allMimeTypes();
    std::ranges::sort(mimetypes, {}, &QMimeType::name);
    for (const auto &mimetype : std::as_const(mimetypes)) {
        auto type = std::make_unique<MimeTypeData>(mimetype);
        // sorted by name, so all types of a group are adjacent
        if (m_groups.empty() || m_groups.back()->name != type->majorType()) {
            m_groups.push_back(std::make_unique<Group>(Group{type->majorType(), {}}));
        }
        m_groups.back()->types.push_back(std::move(type));
    }
    endResetModel();
    Q_EMIT removedListChanged();
}

FileTypesModel::SaveResult FileTypesModel::save()
{
    SaveResult result;

    for (const auto &mime : std::as_const(m_removedList)) {
        MimeTypeWriter::removeOwnMimeType(mime);
        if (auto type = find(mime)) {
            type->markRevertSaved();
        }
        result.needUpdateMimeDb = true;
        result.needUpdateSycoca = true;
    }
    m_removedList.clear();

    forEachType([&result](auto &type) {
        if (!type.isDirty()) {
            return;
        }
        if (type.isServiceListDirty()) {
            result.needUpdateSycoca = true;
        }
        if (type.sync()) {
            result.needUpdateMimeDb = true;
        }
    });

    Q_EMIT removedListChanged();
    return result;
}

void FileTypesModel::refresh()
{
    forEachType([](MimeTypeData &type) {
        type.refresh();
    });
    for (int i = 0; i < rowCount(); ++i) {
        const auto group = index(i, 0);
        if (rowCount(group) > 0) {
            Q_EMIT dataChanged(index(0, 0, group), index(rowCount(group) - 1, 0, group));
        }
    }
}

bool FileTypesModel::matchesFilter(const QModelIndex &index, const QString &filter) const
{
    const auto group = static_cast<const Group *>(index.internalPointer());
    if (!group) {
        return false;
    }
    return filter.isEmpty() || group->types.at(index.row())->matchesFilter(filter);
}

QStringList FileTypesModel::groups() const
{
    QStringList result;
    for (const auto &group : m_groups) {
        result.append(group->name);
    }
    return result;
}

QStringList FileTypesModel::namesInGroup(const QString &group) const
{
    QStringList result;
    const int groupIndex = groupRow(group);
    if (groupIndex >= 0) {
        for (const auto &type : m_groups[groupIndex]->types) {
            result.append(type->name());
        }
    }
    return result;
}

QString FileTypesModel::addType(const QString &group, const QString &type)
{
    const auto major = group.trimmed();
    const auto minor = type.trimmed();
    if (major.isEmpty() || minor.isEmpty()) {
        return {};
    }
    const auto name = major + QLatin1Char('/') + minor;
    if (locate(name).isValid()) {
        return name;
    }

    int groupIndex = groupRow(major);
    if (groupIndex < 0) {
        const auto pos = std::ranges::find_if(m_groups, [&major](const auto &other) {
            return other->name > major;
        });
        groupIndex = static_cast<int>(pos - m_groups.begin());
        beginInsertRows({}, groupIndex, groupIndex);
        m_groups.insert(pos, std::make_unique<Group>(Group{major, {}}));
        endInsertRows();
    }

    auto &types = m_groups[groupIndex]->types;
    const auto pos = std::ranges::find_if(types, [&name](const auto &other) {
        return other->name() > name;
    });
    const int row = static_cast<int>(pos - types.begin());
    beginInsertRows(index(groupIndex, 0), row, row);
    types.insert(pos, std::make_unique<MimeTypeData>(name, true));
    endInsertRows();

    Q_EMIT changed();
    return name;
}

void FileTypesModel::removeType(const QString &name)
{
    const auto location = locate(name);
    if (!location.isValid() || !canRemove(name)) {
        return;
    }
    auto &types = m_groups[location.group]->types;

    if (!types[location.row]->isNew()) {
        m_removedList.append(name);
    }
    if (removeIsRevert(name)) {
        types[location.row]->revertToSystemDefinition();
        emitChanged(location);
    } else {
        if (types.size() == 1) {
            beginRemoveRows({}, location.group, location.group);
            m_groups.erase(m_groups.begin() + location.group);
            endRemoveRows();
        } else {
            beginRemoveRows(index(location.group, 0), location.row, location.row);
            types.erase(types.begin() + location.row);
            endRemoveRows();
        }
    }
    Q_EMIT removedListChanged();
    Q_EMIT changed();
}

bool FileTypesModel::removeIsRevert(const QString &name) const
{
    const auto type = find(name);
    if (!type || type->isNew() || type->isEssential()) {
        return false;
    }
    if (!MimeTypeWriter::hasDefinitionFile(name)) {
        return false;
    }
    const auto mimeFiles = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QLatin1String("mime/") + name + QStringLiteral(".xml"));
    return mimeFiles.count() >= 2;
}

bool FileTypesModel::canRemove(const QString &name) const
{
    const auto type = find(name);
    if (!type || type->isEssential()) {
        return false;
    }
    if (type->isNew()) {
        return true;
    }
    if (!MimeTypeWriter::hasDefinitionFile(name)) {
        return false;
    }
    return !(removeIsRevert(name) && m_removedList.contains(name));
}

void FileTypesModel::setComment(const QString &name, const QString &comment)
{
    const auto location = locate(name);
    if (!location.isValid() || m_groups[location.group]->types[location.row]->comment() == comment) {
        return;
    }
    m_groups[location.group]->types[location.row]->setComment(comment);
    emitChanged(location);
    Q_EMIT changed();
}

void FileTypesModel::setIcon(const QString &name, const QString &icon)
{
    const auto location = locate(name);
    if (!location.isValid()) {
        return;
    }
    m_groups[location.group]->types[location.row]->setUserSpecifiedIcon(icon);
    emitChanged(location);
    Q_EMIT changed();
}

void FileTypesModel::setPatterns(const QString &name, const QStringList &patterns)
{
    const auto location = locate(name);
    if (!location.isValid()) {
        return;
    }
    m_groups[location.group]->types[location.row]->setPatterns(patterns);
    emitChanged(location);
    Q_EMIT changed();
}

void FileTypesModel::setAppServices(const QString &name, const QStringList &services)
{
    const auto location = locate(name);
    if (!location.isValid() || m_groups[location.group]->types[location.row]->appServices() == services) {
        return;
    }
    m_groups[location.group]->types[location.row]->setAppServices(services);
    emitChanged(location);
    Q_EMIT changed();
}

FileTypesFilterModel::FileTypesFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setRecursiveFilteringEnabled(true);
}

QString FileTypesFilterModel::filterText() const
{
    return m_filterText;
}

void FileTypesFilterModel::setFilterText(const QString &text)
{
    if (m_filterText == text) {
        return;
    }
    m_filterText = text;
    invalidateFilter();
    Q_EMIT filterTextChanged();
}

QModelIndex FileTypesFilterModel::indexForName(const QString &name) const
{
    return mapFromSource(static_cast<FileTypesModel *>(sourceModel())->indexOf(name));
}

bool FileTypesFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (m_filterText.isEmpty()) {
        return true;
    }
    return static_cast<FileTypesModel *>(sourceModel())->matchesFilter(sourceModel()->index(sourceRow, 0, sourceParent), m_filterText);
}

#include "moc_filetypesmodel.cpp"
