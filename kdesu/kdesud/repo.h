/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Repo_h_included__
#define __Repo_h_included__


#include <qmap.h>
#include <qcstring.h>


/**
 * Used internally.
 */
struct Data_entry {
    QCString value;
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

    /**
     * Expire data elements which are too old.
     */
    int expire();

    /**
     * Add a data element 
     */
    void add(const QCString &key, Data_entry &data);

    /**
     * Delete a data element. The value field is overwritten before it is
     * deleted because it could contain sensitive information.
     */
    int remove(const QCString& key);

    /**
     * Return a data entry.
     */
    const Data_entry *find(const QCString &key) const;

private:

    QMap<QCString,Data_entry> repo;
    QMap<QCString,Data_entry>::Iterator repo_it;
    mutable QMap<QCString,Data_entry>::ConstIterator repo_cit;

    unsigned head_time;
};

#endif
