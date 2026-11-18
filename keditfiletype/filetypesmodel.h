/*
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStringList>

#include <memory>
#include <vector>

#include "mimetypedata.h"

class FileTypesModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        IsGroupRole,
        MajorTypeRole,
        MinorTypeRole,
        IconRole,
        CommentRole,
        PatternsRole,
    };

    struct SaveResult {
        bool needUpdateMimeDb = false;
        bool needUpdateSycoca = false;
    };

    explicit FileTypesModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void load();
    SaveResult save();
    void refresh();

    MimeTypeData *find(const QString &name) const;
    QModelIndex indexOf(const QString &name) const;
    bool matchesFilter(const QModelIndex &index, const QString &filter) const;
    QStringList groups() const;
    QStringList namesInGroup(const QString &group) const;

    QString addType(const QString &group, const QString &type);
    void removeType(const QString &name);
    bool canRemove(const QString &name) const;
    bool removeIsRevert(const QString &name) const;

    void setComment(const QString &name, const QString &comment);
    void setIcon(const QString &name, const QString &icon);
    void setPatterns(const QString &name, const QStringList &patterns);
    void setAppServices(const QString &name, const QStringList &services);

Q_SIGNALS:
    void changed();
    void removedListChanged();

private:
    struct Group {
        QString name;
        std::vector<std::unique_ptr<MimeTypeData>> types;
    };
    struct Location {
        int group = -1;
        int row = -1;
        bool isValid() const
        {
            return row >= 0;
        }
    };

    Location locate(const QString &name) const;
    int groupRow(const QString &name) const;
    void emitChanged(const Location &location);
    template<typename Func>
    void forEachType(Func &&func) const;

    std::vector<std::unique_ptr<Group>> m_groups;
    QStringList m_removedList;
};

class FileTypesFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)

public:
    explicit FileTypesFilterModel(QObject *parent = nullptr);

    QString filterText() const;
    void setFilterText(const QString &text);
    Q_INVOKABLE QModelIndex indexForName(const QString &name) const;

Q_SIGNALS:
    void filterTextChanged();

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_filterText;
};
