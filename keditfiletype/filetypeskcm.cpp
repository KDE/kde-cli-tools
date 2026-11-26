/*
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>
    SPDX-FileCopyrightText: 2022 Marco Rebhan <me@dblsaiko.net>
    SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filetypeskcm.h"

#include "filetypesmodel.h"
#include "mimetypedata.h"
#include "mimetypewriter.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDialog>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QStandardPaths>
#include <QUrl>
#include <QWindow>

#include <KBuildSycocaProgressDialog>
#include <KFileItem>
#include <KLocalizedString>
#include <KOpenWithDialog>
#include <KPluginFactory>
#include <KPropertiesDialog>
#include <KService>
#include <KSycoca>

using namespace Qt::StringLiterals;

K_PLUGIN_CLASS_WITH_JSON(FileTypesKCM, "kcm_filetypes.json")

namespace
{
void setTransientParent(QDialog *dialog, QQuickItem *item)
{
    if (!item || !item->window()) {
        return;
    }
    QWindow *parentWindow = QQuickRenderControl::renderWindowFor(item->window());
    if (!parentWindow) {
        parentWindow = item->window();
    }
    dialog->setWindowModality(Qt::WindowModal);
    dialog->winId();
    if (QWindow *handle = dialog->windowHandle()) {
        handle->setTransientParent(parentWindow);
    }
}
}

FileTypesKCM::FileTypesKCM(QObject *parent, const KPluginMetaData &data)
    : KQuickConfigModule(parent, data)
    , m_model(new FileTypesModel(this))
    , m_filterModel(new FileTypesFilterModel(this))
{
    setButtons(Help | Apply);

    m_filterModel->setSourceModel(m_model);

    connect(m_model, &FileTypesModel::changed, this, [this] {
        setNeedsSave(true);
    });
    connect(m_model, &FileTypesModel::dataChanged, this, &FileTypesKCM::currentDataChanged);
    connect(m_model, &FileTypesModel::rowsInserted, this, [this](const auto &parent) {
        if (!parent.isValid()) {
            Q_EMIT groupsChanged();
        }
    });
    connect(m_model, &FileTypesModel::rowsRemoved, this, [this](const auto &parent) {
        if (!parent.isValid()) {
            Q_EMIT groupsChanged();
        }
    });
    connect(m_model, &FileTypesModel::modelReset, this, [this] {
        Q_EMIT groupsChanged();
        if (!m_model->find(m_currentType)) {
            setCurrentType({});
        }
    });
    connect(m_model, &FileTypesModel::removedListChanged, this, &FileTypesKCM::currentDataChanged);

    connect(KSycoca::self(), &KSycoca::databaseChanged, m_model, &FileTypesModel::refresh);
}

FileTypesModel *FileTypesKCM::model() const
{
    return m_model;
}
FileTypesFilterModel *FileTypesKCM::filteredModel() const
{
    return m_filterModel;
}

QStringList FileTypesKCM::groups() const
{
    return m_model->groups();
}

QStringList FileTypesKCM::typesInGroup(const QString &group) const
{
    return m_model->namesInGroup(group);
}

void FileTypesKCM::load()
{
    setCurrentType({});
    m_model->load();
    setNeedsSave(false);
}

void FileTypesKCM::save()
{
    const auto result = m_model->save();

    if (result.needUpdateMimeDb) {
        MimeTypeWriter::runUpdateMimeDatabase();
    }
    if (result.needUpdateSycoca) {
        KBuildSycocaProgressDialog::rebuildKSycoca(nullptr);
    }

    setNeedsSave(false);
    Q_EMIT currentDataChanged();
}

QString FileTypesKCM::currentType() const
{
    return m_currentType;
}

void FileTypesKCM::setCurrentType(const QString &name)
{
    if (m_currentType == name) {
        return;
    }
    m_currentType = name;
    Q_EMIT currentTypeChanged();
    Q_EMIT currentDataChanged();
}

QString FileTypesKCM::currentComment() const
{
    const auto data = m_model->find(m_currentType);
    return data ? data->comment() : QString();
}

QString FileTypesKCM::currentIcon() const
{
    const auto data = m_model->find(m_currentType);
    return data ? data->icon() : QString();
}

QStringList FileTypesKCM::currentPatterns() const
{
    const auto data = m_model->find(m_currentType);
    return data ? data->patterns() : QStringList();
}

QVariantList FileTypesKCM::currentApplications() const
{
    QVariantList result;
    const auto data = m_model->find(m_currentType);
    if (!data) {
        return result;
    }
    const auto services = data->appServices();
    for (const auto &id : services) {
        if (const auto service = KService::serviceByStorageId(id)) {
            result.append(QVariantMap{
                {u"storageId"_s, service->storageId()},
                {u"name"_s, service->name()},
                {u"icon"_s, service->icon()},
            });
        }
    }
    return result;
}

bool FileTypesKCM::currentCanRemove() const
{
    return m_model->canRemove(m_currentType);
}

bool FileTypesKCM::currentRemoveIsRevert() const
{
    return m_model->removeIsRevert(m_currentType);
}

void FileTypesKCM::setCurrentComment(const QString &comment)
{
    m_model->setComment(m_currentType, comment);
}

void FileTypesKCM::setCurrentIcon(const QString &icon)
{
    m_model->setIcon(m_currentType, icon);
}

void FileTypesKCM::addPattern(const QString &pattern)
{
    const auto data = m_model->find(m_currentType);
    if (!data || pattern.isEmpty() || data->patterns().contains(pattern)) {
        return;
    }
    m_model->setPatterns(m_currentType, data->patterns() + QStringList{pattern});
}

void FileTypesKCM::removePattern(int index)
{
    const auto data = m_model->find(m_currentType);
    if (!data || index < 0 || index >= data->patterns().size()) {
        return;
    }
    auto patterns = data->patterns();
    patterns.removeAt(index);
    m_model->setPatterns(m_currentType, patterns);
}

QStringList FileTypesKCM::currentApplicationIds() const
{
    QStringList ids;
    const auto apps = currentApplications();
    for (const auto &app : apps) {
        ids.append(app.toMap().value(u"storageId"_s).toString());
    }
    return ids;
}

void FileTypesKCM::setCurrentApplicationIds(const QStringList &ids)
{
    QStringList unique = ids;
    unique.removeDuplicates();
    m_model->setAppServices(m_currentType, unique);
}

void FileTypesKCM::moveApplication(int from, int to)
{
    QStringList ids = currentApplicationIds();
    if (from < 0 || from >= ids.size() || to < 0 || to >= ids.size() || from == to) {
        return;
    }
    ids.move(from, to);
    setCurrentApplicationIds(ids);
}

void FileTypesKCM::addApplication()
{
    if (!m_model->find(m_currentType)) {
        return;
    }

    KOpenWithDialog dlg(m_currentType, QString(), nullptr);
    dlg.setSaveNewApplications(true);
    setTransientParent(&dlg, mainUi());
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    const auto service = dlg.service();
    if (!service) {
        return;
    }
    const MimeTypeData *data = m_model->find(m_currentType);
    if (!data || data->appServices().contains(service->storageId())) {
        return;
    }
    setCurrentApplicationIds(QStringList{service->storageId()} + currentApplicationIds());
}

void FileTypesKCM::editApplication(int index)
{
    auto ids = currentApplicationIds();
    if (index < 0 || index >= ids.size()) {
        return;
    }

    auto service = KService::serviceByStorageId(ids.at(index));
    if (!service) {
        return;
    }
    const auto desktopPath = service->entryPath();

    auto path = desktopPath;

    if (const auto fullPath = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, path); !fullPath.isEmpty()) {
        path = fullPath;
    }

    KFileItem item(QUrl::fromLocalFile(path), QStringLiteral("application/x-desktop"), KFileItem::Unknown);
    KPropertiesDialog dlg(item, nullptr);
    setTransientParent(&dlg, mainUi());
    if (dlg.exec() != QDialog::Accepted) {
        return;
    }

    service = KService::serviceByDesktopPath(desktopPath);
    if (!service) {
        return;
    }
    ids = currentApplicationIds();
    if (index >= ids.size()) {
        return;
    }

    ids[index] = service->storageId();
    setCurrentApplicationIds(ids);
}

void FileTypesKCM::removeApplication(int index)
{
    auto ids = currentApplicationIds();
    if (index < 0 || index >= ids.size()) {
        return;
    }
    ids.removeAt(index);
    setCurrentApplicationIds(ids);
}

void FileTypesKCM::applyApplicationsTo(const QStringList &types)
{
    const auto current = m_model->find(m_currentType);
    if (!current) {
        return;
    }
    const auto services = current->appServices();
    for (const QString &type : types) {
        if (type != m_currentType) {
            m_model->setAppServices(type, services);
        }
    }
}

QString FileTypesKCM::addType(const QString &group, const QString &name)
{
    const auto newType = m_model->addType(group, name);
    if (!newType.isEmpty()) {
        m_filterModel->setFilterText({});
        setCurrentType(newType);
    }
    return newType;
}

void FileTypesKCM::removeCurrentType()
{
    if (!m_model->canRemove(m_currentType)) {
        return;
    }
    const QString name = m_currentType;
    if (!m_model->removeIsRevert(name)) {
        setCurrentType({});
    }
    m_model->removeType(name);
}

#include "filetypeskcm.moc"

#include "moc_filetypeskcm.cpp"
