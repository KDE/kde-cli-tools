/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 */

#ifndef __Repo_h_included__
#define __Repo_h_included__


#include <qmap.h>
#include <qcstring.h>


/**
 * Used internally.
 */
struct Data_entry 
{
    QCString value;
    QCString group;
    unsigned int timeout;
};


/**
 * String repository.
 *
 * This class implements a string repository with expiration.
 */
class Repository {
public:
    Repository();
    ~Repository();

    /** Remove data elements which are expired. */
    int expire();

    /** Add a data element */
    void add(const QCString &key, Data_entry &data);

    /** Delete a data element. */
    int remove(const QCString& key);

    /** Delete all data entries having a given group.  */
    int removeGroup(const QCString& group);

    /** Return a data value.  */
    QCString find(const QCString &key) const;

private:

    QMap<QCString,Data_entry> repo;
    typedef QMap<QCString,Data_entry>::Iterator RepoIterator;
    typedef QMap<QCString,Data_entry>::ConstIterator RepoCIterator;
    unsigned head_time;
};

#endif
