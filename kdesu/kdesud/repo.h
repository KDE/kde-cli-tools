/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#ifndef __Repo_h_included__
#define __Repo_h_included__


#include <map>
#include <queue>
#include <string>


struct Data_entry {
    string value;
    unsigned int timeout;
};


/**
 * String repository.
 *
 * This class implements a string repository with expriation. The
 * strings are indexed by a key which is also a string.
 * Internally, the key/value data is stored in a STL map.
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
    void add(const string &key, Data_entry *data);

    /**
     * Delete a data element. The value field is overwritten before it is
     * deleted because it could contain sensitive information.
     */
    int erase(const string& key);

    /**
     * Return a data entry.
     */
    const Data_entry *find(const string &key) const;

private:

    map<string,Data_entry> repo;
    map<string,Data_entry>::iterator repo_it;
    mutable map<string,Data_entry>::const_iterator repo_cit;

    unsigned head_time;
};

#endif
