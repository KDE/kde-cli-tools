/* vi: ts=8 sts=4 sw=4
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
    void add(const QCString& key, Data_entry& data);

    /** Delete a data element. */
    int remove(const QCString& key);

    /** Delete all data entries having the given group.  */
    int removeGroup(const QCString& group);

    /** Delete all data entries based on key. */
    int removeSpecialKey(const QCString& key );

    /** Checks for the existence of the specified group. */
    int hasGroup(const QCString &group) const;

    /** Return a data value.  */
    QCString find(const QCString& key) const;

    /** Returns the key values for the given group. */
    QCString findKeys(const QCString& group, const char *sep= "-") const;

private:

    QMap<QCString,Data_entry> repo;
    typedef QMap<QCString,Data_entry>::Iterator RepoIterator;
    typedef QMap<QCString,Data_entry>::ConstIterator RepoCIterator;
    unsigned head_time;
};

#endif
