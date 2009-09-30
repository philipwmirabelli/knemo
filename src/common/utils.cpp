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

   Portions taken from FreeSWITCH
       Copyright (c) 2007-2008, Thomas BERNARD <miniupnp@free.fr>

       Permission to use, copy, modify, and/or distribute this software for any
       purpose with or without fee is hereby granted, provided that the above
       copyright notice and this permission notice appear in all copies.
*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <KStandardDirs>
#include <QString>

#ifdef __linux__
  #include <netlink/route/rtnl.h>
  #include <netlink/route/route.h>
#else
  #include <net/route.h>
  #include <netinet/in.h>
  #include <unistd.h>
  #define NEXTADDR(w, u) \
        if (rtm_addrs & (w)) {\
            l = sizeof(struct sockaddr); memmove(cp, &(u), l); cp += l;\
        }
  #define rtm m_rtmsg.m_rtm
#endif

#ifdef __linux__

QString ipv4gwi;
QString ipv6gwi;

QString ipv4gw;
QString ipv6gw;

void parseNetlinkRoute( struct nl_object *object, void * )
{
    struct rtnl_route *const route = reinterpret_cast<struct rtnl_route *>(object);

    int rtfamily = rtnl_route_get_family( route );

    if ( rtfamily == AF_INET ||
         rtfamily == AF_INET6 )
    {
        struct nl_addr *dst = rtnl_route_get_dst( route );
        struct nl_addr *addr = rtnl_route_get_gateway( route );

        if ( nl_addr_get_len( dst ) == 0 && addr )
        {
            char gwaddr[ INET6_ADDRSTRLEN ];
            char gwname[ IFNAMSIZ ];
            memset( gwaddr, 0, sizeof( gwaddr ) );
            struct in_addr * inad = reinterpret_cast<struct in_addr *>(nl_addr_get_binary_addr( addr ));
            nl_addr2str( addr, gwaddr, sizeof( gwaddr ) );
            inet_ntop( rtfamily, &inad->s_addr, gwaddr, sizeof( gwaddr ) );
            int oif = rtnl_route_get_oif( route );
            if_indextoname( oif, gwname );

            if ( rtfamily == AF_INET )
            {
                ipv4gw = gwaddr;
                ipv4gwi = gwname;
            }
            else if ( rtfamily == AF_INET6 )
            {
                ipv6gw = gwaddr;
                ipv6gwi = gwname;
            }
        }
    }
}

QString getNetlinkRoute( int afType, QString *defaultGateway, void *data )
{
    if ( !data )
        return QString();

    struct nl_cache* rtlcache = static_cast<struct nl_cache*>(data);

    nl_cache_foreach( rtlcache, parseNetlinkRoute, NULL);

    if ( afType == AF_INET )
    {
        if ( defaultGateway )
            *defaultGateway = ipv4gw;
        return ipv4gwi;
    }
    else
    {
        if ( defaultGateway )
            *defaultGateway = ipv6gw;
        return ipv6gwi;
    }
}
#else

QString getSocketRoute( int afType, QString *defaultGateway )
{
    struct
    {
        struct rt_msghdr m_rtm;
        char m_space[ 512 ];
    } m_rtmsg;

    int s, seq, l, rtm_addrs, i;
    pid_t pid;
    struct sockaddr so_dst, so_mask;
    char *cp = m_rtmsg.m_space;
    struct sockaddr *gate = NULL, *sa;
    struct rt_msghdr *msg_hdr;

    char outBuf[ INET6_ADDRSTRLEN ];
    memset( &outBuf, 0, sizeof( outBuf ) );
    void *tempAddrPtr = NULL;
    QString ifname;

    pid = getpid();
    seq = 0;
    rtm_addrs = RTA_DST | RTA_NETMASK;

    memset( &so_dst, 0, sizeof( so_dst ) );
    memset( &so_mask, 0, sizeof( so_mask ) );
    memset( &rtm, 0, sizeof( struct rt_msghdr ) );

    rtm.rtm_type = RTM_GET;
    rtm.rtm_flags = RTF_UP | RTF_GATEWAY;
    rtm.rtm_version = RTM_VERSION;
    rtm.rtm_seq = ++seq;
    rtm.rtm_addrs = rtm_addrs;

    if ( afType == AF_INET )
    {
        so_dst.sa_family = AF_INET;
        so_mask.sa_family = AF_INET;
    }
    else
    {
        so_dst.sa_family = AF_INET6;
        so_mask.sa_family = AF_INET6;
    }

    NEXTADDR( RTA_DST, so_dst );
    NEXTADDR( RTA_NETMASK, so_mask );

    rtm.rtm_msglen = l = cp - reinterpret_cast<char *>(&m_rtmsg);

    s = socket(PF_ROUTE, SOCK_RAW, 0);

    if ( write( s, reinterpret_cast<char *>(&m_rtmsg), l ) < 0 )
    {
        close( s );
        return ifname;
    }

    do
    {
        l = read(s, reinterpret_cast<char *>(&m_rtmsg), sizeof( m_rtmsg ) );
    } while ( l > 0 && (rtm.rtm_seq != seq || rtm.rtm_pid != pid) );

    close( s );

    msg_hdr = &rtm;

    cp = reinterpret_cast<char *>(msg_hdr + 1);
    if ( msg_hdr->rtm_addrs )
    {
        for ( i = 1; i; i <<= 1 )
            if ( i & msg_hdr->rtm_addrs )
            {
                sa = reinterpret_cast<struct sockaddr *>(cp);
                if ( i == RTA_GATEWAY )
                {
                    gate = sa;
                    char tempname[ IFNAMSIZ ];
                    if_indextoname( msg_hdr->rtm_index, tempname );
                    ifname = tempname;
                }

                cp += sizeof( struct sockaddr );
            }
    }
    else
        return ifname;

    if ( AF_INET == afType )
        tempAddrPtr = & reinterpret_cast<struct sockaddr_in *>(gate)->sin_addr;
    else
        tempAddrPtr = & reinterpret_cast<struct sockaddr_in6 *>(gate)->sin6_addr;
    inet_ntop( gate->sa_family, tempAddrPtr, outBuf, sizeof( outBuf ) );
    if ( defaultGateway && strncmp( outBuf, "0.0.0.0", 7 ) != 0 )
        *defaultGateway = outBuf;
    return ifname;
}
#endif

QString getDefaultRoute( int afType, QString *defaultGateway, void *data )
{
#ifdef __linux__
    return getNetlinkRoute( afType, defaultGateway, data );
#else
    return getSocketRoute( afType, defaultGateway );
#endif
}

QStringList findIconSets()
{
    KStandardDirs iconDirs;
    iconDirs.addResourceType("knemo_pics", "data", "knemo/pics");
    QStringList iconlist = iconDirs.findAllResources( "knemo_pics", "*.png" );

    QStringList iconSets;
    foreach ( QString iconName, iconlist )
    {
        QRegExp rx( "pics\\/(.+)_(connected|disconnected|incoming|outgoing|traffic)\\.png" );
        if ( rx.indexIn( iconName ) > -1 )
            if ( !iconSets.contains( rx.cap( 1 ) ) )
                iconSets << rx.cap( 1 );
    }
    return iconSets;
}