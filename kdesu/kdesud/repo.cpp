/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <time.h>
#include <assert.h>

#include <qcstring.h>
#include <qmap.h>
#include <qvaluestack.h>

#include "repo.h"


Repository::Repository()
{
    head_time = (unsigned) -1;
}


Repository::~Repository()
{}


void Repository::add(const QCString &key, Data_entry &data)
{
    if (repo.find(key) != repo.end())
	remove(key);

    data.timeout += time(0L);
    head_time = QMIN(head_time, data.timeout);
    repo[key] = data;

    qDebug("Added key: %s", (const char *) key);
}


int Repository::remove(const QCString &key)
{
    repo_it = repo.find(key);
    if (repo_it == repo.end()) 
	return -1;
    
    repo_it.data().value.fill('x');
    repo.remove(key);

    qDebug("Deleted key: %s", (const char *) key);
    return 0;
}


const Data_entry *Repository::find(const QCString &key) const
{
    repo_cit = repo.find(key);
    if (repo_cit == repo.end())
	return 0L;

    qDebug("Found key: %s", (const char *) key);
    return &repo_cit.data();
}


int Repository::expire()
{
    unsigned current = time(0L);

    if (current < head_time)
	return 0;
    
    QValueStack<QCString> keys;

    unsigned t;
    head_time = (unsigned) -1;
    for (repo_cit=repo.begin(); repo_cit!=repo.end(); repo_cit++) {
	t = repo_cit.data().timeout;
	if (current >= t)
	    keys.push(repo_cit.key());
	else 
	    head_time = QMIN(head_time, t);
    }

    
    int n = keys.count();
    while (!keys.isEmpty())
	remove(keys.pop());
    
    qDebug("Expired %d entries", n);
    return n;
}

