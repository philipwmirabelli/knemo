/* This file is part of KNemo
   Copyright (C) 2004, 2005 Percy Leonhardt <percy@eris23.de>
   Copyright (C) 2009, 2010 John Stamp <jstamp@users.sourceforge.net>

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

#include <math.h>
#include <unistd.h>

#include <QPainter>
#include <QPixmapCache>

#include <QAction>
#include <KActionCollection>
#include <KColorScheme>
#include <KConfigGroup>
#include <KHelpMenu>
#include <QIcon>
#include <KLocalizedString>
#include <QMenu>
#include <KAboutData>
#include <KProcess>
#include <Plasma/Theme>

#include "global.h"
#include "utils.h"
#include "interface.h"
#include "knemodaemon.h"
#include "interfaceicon.h"
#include "interfacetray.h"

#define SHRINK_MAX 0.75
#define HISTSIZE_STORE 0.5

InterfaceIcon::InterfaceIcon( Interface* interface )
    : QObject(),
      mInterface( interface ),
      mTray( 0L ),
      barIncoming( 0 ),
      barOutgoing( 0 )
{
    statusAction = new QAction( i18n( "Show &Status Dialog" ), this );
    plotterAction = new QAction( QIcon::fromTheme( QLatin1String("utilities-system-monitor") ),
                       i18n( "Show &Traffic Plotter" ), this );
    statisticsAction = new QAction( QIcon::fromTheme( QLatin1String("view-statistics") ),
                          i18n( "Show St&atistics" ), this );
    configAction = new QAction( QIcon::fromTheme( QLatin1String("configure") ),
                       i18n( "&Configure KNemo..." ), this );

    connect( statusAction, SIGNAL( triggered() ),
             this, SLOT( showStatus() ) );
    connect( plotterAction, SIGNAL( triggered() ),
             this, SLOT( showGraph() ) );
    connect( statisticsAction, SIGNAL( triggered() ),
             this, SLOT( showStatistics() ) );
    connect( configAction, SIGNAL( triggered() ),
             this, SLOT( showConfigDialog() ) );
}

InterfaceIcon::~InterfaceIcon()
{
    delete mTray;
}

void InterfaceIcon::configChanged()
{
    histSize = HISTSIZE_STORE/generalSettings->pollInterval;
    if ( histSize < 2 )
        histSize = 2;

    for ( int i=0; i < histSize; i++ )
    {
        inHist.append( 0 );
        outHist.append( 0 );
    }

    maxRate = mInterface->settings().maxRate;

    updateTrayStatus();

    if ( mTray != 0L )
    {
        updateMenu();
        if ( mInterface->settings().iconTheme == TEXT_THEME )
             updateIconText( true );
        else if ( mInterface->settings().iconTheme == NETLOAD_THEME )
             updateBars( true );
    }
}

void InterfaceIcon::updateIconImage( int status )
{
    if ( mTray == 0L || mInterface->settings().iconTheme == TEXT_THEME )
        return;

    QString iconName;
    if ( mInterface->settings().iconTheme == SYSTEM_THEME )
        iconName = QStringLiteral("network-");
    else
        iconName = QLatin1String("knemo-") + mInterface->settings().iconTheme + QLatin1Char('-');

    // Now set the correct icon depending on the status of the interface.
    if ( ( status & KNemoIface::RxTraffic ) &&
         ( status & KNemoIface::TxTraffic ) )
    {
        iconName += ICON_RX_TX;
    }
    else if ( status & KNemoIface::RxTraffic )
    {
        iconName += ICON_RX;
    }
    else if ( status & KNemoIface::TxTraffic )
    {
        iconName += ICON_TX;
    }
    else if ( status & KNemoIface::Connected )
    {
        iconName += ICON_IDLE;
    }
    else if ( status & KNemoIface::Available )
    {
        iconName += ICON_OFFLINE;
    }
    else
    {
        iconName += ICON_ERROR;
    }
    mTray->setIconByName( iconName );
}

int InterfaceIcon::calcHeight( int iconHeight, QList<unsigned long>& hist, unsigned int& net_max )
{
    unsigned long histcalculate = 0;
    unsigned long rate = 0;

    foreach( unsigned long j, hist )
    {
        histcalculate += j;
    }
    rate = histcalculate / histSize;

    /* update maximum */
    if ( !mInterface->settings().barScale )
    {
        QList<unsigned long>sortedMax( hist );
        qSort( sortedMax );
        unsigned long max = sortedMax.last();
        int multiplier = 1024;
        if ( generalSettings->useBitrate )
            multiplier = 1000;
        if( rate > net_max )
        {
            net_max = rate;
        }
        else if( max < net_max * SHRINK_MAX
                && net_max * SHRINK_MAX >= multiplier )
        {
            net_max *= SHRINK_MAX;
        }
    }
    qreal ratio = static_cast<double>(rate)/net_max;
    if ( ratio > 1.0 )
        ratio = 1.0;
    return ratio*iconHeight;
}

QColor InterfaceIcon::calcColor( const QColor& low )
{
    const BackendData * data = mInterface->backendData();

    if ( data->status & KNemoIface::Connected )
        return low;
    else if ( data->status & KNemoIface::Available )
        return KColorScheme(QPalette::Active).foreground(KColorScheme::InactiveText).color();
    else
        return KColorScheme(QPalette::Active).foreground(KColorScheme::NegativeText).color();
}

void InterfaceIcon::updateBars( bool doUpdate )
{
    // Has color changed?
    QColor rxColor = calcColor( KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::ActiveText).color() );
    QColor txColor = calcColor( KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::NeutralText).color() );
    if ( rxColor != colorIncoming )
    {
        doUpdate = true;
        colorIncoming = rxColor;
    }

    QSize iconSize = getIconSize();

    // Has height changed?
    // FIXME: they should have same scale!!
    int rateIn = calcHeight( iconSize.height(), inHist, maxRate );
    int rateOut = calcHeight( iconSize.height(), outHist, maxRate );
    if ( rateIn != barIncoming )
    {
        doUpdate = true;
        barIncoming = rateIn;
    }
    if ( rateOut != barOutgoing )
    {
        doUpdate = true;
        barOutgoing = rateOut;
    }

    if ( !doUpdate )
        return;

    int barWidth = static_cast<int>(round(iconSize.width()/3.0) + 0.5);
    int margins = iconSize.width() - (barWidth*2);
    int midMargin = static_cast<int>(round(margins/3.0) + 0.5);
    int outerMargin = static_cast<int>(round((margins - midMargin)/2.0) + 0.5);
    midMargin = outerMargin + barWidth + midMargin;
    QPixmap barIcon(iconSize);
    barIcon.fill( Qt::transparent );

    int top = iconSize.height() - barOutgoing;
    QRect topLeftRect( outerMargin, 0, barWidth, top );
    QRect leftRect( outerMargin, top, barWidth, iconSize.height() );
    top = iconSize.height() - barIncoming;
    QRect topRightRect( midMargin, 0, barWidth, top );
    QRect rightRect( midMargin, top, barWidth, iconSize.height() );

    const BackendData * data = mInterface->backendData();
    QColor bgColor;
    if ( data->status & KNemoIface::Connected )
    {
        bgColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::InactiveText).color();
        bgColor.setAlpha( 77 );
    }
    else if ( data->status & KNemoIface::Available )
    {
        bgColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::InactiveText).color();
        rxColor.setAlpha( 153 );
    }
    else
    {
        bgColor = KColorScheme(QPalette::Active, KColorScheme::Window).foreground(KColorScheme::NegativeText).color();
    }

    QPainter p( &barIcon );
    p.fillRect( leftRect, txColor );
    p.fillRect( rightRect, rxColor );
    p.fillRect( topLeftRect, bgColor );
    p.fillRect( topRightRect, bgColor );
    mTray->setIconByPixmap( barIcon );
    QPixmapCache::clear();
}

QString InterfaceIcon::compactTrayText(unsigned long data )
{
    QString dataString;
    // Space is tight, so no space between number and units, and the complete
    // string should be no more than 4 chars.
    /* Visually confusing to display bytes
    if ( bytes < 922 ) // 922B = 0.9K
        byteString = i18n( "%1B", bytes );
    */
    double multiplier = 1024;
    if ( generalSettings->useBitrate )
        multiplier = 1000;

    int precision = 0;
    if ( data < multiplier*9.95 ) // < 9.95K
    {
        precision = 1;
    }
    if ( data < multiplier*999.5 ) // < 999.5K
    {
        if ( generalSettings->useBitrate )
            dataString = i18n( "%1k", QString::number( data/multiplier, 'f', precision ) );
        else
            dataString = i18n( "%1K", QString::number( data/multiplier, 'f', precision ) );
        return dataString;
    }

    if ( data < pow(multiplier, 2)*9.95 ) // < 9.95M
        precision = 1;
    if ( data < pow(multiplier, 2)*999.5 ) // < 999.5M
    {
        dataString = i18n( "%1M", QString::number( data/pow(multiplier, 2), 'f', precision ) );
        return dataString;
    }

    if ( data < pow(multiplier, 3)*9.95 ) // < 9.95G
        precision = 1;
    // xgettext: no-c-format
    dataString = i18n( "%1G", QString::number( data/pow(multiplier, 3), 'f', precision) );
    return dataString;
}

void InterfaceIcon::updateIconText( bool doUpdate )
{
    // Has color changed?
    QColor rxColor = calcColor( KColorScheme(QPalette::Active).foreground(KColorScheme::ActiveText).color() );
    QColor txColor = calcColor( KColorScheme(QPalette::Active).foreground(KColorScheme::NeutralText).color() );
    if ( rxColor != colorIncoming )
    {
        doUpdate = true;
        colorIncoming = rxColor;
    }

    // Has text changed?
    QString byteText = compactTrayText( mInterface->rxRate() );
    if ( byteText != textIncoming )
    {
        doUpdate = true;
        textIncoming = byteText;
    }
    byteText = compactTrayText( mInterface->txRate() );
    if ( byteText != textOutgoing )
    {
        doUpdate = true;
        textOutgoing = byteText;
    }

    if ( !doUpdate )
        return;

    QSize iconSize = getIconSize();
    QPixmap textIcon(iconSize);
    QRect topRect( 0, 0, iconSize.width(), iconSize.height()/2 );
    QRect bottomRect( 0, iconSize.width()/2, iconSize.width(), iconSize.height()/2 );
    textIcon.fill( Qt::transparent );
    QPainter p( &textIcon );
    p.setBrush( Qt::NoBrush );
    p.setOpacity( 1.0 );

    // rxFont and txFont should be the same size per poll period
    QFont rxFont = setIconFont( textIncoming, plasmaTheme->smallestFont(), iconSize.height() );
    QFont txFont = setIconFont( textOutgoing, plasmaTheme->smallestFont(), iconSize.height() );
    if ( rxFont.pointSizeF() > txFont.pointSizeF() )
        rxFont.setPointSizeF( txFont.pointSizeF() );

    p.setFont( rxFont );
    p.setPen( rxColor );
    p.drawText( topRect, Qt::AlignCenter | Qt::AlignRight, textIncoming );

    p.setFont( rxFont );
    p.setPen( txColor );
    p.drawText( bottomRect, Qt::AlignCenter | Qt::AlignRight, textOutgoing );
    mTray->setIconByPixmap( textIcon );
    QPixmapCache::clear();
}

void InterfaceIcon::updateToolTip()
{
    if ( mTray == 0L )
        return;
    inHist.prepend( mInterface->rxRate() );
    outHist.prepend( mInterface->txRate() );
    while ( inHist.count() > histSize )
    {
        inHist.removeLast();
        outHist.removeLast();
    }


    if ( mInterface->settings().iconTheme == TEXT_THEME )
        updateIconText();
    else if ( mInterface->settings().iconTheme == NETLOAD_THEME )
        updateBars();
    mTray->updateToolTip();
}

void InterfaceIcon::updateMenu()
{
    QMenu* menu = mTray->contextMenu();

    InterfaceSettings& settings = mInterface->settings();

    if ( settings.activateStatistics )
        menu->insertAction( configAction, statisticsAction );
    else
        menu->removeAction( statisticsAction );
}

void InterfaceIcon::updateTrayStatus()
{
    const QString ifaceName( mInterface->ifaceName() );
    const BackendData * data = mInterface->backendData();
    int currentStatus = data->status;
    int minVisibleState = mInterface->settings().minVisibleState;

    QString title = mInterface->settings().alias;
    if ( title.isEmpty() )
        title = ifaceName;

    if ( mTray != 0L && currentStatus < minVisibleState )
    {
        delete mTray;
        mTray = 0L;
    }
    else if ( mTray == 0L && currentStatus >= minVisibleState )
    {
        mTray = new InterfaceTray( mInterface, ifaceName );
        QMenu* menu = mTray->contextMenu();

        menu->removeAction( menu->actions().at( 0 ) );
        // FIXME: title for QMenu?
        //menu->addTitle( QIcon::fromTheme( QLatin1String("knemo") ), i18n( "KNemo - %1", title ) );
        menu->addAction( statusAction );
        menu->addAction( plotterAction );
        menu->addAction( configAction );
        KHelpMenu* helpMenu( new KHelpMenu( menu, KAboutData::applicationData(), false ) );
        menu->addMenu( helpMenu->menu() )->setIcon( QIcon::fromTheme( QLatin1String("help-contents") ) );

        if ( mInterface->settings().iconTheme == TEXT_THEME )
            updateIconText();
        else if ( mInterface->settings().iconTheme == NETLOAD_THEME )
            updateBars();
        else
            updateIconImage( mInterface->ifaceState() );
        updateMenu();
    }
    else if ( mTray != 0L )
    {
        if ( mInterface->settings().iconTheme != TEXT_THEME &&
             mInterface->settings().iconTheme != NETLOAD_THEME )
            updateIconImage( mInterface->ifaceState() );
    }
}

void InterfaceIcon::showConfigDialog()
{
    KNemoDaemon::sSelectedInterface = mInterface->ifaceName();

    KProcess process;
    process << QLatin1String("kcmshell5") << QLatin1String("kcm_knemo");
    process.startDetached();
}

void InterfaceIcon::showStatistics()
{
    emit statisticsSelected();
}

void InterfaceIcon::showStatus()
{
    mInterface->showStatusDialog( true );
}

void InterfaceIcon::showGraph()
{
    mInterface->showSignalPlotter( true );
}

#include "moc_interfaceicon.cpp"
