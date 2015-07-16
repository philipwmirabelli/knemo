/* This file is part of KNemo
   Copyright (C) 2010 John Stamp <jstamp@users.sourceforge.net>

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

#include "statsfactory.h"
#include "stats_vnstat.h"
#include <QStandardPaths>

ExternalStats * StatsFactory::stats( Interface * iface, KCalendarSystem * calendar )
{
    ExternalStats * s = NULL;
    if ( ! QStandardPaths::findExecutable("vnstat").isEmpty() )
    {
        s = new StatsVnstat( iface, calendar );
    }
    return s;
}
