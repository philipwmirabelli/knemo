/* This file is part of KNemo
   Copyright (C) 2006 Percy Leonhardt <percy@eris23.de>

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

#include <stdio.h>

#include <qmap.h>
#include <qdir.h>
#include <qregexp.h>
#include <qstringlist.h>

#include <kdebug.h>
#include <kprocess.h>
#include <kio/global.h>

#include "sysbackend.h"

#include "config.h"

#define SYSPATH "/sys/class/net/"

SysBackend::SysBackend( QDict<Interface>& interfaces )
    : BackendBase( interfaces )
{
}

SysBackend::~SysBackend()
{
}

BackendBase* SysBackend::createInstance( QDict<Interface>& interfaces )
{
    return new SysBackend( interfaces );
}

void SysBackend::update()
{
    QDir dir( SYSPATH );
    QStringList ifList = dir.entryList( QDir::Dirs );

    QDictIterator<Interface> ifIt( mInterfaces );
    for ( ; ifIt.current(); ++ifIt )
    {
        QString key = ifIt.currentKey();
        Interface* interface = ifIt.current();

        if ( ifList.find( key ) == ifList.end() )
        {
            // The interface does not exist. Meaning the driver
            // isn't loaded and/or the interface has not been created.
            interface->getData().existing = false;
            interface->getData().available = false;
        }
        else
        {
            if ( QFile::exists( SYSPATH + key + "/wireless" ) )
            {
                interface->getData().wirelessDevice = true;
            }

            unsigned int carrier = 0;
            if ( !readNumberFromFile( SYSPATH + key + "/carrier", carrier ) ||
                 carrier == 0 )
            {
                // The interface is there but not useable.
                interface->getData().existing = true;
                interface->getData().available = false;
            }
            else
            {
                // ...determine the type of the interface
                unsigned int type = 0;
                if ( readNumberFromFile( SYSPATH + key + "/type", type ) &&
                     type == 512 )
                {
                    interface->setType( Interface::PPP );
                }
                else
                {
                    interface->setType( Interface::ETHERNET );
                }

                // Update the interface.
                interface->getData().existing = true;
                interface->getData().available = true;
                updateInterfaceData( key, interface->getData(), interface->getType() );

                if ( interface->getData().wirelessDevice == true )
                {
                    updateWirelessData( key, interface->getWirelessData() );
                }
            }
        }
    }
    updateComplete();
}

bool SysBackend::readNumberFromFile( const QString& fileName, unsigned int& value )
{
    FILE* file = fopen( fileName.latin1(), "r" );
    if ( file != NULL )
    {
        if ( fscanf( file, "%ul", &value ) > 0 )
        {
            fclose( file );
            return true;
        }
        fclose( file );
    }

    return false;
}

bool SysBackend::readStringFromFile( const QString& fileName, QString& string )
{
    char buffer[64];
    FILE* file = fopen( fileName.latin1(), "r" );
    if ( file != NULL )
    {
        if ( fscanf( file, "%s", buffer ) > 0 )
        {
            fclose( file );
            string = buffer;
            return true;
        }
        fclose( file );
    }

    return false;
}

void SysBackend::updateInterfaceData( const QString& ifName, InterfaceData& data, int type )
{
    QString ifFolder = SYSPATH + ifName + "/";

    unsigned int rxPackets = 0;
    if ( readNumberFromFile( ifFolder + "statistics/rx_packets", rxPackets ) )
    {
        data.rxPackets = rxPackets;
    }

    unsigned int txPackets = 0;
    if ( readNumberFromFile( ifFolder + "statistics/tx_packets", txPackets ) )
    {
        data.txPackets = txPackets;
    }

    unsigned int rxBytes = 0;
    if ( readNumberFromFile( ifFolder + "statistics/rx_bytes", rxBytes ) )
    {
        // We count the traffic on ourself to avoid an overflow after
        // 4GB of traffic.
        if ( rxBytes < data.prevRxBytes )
        {
            // there was an overflow
            data.rxBytes += 0x7FFFFFFF - data.prevRxBytes;
            data.prevRxBytes = 0L;
        }
        if ( data.rxBytes == 0L )
        {
            // on startup set to currently received bytes
            data.rxBytes = rxBytes;
            // this is new: KNemo only counts the traffic transfered
            // while it is running. Important to not falsify statistics!
            data.prevRxBytes = rxBytes;
        }
        else
            // afterwards only add difference to previous number of bytes
            data.rxBytes += rxBytes - data.prevRxBytes;

        data.incomingBytes = rxBytes - data.prevRxBytes;
        data.prevRxBytes = rxBytes;
        data.rxString = KIO::convertSize( data.rxBytes );
    }

    unsigned int txBytes = 0;
    if ( readNumberFromFile( ifFolder + "statistics/tx_bytes", txBytes ) )
    {
        // We count the traffic on ourself to avoid an overflow after
        // 4GB of traffic.
        if ( txBytes < data.prevTxBytes )
        {
            // there was an overflow
            data.txBytes += 0x7FFFFFFF - data.prevTxBytes;
            data.prevTxBytes = 0L;
        }
        if ( data.txBytes == 0L )
        {
            // on startup set to currently received bytes
            data.txBytes = txBytes;
            // this is new: KNemo only counts the traffic transfered
            // while it is running. Important to not falsify statistics!
            data.prevTxBytes = txBytes;
        }
        else
            // afterwards only add difference to previous number of bytes
            data.txBytes += txBytes - data.prevTxBytes;

        data.incomingBytes = txBytes - data.prevTxBytes;
        data.prevTxBytes = txBytes;
        data.txString = KIO::convertSize( data.txBytes );
    }

    if ( type == Interface::ETHERNET )
    {
        QString hwAddress;
        if ( readStringFromFile( ifFolder + "address", hwAddress ) )
        {
            data.hwAddress = hwAddress;
        }
    }
}

void SysBackend::updateWirelessData( const QString& ifName, WirelessData& data )
{
    QString wirelessFolder = SYSPATH + ifName + "/wireless/";

    unsigned int link = 0;
    if ( readNumberFromFile( wirelessFolder + "link", link ) )
    {
        data.linkQuality = QString::number( link );
    }
}
