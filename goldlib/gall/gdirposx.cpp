//  This may look like C code, but it is really -*- C++ -*-

//  ------------------------------------------------------------------
//  The Goldware Library
//  Copyright (C) 1999-2000 Alexander S. Aganichev
//  ------------------------------------------------------------------
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Library General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the Free
//  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//  MA 02111-1307, USA
//  ------------------------------------------------------------------
//  $Id$
//  ------------------------------------------------------------------
//  Directory maintenance class.
//  ------------------------------------------------------------------

#include <gdirposx.h>
#include <gstrall.h>
#include <gwildmat.h>
#include <dirent.h>
#ifndef __HAVE_DRIVES__
#include <pwd.h>
#endif


//  ------------------------------------------------------------------

gdirentry::gdirentry() : name(), dirname(NULL)
{
}


//  ------------------------------------------------------------------

gdirentry::~gdirentry()
{
}


//  ------------------------------------------------------------------

gposixdir::gposixdir() : dirname(), ret(), entries()
{
	ok = false;
}


//  ------------------------------------------------------------------

gposixdir::gposixdir(const char *name) : dirname(), ret(), entries()
{
	cd(name);
}


//  ------------------------------------------------------------------

gposixdir::~gposixdir()
{
}


//  ------------------------------------------------------------------

void gposixdir::cd(const char *name, bool relative)
{
	std::string ndirname;
	if(!*name)
		name = ".";
	if(relative) {
		dirname += "/";
		dirname += name;
	} else
		dirname = name;
	ok = maketruepath(dirname);
	entries.clear();
	DIR *d = opendir(dirname.c_str());
	if(d == NULL)
		ok = false;
	else {
		struct dirent *de;
		while((de=readdir(d)) != NULL) {
			ndirname = de->d_name;
#ifdef __HAVE_DRIVES__
			if((ndirname != ".") && !((ndirname == "..") && streql(dirname.c_str()+1, ":/")))
#else
			if((ndirname != ".") && !((ndirname == "..") && (dirname == "/")))
#endif
				entries.push_back(ndirname);
		}
		closedir(d);
		rewind();
	}
}


//  ------------------------------------------------------------------

const gdirentry *gposixdir::nextentry(const char *mask, bool nameonly)
{
	while(last_entry < entries.size()) {
		if(mask && !gwildmat(entries[last_entry].c_str(), mask)) {
			++last_entry;
			continue;
		}
		ret.name = entries[last_entry];
		ret.dirname = dirname.c_str();
		if(!nameonly) {
			std::string pn = ret.dirname;
			pn += "/";
			pn += ret.name;
			size_t skipfrom;
			while((skipfrom=pn.find("//")) != pn.npos)
				pn.erase(skipfrom, 1);
			stat(pn.c_str(), &ret.stat_info);
		}
		++last_entry;
		return &ret;
	}
	return NULL;
}


//  ------------------------------------------------------------------
