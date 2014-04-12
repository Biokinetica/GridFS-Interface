#include "ReadWriteInterface.h"

void ProgressMeter::reset( unsigned long long total , int secondsBetween , int checkInterval) {
        _total = total;
        _secondsBetween = secondsBetween;
        _checkInterval = checkInterval;

        _done = 0;
        _hits = 0;
        _lastTime = (int)time(0);

        _active = 1;
    }


    bool ProgressMeter::hit( int n ) {
           if ( ! _active ) {
               cout << "warning: hit on in-active ProgressMeter" << endl;
                return false;
            }

          _done += n;
           _hits++;
           if ( _hits % _checkInterval )
              return false;

         int t = (int) time(0);
           if ( t - _lastTime < _secondsBetween )
              return false;

          if ( _total > 0 ) {
            int per = (int)( ( (double)_done * 100.0 ) / (double)_total );

            cout << "\t\t" << _name << ": " << _done << '/' << _total << '\t' << per << '%' << endl;
        }
             _lastTime = t;
            return true;
        }

    string ProgressMeter::toString() {
        if ( ! _active )
            return "";
        stringstream buf;
        buf << _name << ": " << _done << '/' << _total << ' ' << (_done*100)/_total << '%';

        if ( ! _units.empty() ) {
            buf << "\t(" << _units << ")" << endl;
        }

        return buf.str();
    }
