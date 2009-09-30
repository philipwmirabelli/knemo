/* This file is part of KNemo
   Copyright (C) 2004 Percy Leonhardt <percy@eris23.de>
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

#ifndef DATA_H
#define DATA_H

#include <algorithm>
#include <QString>
#include <QList>

/**
 * This file contains data structures used to store information about
 * an interface. It is shared between the daemon and the control center
 * module.
 *
 * @short Shared data structures
 * @author Percy Leonhardt <percy@eris23.de>
 */

using namespace std;

/* This is for clamping min/max values read from the settings file */
template <class T> inline T clamp(T x, T a, T b)
{
	return min(max(x,a),b);
}

struct InterfaceCommand
{
    bool runAsRoot;
    QString command;
    QString menuText;
};

struct InterfaceSettings
{
    InterfaceSettings()
      : numCommands( 0 ),
        trafficThreshold( 0 ),
        hideWhenNotExisting( false ),
        hideWhenNotAvailable( false ),
        activateStatistics( false ),
        customCommands( false )
    {}

    QString iconSet;
    int numCommands;
    int trafficThreshold;
    bool hideWhenNotExisting;
    bool hideWhenNotAvailable;
    bool activateStatistics;
    bool customCommands;
    QString alias;
    QList<InterfaceCommand> commands;
};

#ifndef __linux__
enum rt_scope_t
{
    RT_SCOPE_UNIVERSE=0,
    RT_SCOPE_SITE=200,
    RT_SCOPE_LINK=253,
    RT_SCOPE_HOST=254,
    RT_SCOPE_NOWHERE=255
};
#endif

enum ToolTipEnums
{
    INTERFACE        = 0x00000001,
    ALIAS            = 0x00000002,
    STATUS           = 0x00000004,
    UPTIME           = 0x00000008,
    IP_ADDRESS       = 0x00000010,
    SCOPE            = 0x00000020,
    HW_ADDRESS       = 0x00000040,
    PTP_ADDRESS      = 0x00000080,
    RX_PACKETS       = 0x00000100,
    TX_PACKETS       = 0x00000200,
    RX_BYTES         = 0x00000400,
    TX_BYTES         = 0x00000800,
    ESSID            = 0x00001000,
    MODE             = 0x00002000,
    FREQUENCY        = 0x00004000,
    BIT_RATE         = 0x00008000,
    ACCESS_POINT     = 0x00010000,
    LINK_QUALITY     = 0x00020000,
    BCAST_ADDRESS    = 0x00040000,
    GATEWAY          = 0x00080000,
    DOWNLOAD_SPEED   = 0x00100000,
    UPLOAD_SPEED     = 0x00200000,
    NICK_NAME        = 0x00400000,
    ENCRYPTION       = 0x00800000
};

const static int defaultTip = INTERFACE | STATUS | IP_ADDRESS | ESSID | LINK_QUALITY | DOWNLOAD_SPEED | UPLOAD_SPEED;

#endif // DATA_H
