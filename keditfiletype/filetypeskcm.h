/*
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once

#include <KQuickConfigModule>

#include <QStringList>
#include <QVariantList>

class FileTypesModel;
class FileTypesFilterModel;

class FileTypesKCM : public KQuickConfigModule
{
    Q_OBJECT

    Q_PROPERTY(FileTypesModel *model READ model CONSTANT)
    Q_PROPERTY(FileTypesFilterModel *filteredModel READ filteredModel CONSTANT)
    Q_PROPERTY(QStringList groups READ groups NOTIFY groupsChanged)

    Q_PROPERTY(QString currentType READ currentType WRITE setCurrentType NOTIFY currentTypeChanged)
    Q_PROPERTY(QString currentComment READ currentComment NOTIFY currentDataChanged)
    Q_PROPERTY(QString currentIcon READ currentIcon NOTIFY currentDataChanged)
    Q_PROPERTY(QStringList currentPatterns READ currentPatterns NOTIFY currentDataChanged)

    /// List of {storageId, name, icon} maps, in order of preference
    Q_PROPERTY(QVariantList currentApplications READ currentApplications NOTIFY currentDataChanged)
    Q_PROPERTY(bool currentCanRemove READ currentCanRemove NOTIFY currentDataChanged)
    Q_PROPERTY(bool currentRemoveIsRevert READ currentRemoveIsRevert NOTIFY currentDataChanged)

public:
    FileTypesKCM(QObject *parent, const KPluginMetaData &data);

    void load() override;
    void save() override;

    [[nodiscard]] FileTypesModel *model() const;
    [[nodiscard]] FileTypesFilterModel *filteredModel() const;
    [[nodiscard]] QStringList groups() const;

    [[nodiscard]] QString currentType() const;
    void setCurrentType(const QString &name);
    [[nodiscard]] QString currentComment() const;
    [[nodiscard]] QString currentIcon() const;
    [[nodiscard]] QStringList currentPatterns() const;
    [[nodiscard]] QVariantList currentApplications() const;
    [[nodiscard]] bool currentCanRemove() const;
    [[nodiscard]] bool currentRemoveIsRevert() const;

    Q_INVOKABLE void setCurrentComment(const QString &comment);
    Q_INVOKABLE void setCurrentIcon(const QString &icon);
    Q_INVOKABLE void addPattern(const QString &pattern);
    Q_INVOKABLE void removePattern(int index);

    Q_INVOKABLE void moveApplication(int from, int to);
    Q_INVOKABLE void addApplication();
    Q_INVOKABLE void editApplication(int index);
    Q_INVOKABLE void removeApplication(int index);
    /// Uses the application preference order of the current type for all the given types
    Q_INVOKABLE void applyApplicationsTo(const QStringList &types);

    /// Returns the name of the new type
    Q_INVOKABLE [[nodiscard]] QString addType(const QString &group, const QString &name);
    Q_INVOKABLE void removeCurrentType();

    Q_INVOKABLE [[nodiscard]] QStringList typesInGroup(const QString &group) const;

Q_SIGNALS:
    void groupsChanged();
    void currentTypeChanged();
    void currentDataChanged();

private:
    QStringList currentApplicationIds() const;
    void setCurrentApplicationIds(const QStringList &ids);

    FileTypesModel *const m_model;
    FileTypesFilterModel *const m_filterModel;
    QString m_currentType;
};
