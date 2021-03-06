/* This file is part of KNemo
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

#ifndef BSDBACKEND_H
#define BSDBACKEND_H

#include <sys/socket.h>
#include <QStringList>

#include "backendbase.h"
#include <sys/types.h>
#include <net/if.h>
#include <net80211/ieee80211_ioctl.h>

/**
 * This backend uses getifaddrs() and friends.
 * It then triggers the interface monitor to look for changes
 * in the state of the interface.
 *
 * @short Update the information of the interfaces using system calls
 * @author John Stamp <jstamp@users.sourceforge.net>
 */

class BSDBackend : public BackendBase
{
    Q_OBJECT
public:
    BSDBackend();
    virtual ~BSDBackend();

    static BackendBase* createInstance();

    virtual void update();
    virtual QStringList ifaceList();
    virtual QString defaultRouteIface( int afInet );

private:
    void updateIfaceData( struct ifaddrs * ifap, const QString& ifName, BackendData* data );
    void updateWirelessData( const QString& ifName, BackendData* data );
    QString formattedAddr( struct sockaddr * addr );
    QString getAddr( struct ifaddrs *ifa, AddrData& addrData );
    int getSubnet( struct ifaddrs *ifa );
    int m_fd;

    int get80211( const QString &ifName, int type, void *data, int len );
    int get80211len( const QString &ifName, int type, void *data, int len, int *plen);
    int get80211id( const QString &ifName, int ix, void *data, size_t len, int *plen, int mesh );
    int get80211val( const QString &ifName, int type, int *val );
    enum ieee80211_opmode get80211opmode( const QString &ifName );
};

#endif // BSDBACKEND_H
