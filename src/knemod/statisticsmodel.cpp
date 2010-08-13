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

#include "statisticsmodel.h"
#include "global.h"
#include <QStringList>
#include <KLocale>
#include <kio/global.h>

StatisticsModel::StatisticsModel( enum KNemoStats::PeriodUnits t, QObject *parent ) :
    QStandardItemModel( parent ),
    mPeriodType( t ),
    mCalendar( 0 )
{
    QStringList headerList;
    headerList << i18n( "Date" ) << i18n( "Sent" ) << i18n( "Received" ) << i18n( "Total" );
    setHorizontalHeaderLabels( headerList );
    setSortRole( DataRole );
}

StatisticsModel::~StatisticsModel()
{
}

void StatisticsModel::addBytes( enum StatsColumn column, quint64 bytes, int row )
{
    if ( !bytes || !rowCount() )
        return;
    if ( row < 0 )
        row = rowCount() - 1;

    quint64 b = item( row, column )->data( DataRole ).toULongLong() + bytes;
    item( row, column )->setData( b, DataRole );
    updateText( item( row, column ) );
}

quint64 StatisticsModel::bytes( enum StatsColumn column, int row ) const
{
    if ( row < 0 )
        row = rowCount() - 1;

    if ( rowCount() && rowCount() > row )
        return item( row, column )->data( DataRole ).toULongLong();
    else
        return 0;
}

QString StatisticsModel::text( StatsColumn column, int row ) const
{
    if ( row < 0 )
        row = rowCount() - 1;

    if ( rowCount() && rowCount() > row )
        return item( row, column )->data( Qt::DisplayRole ).toString();
    else
        return QString();
}

void StatisticsModel::updateText( QStandardItem * i )
{
    quint64 all = i->data( DataRole ).toULongLong();
    i->setData( KIO::convertSize( all ), Qt::DisplayRole );
}

int StatisticsModel::createEntry()
{
    QList<QStandardItem*> entry;
    QStandardItem * dateItem = new QStandardItem();
    QStandardItem * txItem = new QStandardItem();
    QStandardItem * rxItem = new QStandardItem();
    QStandardItem * totalItem = new QStandardItem();
    //dateItem->setData( rowCount(), IdRole );
    entry << dateItem << txItem << rxItem << totalItem;
    appendRow( entry );
    setTraffic( rowCount() - 1, 0, 0 );
    return ( rowCount() - 1 );
}

void StatisticsModel::setDateTime( QDateTime dateTime )
{
    if ( !rowCount() )
        return;
    item( rowCount() - 1, Date )->setData( dateTime, DataRole );
    updateDateText( rowCount() - 1 );
}

void StatisticsModel::setDays( int days )
{
    if ( !rowCount() )
        return;
    item( rowCount() - 1, Date )->setData( days, SpanRole );
}

void StatisticsModel::updateDateText( int row )
{
    if ( row < 0 || !rowCount() )
        return;

    QString dateStr;
    QDateTime dt = dateTime( row );
    int dy = days( row );
    switch ( mPeriodType )
    {
        case KNemoStats::Hour:
            dateStr = KGlobal::locale()->formatTime( dt.time() );
            dateStr += " " + mCalendar->formatDate( dt.date(), KLocale::FancyShortDate );
            break;
        case KNemoStats::Month:
            // Format for simple period
            // Starts on the first of the month, lasts exactly one month
            if ( mCalendar->day( dt.date() ) == 1 &&
                 dy == mCalendar->daysInMonth( dt.date() ) )
                dateStr = QString( "%1 %2" )
                            .arg( mCalendar->monthName( dt.date(), KCalendarSystem::ShortName ) )
                            .arg( mCalendar->year( dt.date() ) );
            // Format for complex period
            else
            {
                QDate endDate = dt.date().addDays( dy - 1 );
                dateStr = QString( "%1 %2 - %4 %5 %6" )
                            .arg( mCalendar->day( dt.date() ) )
                            .arg( mCalendar->monthName( dt.date(), KCalendarSystem::ShortName ) )
                            .arg( mCalendar->day( endDate ) )
                            .arg( mCalendar->monthName( endDate, KCalendarSystem::ShortName ) )
                            .arg( mCalendar->year( endDate ) );
            }
            break;
        case KNemoStats::Year:
            dateStr = QString::number( mCalendar->year( dt.date() ) );
            break;
        default:
            dateStr = mCalendar->formatDate( dt.date(), KLocale::ShortDate );
    }
    item( row, 0 )->setData( dateStr, Qt::DisplayRole );
}

QDateTime StatisticsModel::dateTime( int row ) const
{
    if ( row < 0 )
        row = rowCount() - 1;

    if ( rowCount() && rowCount() > row )
        return item( row, Date )->data( DataRole ).toDateTime();
    else
        return QDateTime();
}

QDate StatisticsModel::date( int row ) const
{
    return dateTime( row ).date();
}

int StatisticsModel::days( int row ) const
{
    if ( row < 0 )
        row = rowCount() - 1;

    if ( rowCount() && rowCount() > row )
    {
        return item( row, Date )->data( SpanRole ).toInt();
    }
    else
        return 0;
}

quint64 StatisticsModel::rxBytes( int row ) const
{
    return bytes( RxBytes, row );
}

quint64 StatisticsModel::txBytes( int row ) const
{
    return bytes( TxBytes, row );
}

quint64 StatisticsModel::totalBytes( int row ) const
{
    return bytes( TotalBytes, row );
}

QString StatisticsModel::txText( int row ) const
{
    return text( TxBytes, row );
}

QString StatisticsModel::rxText( int row ) const
{
    return text( RxBytes, row );
}

QString StatisticsModel::totalText( int row ) const
{
    return text( TotalBytes, row );
}

void StatisticsModel::addRxBytes( quint64 bytes, int row )
{
    addBytes( RxBytes, bytes, row );
    addBytes( TotalBytes, bytes, row );
}

void StatisticsModel::addTxBytes( quint64 bytes, int row )
{
    addBytes( TxBytes, bytes, row );
    addBytes( TotalBytes, bytes, row );
}

void StatisticsModel::setTraffic( int i, quint64 rx, quint64 tx )
{
    if ( i < 0 || i >= rowCount() )
        return;

    item( i, RxBytes )->setData( rx, DataRole );
    item( i, TxBytes )->setData( tx, DataRole );
    item( i, TotalBytes )->setData( rx+tx, DataRole );
    updateText( item( i, RxBytes ) );
    updateText( item( i, TxBytes ) );
    updateText( item( i, TotalBytes ) );
}

#include "statisticsmodel.moc"
