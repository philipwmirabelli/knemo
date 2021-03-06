/* This file is part of KNemo
   Copyright (C) 2006 Percy Leonhardt <percy@eris23.de>
   Copyright (C) 2009 John Stamp <jstamp@users.sourceforge.net>

   KNemo is free software; you can redistribute it and/or modify
   it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   KNemo is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef BACKENDFACTORY_H
#define BACKENDFACTORY_H

#ifdef __linux__
  #include "netlinkbackend.h"
#else
  #include "bsdbackend.h"
#endif

class BackendFactory
{
public:
    static BackendBase * backend()
    {
#ifdef __linux__
        return NetlinkBackend::createInstance();
#else
        return BSDBackend::createInstance();
#endif
    }
};

#endif // BACKENDFACTORY_H
