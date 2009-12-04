/* This file is part of KNemo
   Copyright (C) 2004, 2005, 2006 Percy Leonhardt <percy@eris23.de>
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

#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <KCModule>

#include "data.h"
#include "ui_configdlg.h"

class QTreeWidgetItem;
class KCalendarSystem;

/**
 * This is the configuration dialog for KNemo
 * It is implemented as a control center module so that it is still
 * possible to configure KNemo even when there is no icon visible
 * in the system tray.
 *
 * @short Configuration dialog for KNemo
 * @author Percy Leonhardt <percy@eris23.de>
 */

class ConfigDialog : public KCModule
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    ConfigDialog( QWidget *parent, const QVariantList &args );

    /**
     * Default Destructor
     */
    virtual ~ConfigDialog();

    void load();
    void save();
    void defaults();

private slots:
    void buttonNewSelected();
    void buttonAllSelected();
    void buttonDeleteSelected();
    void buttonAddCommandSelected();
    void buttonRemoveCommandSelected();
    void setUpDownButtons( QTreeWidgetItem* item );
    void buttonCommandUpSelected();
    void buttonCommandDownSelected();
    void buttonAddToolTipSelected();
    void buttonRemoveToolTipSelected();
    void buttonNotificationsSelected();
    void interfaceSelected( int row );
    void aliasChanged( const QString& text );
    void iconThemeChanged( int set );
    void checkBoxDisconnectedToggled( bool on );
    void checkBoxUnavailableToggled( bool on );
    void checkBoxStatisticsToggled( bool on );
    void checkBoxCustomBillingToggled( bool on );
    void warnThresholdChanged( double val );
    void warnRxTxToggled( bool on );
    void billingStartInputChanged( const QDate& );
    void billingMonthsInputChanged( int value );
    void checkBoxStartKNemoToggled( bool on );
    void spinBoxTrafficValueChanged( int value );
    void colorButtonChanged();
    void listViewCommandsSelectionChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous );
    void listViewCommandsChanged( QTreeWidgetItem* item, int column );
    void moveTips( QListWidget *from, QListWidget *to );

private:
    void setMaxDay();
    void setupToolTipTab();
    void setupToolTipMap();
    void updateControls( InterfaceSettings *settings );
    InterfaceSettings * getItemSettings();
    int findIndexFromName( const QString& internalName );
    QString findNameFromIndex( int index );
    QPixmap textIcon( QString incomingText, QString outgoingText, int status );

    int mToolTipContent;
    bool mLock;
    Ui::ConfigDlg* mDlg;
    const KCalendarSystem* mCalendar;
    int mMaxDay;

    // Delete this once KCalendarSystem fixed
    QString mDefaultCalendarType;

    KSharedConfigPtr mConfig;
    QMap<QString, InterfaceSettings *> mSettingsMap;
    QMap<quint32, QString> mToolTips;
    QList<QString> mDeletedIfaces;
};

#endif // CONFIGDIALOG_H
