/* This file is part of KNemo
   Copyright (C) 2004, 2005 Percy Leonhardt <percy@eris23.de>
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

#ifndef INTERFACEICON_H
#define INTERFACEICON_H

#include <QSystemTrayIcon>

class Interface;
class InterfaceTray;
class KAction;
class KActionCollection;
class QAction;

/**
 * This is the logical representation of the systemtray icon. It handles
 * creation and deletion of the real icon, setting the tooltip, setting
 * the correct icon image and displaying of the settings dialog.
 *
 * @short Logical representation of the systemtray icon
 * @author Percy Leonhardt <percy@eris23.de>
 */

class InterfaceIcon : public QObject
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    InterfaceIcon( Interface* interface );

    /**
     * Default Destructor
     */
    virtual ~InterfaceIcon();

    /*
     * Fill the context menu with entries if the user configured
     * start and stop command
     */
    void updateMenu();

signals:
    void statisticsSelected();

public slots:
    /*
     * Changes the icon image displayed in the tray
     */
    void updateStatus( int status );

    /*
     * Creates or deletes the tray icon
     */
    void updateTrayStatus( int previousState );

    /*
     * Change the tooltip according to the alias of the interface
     */
    void updateToolTip();

private slots:
    /*
     * Called when the user selects 'Configure KNemo' from the context menu
     */
    void showConfigDialog();

    /*
     * Called when the user setup custom commands and selects one
     * in the context menu
     */
    void menuTriggered( QAction * );

    void iconActivated( QSystemTrayIcon::ActivationReason reason );

    void showGraph();
    void showStatistics();

private:
    // the interface this icon belongs to
    Interface* mInterface;
    // the real tray icon
    InterfaceTray* mTray;
    KActionCollection* commandActions;
    KAction* plotterAction;
    KAction* statisticsAction;
    KAction* configAction;

    static const QString ICON_DISCONNECTED;
    static const QString ICON_CONNECTED;
    static const QString ICON_INCOMING;
    static const QString ICON_OUTGOING;
    static const QString ICON_TRAFFIC;
};

#endif // INTERFACEICON_H
