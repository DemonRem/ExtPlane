#include "flightgeardatasource.h"
#include <QDebug>
#include <QThread>
#include "../extplane-server/datarefs/floatdataref.h"

FlightGearDataSource::FlightGearDataSource() : DataSource()
{
    connect(&tcpClient, &BasicTcpClient::tcpClientConnected,
            this, &FlightGearDataSource::sessionOpened);
    connect(&tcpClient, &BasicTcpClient::receivedLine,
            this, &FlightGearDataSource::readLine);
    connect(&tcpClient, &BasicTcpClient::networkError,
            this, &FlightGearDataSource::gotNetworkError);

    refMap.insert("sim/cockpit2/gauges/indicators/heading_electric_deg_mag_pilot", "/instrumentation/magnetic-compass/indicated-heading-deg");
    refMap.insert("sim/cockpit2/gauges/indicators/airspeed_kts_pilot", "/instrumentation/airspeed-indicator/indicated-speed-kt");
    refMap.insert("sim/cockpit2/gauges/indicators/pitch_vacuum_deg_pilot", "/instrumentation/attitude-indicator/indicated-pitch-deg");
    refMap.insert("sim/cockpit2/gauges/indicators/roll_vacuum_deg_pilot", "/instrumentation/attitude-indicator/indicated-roll-deg");
    refMap.insert("sim/flightmodel/misc/h_ind", "/instrumentation/altimeter/indicated-altitude-ft");
    refMap.insert("sim/cockpit2/gauges/actuators/barometer_setting_in_hg_pilot", "/instrumentation/altimeter/setting-inhg");
    refMap.insert("sim/flightmodel/position/vh_ind", "/instrumentation/vertical-speed-indicator/indicated-speed-fpm");
    refMap.insert("sim/cockpit2/gauges/indicators/slip_deg", "/instrumentation/slip-skid-ball/indicated-slip-skid");

    tcpClient.setLineEnding("\r\n");
}

void FlightGearDataSource::connectToSource(QString host, int port)
{
    // @todo continue here..
    tcpClient.connectToHost("localhost", 5401);
}

DataRef *FlightGearDataSource::subscribeRef(QString name)
{
    qDebug() << Q_FUNC_INFO << name;
    if(refMap.contains(name)) {
        QString fgRef = refMap.value(name);
        FloatDataRef *newRef = new FloatDataRef(this, name, 0);
        floatRefs.append(newRef);
        tcpClient.writeLine("subscribe " + fgRef);
        return newRef;
    }
    return 0;
}

void FlightGearDataSource::unsubscribeRef(DataRef *ref)
{
    qDebug() << Q_FUNC_INFO << ref->name();
    tcpClient.writeLine("unsubscribe " + refMap.value(ref->name()));
    refMap.remove(ref->name());
    ref->deleteLater();
}

void FlightGearDataSource::updateDataRef(DataRef *ref)
{
}

void FlightGearDataSource::changeDataRef(DataRef *ref)
{
}

void FlightGearDataSource::keyStroke(int keyid)
{
}

void FlightGearDataSource::buttonPress(int buttonid)
{
}

void FlightGearDataSource::buttonRelease(int buttonid)
{
}

void FlightGearDataSource::command(QString &name, extplaneCommandType type)
{
}

void FlightGearDataSource::sessionOpened()
{
    tcpClient.writeLine("data");
}

void FlightGearDataSource::readLine(QString line)
{
    if(line.contains("=")) {
        QStringList splitted = line.split('=');
        QString fgRef = splitted[0];
        QString valueStr = splitted[1];
        foreach (FloatDataRef *ref, floatRefs) {
            if(refMap.value(ref->name()) == fgRef) {
                ref->updateValue(valueStr.toFloat());
            }
        }
    } else {
        qDebug() << Q_FUNC_INFO << "Unknown:" << line;
    }

}

void FlightGearDataSource::gotNetworkError(QString errstring)
{
    emit sourceNetworkError(errstring);
}