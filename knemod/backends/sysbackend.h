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

#ifndef SYSBACKEND_H
#define SYSBACKEND_H

#include "backendbase.h"

/**
 * The sys backend uses the sys filesystem available in 2.6
 * kernels. It reads all necessary information from the files
 * and folders located at /sys and parses their output.
 * It then triggers the interface monitor to look for changes
 * in the state of the interface.
 *
 * @short Update the information of the interfaces via sys filesystem
 * @author Percy Leonhardt <percy@eris23.de>
 */

class SysBackend : public BackendBase
{
public:
    SysBackend(QDict<Interface>& interfaces );
    virtual ~SysBackend();

    static BackendBase* createInstance( QDict<Interface>& interfaces );

    void update();

private:
    bool readNumberFromFile( const QString& fileName, unsigned int& value );
    bool readStringFromFile( const QString& fileName, QString& string );
    void updateWirelessData( const QString& ifName, WirelessData& data );
    void updateInterfaceData( const QString& ifName, InterfaceData& data, int type );

};

#endif // SYSBACKEND_H
