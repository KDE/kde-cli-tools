/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999,2000 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include "repo.h"

#include <time.h>
#include <assert.h>

#include <QStack>
#include <KDebug>


Repository::Repository()
{
    head_time = (unsigned) -1;
}


Repository::~Repository()
{}


void Repository::add(const QByteArray &key, Data_entry &data)
{
    RepoIterator it = repo.find(key);
    if (it != repo.end())
        remove(key);
    if (data.timeout == 0)
        data.timeout = (unsigned) -1;
    else
        data.timeout += time(0L);
    head_time = qMin(head_time, data.timeout);
    repo.insert(key, data);
}

int Repository::remove(const QByteArray &key)
{
    if( key.isEmpty() )
        return -1;

     RepoIterator it = repo.find(key);
     if (it == repo.end())
        return -1;
     it.value().value.fill('x');
     it.value().group.fill('x');
     repo.erase(it);
     return 0;
}

int Repository::removeSpecialKey(const QByteArray &key)
{
    int found = -1;
    if ( !key.isEmpty() )
    {
        QStack<QByteArray> rm_keys;
        for (RepoCIterator it=repo.constBegin(); it!=repo.constEnd(); ++it)
        {
            if (  key.indexOf( it.value().group ) == 0 &&
                  it.key().indexOf( key ) >= 0 )
            {
                rm_keys.push(it.key());
                found = 0;
            }
        }
        while (!rm_keys.isEmpty())
        {
            kDebug(1205) << "Removed key: " << rm_keys.top();
            remove(rm_keys.pop());
        }
    }
    return found;
}

int Repository::removeGroup(const QByteArray &group)
{
    int found = -1;
    if ( !group.isEmpty() )
    {
        QStack<QByteArray> rm_keys;
        for (RepoCIterator it=repo.constBegin(); it!=repo.constEnd(); ++it)
        {
            if (it.value().group == group)
            {
                rm_keys.push(it.key());
                found = 0;
            }
        }
        while (!rm_keys.isEmpty())
        {
            kDebug(1205) << "Removed key: " << rm_keys.top();
            remove(rm_keys.pop());
        }
    }
    return found;
}

int Repository::hasGroup(const QByteArray &group) const
{
    if ( !group.isEmpty() )
    {
        RepoCIterator it;
        for (it=repo.begin(); it!=repo.end(); ++it)
        {
            if (it.value().group == group)
                return 0;
        }
    }
    return -1;
}

QByteArray Repository::findKeys(const QByteArray &group, const char *sep ) const
{
    QByteArray list="";
    if( !group.isEmpty() )
    {
        kDebug(1205) << "Looking for matching key with group key: " << group;
        int pos;
        QByteArray key;
        RepoCIterator it;
        for (it=repo.begin(); it!=repo.end(); ++it)
        {
            if (it.value().group == group)
            {
                key = it.key();
                kDebug(1205) << "Matching key found: " << key;
                pos = key.lastIndexOf(sep);
                key.truncate( pos );
                key.remove(0, 2);
                if (!list.isEmpty())
                {
                    // Add the same keys only once please :)
                    if( !list.contains(key) )
                    {
                        kDebug(1205) << "Key added to list: " << key;
                        list += '\007'; // I do not know
                        list.append(key);
                    }
                }
                else
                    list = key;
            }
        }
    }
    return list;
}

QByteArray Repository::find(const QByteArray &key) const
{
    if( key.isEmpty() )
        return 0;

    RepoCIterator it = repo.find(key);
    if (it == repo.end())
        return 0;
    return it.value().value;
}


int Repository::expire()
{
    unsigned current = time(0L);
    if (current < head_time)
	return 0;

    unsigned t;
    QStack<QByteArray> keys;
    head_time = (unsigned) -1;
    RepoIterator it;
    for (it=repo.begin(); it!=repo.end(); ++it)
    {
	t = it.value().timeout;
	if (t <= current)
	    keys.push(it.key());
	else
	    head_time = qMin(head_time, t);
    }

    int n = keys.count();
    while (!keys.isEmpty())
	remove(keys.pop());
    return n;
}

