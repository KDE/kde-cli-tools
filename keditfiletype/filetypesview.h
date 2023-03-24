/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2000-2008 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2008 Urs Wolfer <uwolfer @ kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILETYPESVIEW_H
#define FILETYPESVIEW_H

#include <QLabel>
#include <QList>
#include <QMap>
#include <QStackedWidget>

#include <KCModule>
#include <KSharedConfig>

#include "typeslistitem.h"

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class KLineEdit;
class FileTypeDetails;
class FileGroupDetails;

class FileTypesView : public KCModule
{
    Q_OBJECT
public:
    FileTypesView(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~FileTypesView() override;

    void load() override;
    void save() override;
    void defaults() override;

protected Q_SLOTS:
    void addType();
    void removeType();
    void updateDisplay(QTreeWidgetItem *);
    void slotDoubleClicked(QTreeWidgetItem *);
    void slotFilter(const QString &patternFilter);
    void setDirty(bool state);

    void slotDatabaseChanged();
    void slotEmbedMajor(const QString &major, bool &embed);

private:
    void readFileTypes();
    void updateRemoveButton(TypesListItem *item);

private:
    QTreeWidget *typesLV;
    QPushButton *m_removeTypeB;

    QStackedWidget *m_widgetStack;
    FileTypeDetails *m_details;
    FileGroupDetails *m_groupDetails;
    QLabel *m_emptyWidget;

    KLineEdit *patternFilterLE;
    QStringList removedList;
    bool m_dirty;
    bool m_removeButtonSaysRevert;
    QMap<QString, TypesListItem *> m_majorMap; // groups
    QList<TypesListItem *> m_itemList;

    KSharedConfig::Ptr m_fileTypesConfig;
};

// helper class for loading the icon on request instead of preloading lots of probably
// unused icons which takes quite a lot of time
class TypesListTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit TypesListTreeWidget(QWidget *parent)
        : QTreeWidget(parent)
    {
    }

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        static_cast<TypesListItem *>(itemFromIndex(index))->loadIcon();

        QTreeWidget::drawRow(painter, option, index);
    }
};

#endif
