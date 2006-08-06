/* This file is part of KNemo
   Copyright (C) 2004, 2006 Percy Leonhardt <percy@eris23.de>

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

#ifndef KNEMODAEMON_H
#define KNEMODAEMON_H

#include <qdict.h>
#include <qcolor.h>
#include <qcstring.h>
#include <qdatetime.h>

#include <kdedmodule.h>
#include <knotifyclient.h>

#include "data.h"
#include "global.h"

class QTimer;
class KInstance;
class Interface;
class BackendBase;
class KNotifyClient::Instance;

/**
 * This class is the main entry point of KNemo. It reads the configuration,
 * creates the logical interfaces and starts an interface updater. It also
 * takes care of configuration changes by the user.
 *
 * @short KNemos main entry point
 * @author Percy Leonhardt <percy@eris23.de>
 */
class KNemoDaemon : public KDEDModule
{
    K_DCOP
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KNemoDaemon( const QCString& name );

    /**
     * Default Destructor
     */
    virtual ~KNemoDaemon();

    // tell the control center module which interface the user selected
    static QString sSelectedInterface;

k_dcop:
    /*
     * Called from the control center module when the user changed
     * the settings. It updates the internal list of interfaces
     * that should be monitored.
     */
    virtual void reparseConfiguration();

    /* When the user selects 'Configure KNemo...' from the context
     * menus this functions gets called from the control center
     * module. This way the module knows for which interface the
     * user opened the dialog and can preselect the appropriate
     * interface in the list.
     */
    virtual QString getSelectedInterface();

private:
    /*
     * Read the configuration on startup
     */
    void readConfig();

private slots:
    /**
     * trigger the backend to update the interface informations
     */
    void updateInterfaces();

private:
    QColor mColorVLines;
    QColor mColorHLines;
    QColor mColorIncoming;
    QColor mColorOutgoing;
    QColor mColorBackground;

    // needed to calculate the number of
    // seconds since the last update
    QDateTime mLastUpdateTime;
    // every time this timer expires we will
    // gather new informations from the backend
    QTimer* mPollTimer;
    // our own instance
    KInstance* mInstance;
    // needed so that KNotifyClient::event will work
    KNotifyClient::Instance* mNotifyInstance;
    // application wide settings are stored here
    GeneralData mGeneralData;
    // settings for the traffic plotter are stored here
    PlotterSettings mPlotterSettings;
    // the name of backend we currently use
    QString mBackendName;
    // the backend used to update the interface informations
    BackendBase* mBackend;
    // a list of all interfaces the user wants to monitor
    QDict<Interface> mInterfaceDict;
};

#endif // KNEMODAEMON_H
