/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <time.h>
#include <assert.h>

#include <qcstring.h>
#include <qmap.h>
#include <qvaluestack.h>
#include <kdebug.h>

#include "repo.h"


Repository::Repository()
{
    head_time = (unsigned) -1;
}


Repository::~Repository()
{}


void Repository::add(const QCString &key, Data_entry &data)
{
    RepoIterator it = repo.find(key);
    if (it != repo.end())
	remove(key);
    if (data.timeout == 0)
	data.timeout = (unsigned) -1;
    else
	data.timeout += time(0L);
    head_time = QMIN(head_time, data.timeout);
    repo.insert(key, data);
}


int Repository::remove(const QCString &key)
{
    RepoIterator it = repo.find(key);
    if (it == repo.end()) 
	return -1;
    it.data().value.fill('x');
    it.data().group.fill('x');
    repo.remove(key);
    return 0;
}


int Repository::removeGroup(const QCString &group)
{
    RepoIterator it;
    for (it=repo.begin(); it!=repo.end(); it++)
    {
	if (it.data().group == group)
	{
	    repo.remove(it.key());
	    return 0;
	}
    }
    return -1;
}

    
QCString Repository::find(const QCString &key) const
{
    RepoCIterator it = repo.find(key);
    if (it == repo.end())
	return 0;
    return it.data().value;
}


int Repository::expire()
{
    unsigned current = time(0L);
    if (current < head_time)
	return 0;
    
    unsigned t;
    QValueStack<QCString> keys;
    head_time = (unsigned) -1;
    RepoIterator it;
    for (it=repo.begin(); it!=repo.end(); it++) 
    {
	t = it.data().timeout;
	if (t <= current)
	    keys.push(it.key());
	else 
	    head_time = QMIN(head_time, t);
    }

    int n = keys.count();
    while (!keys.isEmpty())
	remove(keys.pop());
    return n;
}

