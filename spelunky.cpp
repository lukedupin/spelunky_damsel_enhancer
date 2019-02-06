#include "spelunky.h"

#include <QTextStream>
#include <QRegExp>

Spelunky::Spelunky(const char* pid, const char* boost, QObject *parent) :
    QObject(parent),
    _pid(pid),
    _resetBoost( QString(boost).toInt() )
{
    //Run in additive mode
    if ( _resetBoost < 0 )
    {
        qDebug("Running in cumulative mode");
        _cumulative = true;
        _resetBoost = -_resetBoost;
    }
    _boost = _resetBoost;

    QObject::connect( &_timer, &QTimer::timeout, this, &Spelunky::run );

    _timer.start(1000);
}

//Find the heart address
bool Spelunky::findAddress()
{
    auto&& file = QFile(QString("/proc/%1/maps").arg(_pid));
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ))
    {
        qDebug("Failed to open maps file");
        return false;
    }

    auto&& reg = QRegExp("^([0-9a-fA-F]+)-([0-9a-fA-F]+) rw-p 00000000 00:00 0");

    //Reset my address and range
    _address = 0;

    //Now we need to find the address, little magic coming now
    auto&& mem_filename = QString("/proc/%1/mem").arg(_pid);
    _mem.setFileName(mem_filename);

    //Open up
    if ( !_mem.open(QIODevice::ReadWrite) )
    {
        qDebug("Couldn't open %s", mem_filename.toUtf8().data());
        return false;
    }

    //Read in the lines
    auto&& stream = QTextStream(&file);
    for (  auto line = stream.readLine(); !line.isNull(); line = stream.readLine() )
    {
        qint64 base_addr = 0;
        qint64 base_range = 0;
        qint64 base_size = 0;

        //Did we find our region?
        if ( reg.indexIn(line, 0) != -1 )
        {
            bool addr_ok = false;
            bool range_ok = false;
            base_addr = reg.cap(1).toLong( &addr_ok, 16 );
            base_range = reg.cap(2).toLong( &range_ok, 16 );

            //Did that work?
            if ( !addr_ok || !range_ok )
                continue;

            //Did we find our base address?
            base_size = base_range - base_addr;
            if ( base_addr <= 0 || base_range <= 0 || base_size <= 0)
                continue;
        }
        else
            continue;


        //Seek and look
        //qDebug("Reading: 0x%X at 0x%X", base_size, base_addr);
        _mem.seek(base_addr);
        auto&& buffer = _mem.read( base_size );
        if ( buffer.size() != base_size )
        {
            qDebug("Couldn't read 0x%08llX", base_size);
            continue;
        }

        //Find the memory we are looking for
        int* ptr = reinterpret_cast<int*>(buffer.data());
        for ( qint64 i = 0; i * 4 < base_size; i++ ) //Looking for an int
        {
            //if ( QString().sprintf("%08X", base_addr + i * 4 + 0x8c ).startsWith("00C") )
                //continue;

            //Look for our magic value
            if ( ptr[i] == 0x00B779D0 )
            {
                _address = base_addr + i * 4 + 0x8c;
                qDebug("Found memory lock at 0x%08X", (int)_address );
                //return true;
            }
        }
    }

    return _address != 0;
}

bool Spelunky::lostLink()
{
    qDebug("Lost link");
    _mem.close();
    _address = 0;

    return false;
}

bool Spelunky::followAddress()
{
    if ( !_mem.isOpen() )
        return lostLink();

    //Seek and read
    _mem.seek(_address);
    auto&& buffer = _mem.read(4);
    if ( buffer.size() != 4 )
    {
        _assignDamsel = _capturedDamsel;
        return lostLink();
    }

    //Pull my data
    auto current_heart = *reinterpret_cast<int*>(buffer.data());
    if ( current_heart <= 0 || current_heart >= 100 )
    {
        _assignDamsel = _capturedDamsel;
        return lostLink();
    }

    //If we are "first run"
    if ( _hearts <= 0 )
    {
        _hearts = current_heart;
        qDebug("Found initial heart value of: %d", _hearts);
    }

    //Should we assign a damsel boost? this needs to be done after a link loss
    if ( _assignDamsel )
    {
        _capturedDamsel = _assignDamsel = false;
        _hearts += _boost - 1;
        _mem.seek(_address);
        _mem.write( reinterpret_cast<char*>(&_hearts), sizeof(_hearts) );

        current_heart = _hearts;
        qDebug("Assigning damsel boost of %d.  Hearts: %d", _boost, _hearts );

        //Are we increasing the boost?
        if ( _cumulative )
            _boost++;
    }

    /*
    //Did the user rescue a lady?
    if ( current_heart != _hearts )
        qDebug("%d and %d", current_heart, _hearts );
        */

    //Deal with changes
    if ( current_heart == _hearts + 1 )
    {
        _capturedDamsel = true;
        _hearts = current_heart;
        qDebug("Detected damsel rescue");
    }
    else if ( current_heart > _hearts )
    {
        _hearts = current_heart;
        if ( _cumulative )
            _boost = _resetBoost;
        qDebug("New game?  Setting hearts to current: %d", _hearts );
    }

    else if ( current_heart < _hearts )
    {
        _hearts = current_heart;
        qDebug("Detected hit.  Hearts now: %d", _hearts);
    }

    return true;
}

void Spelunky::run()
{
    if ( _address <= 0 )
    {
        if ( !findAddress() )
            _mem.close();
    }
    else
        followAddress();
}
