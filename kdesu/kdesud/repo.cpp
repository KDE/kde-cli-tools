/* vi: ts=8 sts=4 sw=4
 *
 * $Id$
 *
 * This file is part of the KDE project, module kdesu.
 * Copyright (C) 1999 Geert Jansen <g.t.jansen@stud.tue.nl>
 */

#include <time.h>
#include <assert.h>

#include <queue>
#include <map>
#include <stack>
#include <string>

#include "repo.h"
#include "kdesud.h"
#include "debug.h"


Repository::Repository()
{
    head_time = (unsigned) -1;
}


Repository::~Repository()
{}


void Repository::add(const string &key, Data_entry *data)
{
    if (repo.find(key) != repo.end())
	erase(key);

    data->timeout += time(0L);
    head_time = min(head_time, data->timeout);
    repo[key] = *data;

    debug("Added key: %s", key.c_str());
}


int Repository::erase(const string &key)
{
    repo_it = repo.find(key);
    if (repo_it == repo.end()) 
	return -1;
    
    clear(repo_it->second.value);
    repo.erase(key);

    debug("Deleted key: %s", key.c_str());
    return 0;
}


const Data_entry *Repository::find(const string &key) const
{
    repo_cit = repo.find(key);
    if (repo_cit == repo.end())
	return 0L;

    debug("Found key: %s", key.c_str());
    return &(repo_cit->second);
}


int Repository::expire()
{
    unsigned current = time(0L);

    if (current < head_time)
	return 0;
    
    stack<string> keys;

    unsigned t;
    head_time = (unsigned) -1;
    for (repo_cit=repo.begin(); repo_cit!=repo.end(); repo_cit++) {
	t = repo_cit->second.timeout;
	if (current >= t)
	    keys.push(repo_cit->first);
	else 
	    head_time = min(head_time, t);
    }

    string s;
    int n = keys.size();
    while (keys.size()) {
	s = keys.top();
	erase(s);
	keys.pop();
    }
    
    debug("Expired %d entries", n);
    return n;
}

