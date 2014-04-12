#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/noncopyable.hpp>
#include <list>
#include "files.h"
#include "md5.h"
#include "hex.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/client/dbclientcursor.h>

using namespace std;

class ProgressMeter : boost::noncopyable {
    public:
        ProgressMeter(unsigned long long total,
                      int secondsBetween = 3,
                      int checkInterval = 100,
                      string units = "",
                      string name = "Progress")
                : _showTotal(true),
                  _units(units) {
            _name = name.c_str();
            reset( total , secondsBetween , checkInterval );
        }

        ProgressMeter() : _active(0), _showTotal(true), _units("") {
            _name = "Progress";
        }

        // typically you do ProgressMeterHolder
        void reset( unsigned long long total , int secondsBetween = 3 , int checkInterval = 100 );

        void finished() { _active = 0; }
        bool isActive() const { return _active; }


        bool hit( int n = 1 );

        void setUnits( const string& units ) { _units = units; }
        string getUnit() const { return _units; }

        void setName(string name) { _name = name.c_str(); }
        string getName() { return _name; }

        void setTotalWhileRunning( unsigned long long total ) {
            _total = total;
        }

        unsigned long long done() const { return _done; }

        unsigned long long hits() const { return _hits; }

        unsigned long long total() const { return _total; }

        void showTotal(bool doShow) {
            _showTotal = doShow;
        }

        string toString();

        bool operator==( const ProgressMeter& other ) const { return this == &other; }

    private:

        bool _active;

        unsigned long long _total;
        bool _showTotal;
        int _secondsBetween;
        int _checkInterval;

        unsigned long long _done;
        unsigned long long _hits;
        int _lastTime;

        string _units;
        string _name;
    };

    class ProgressMeterHolder : boost::noncopyable {
    public:
        ProgressMeterHolder( ProgressMeter& pm )
            : _pm( pm ) {
        }

        ~ProgressMeterHolder() {
            _pm.finished();
        }

        ProgressMeter* operator->() { return &_pm; }

        ProgressMeter* get() { return &_pm; }

        bool hit( int n = 1 ) { return _pm.hit( n ); }

        void finished() { _pm.finished(); }

        bool operator==( const ProgressMeter& other ) { return _pm == other; }

    private:
        ProgressMeter& _pm;
    };

class ReadWriteInterface
{
public:
    bool connect(string db, mongo::HostAndPort server)
            {
    dbName = db;

    conn.connect(server, err);

    ok = conn.auth(dbName.c_str(),"Username","password",err);
    if ( ! ok ){
        cout << "DIDN'T WORK" << endl;
        return false;
    }
    MONGO_verify( ok );

    return 1;
            }
    bool logout()
        {
            conn.logout(dbName,obj);
        return 1;
        }
    bson::bo logoutResult() {return obj;}
    protected:
        string err, dbName;
        bson::bo obj;
        bool ok;
        mongo::DBClientConnection conn;
        ProgressMeter meter;
        unsigned long long total = 0;
        unique_ptr<mongo::DBClientCursor> files;
};

class Uploader : public ReadWriteInterface
{
    public: Uploader(){};
            bool upload(string filename, string collection);
            bool uploadList(list<string> List, string collection);
            unsigned int missedUploads(){return missedFiles;}

    private:

        boost::filesystem::path p;
        unsigned int missedFiles;
        list<pair<string,int>> masterList, missedList;
        list<string> hashlist;
        list<string>::iterator hashIter;
        CryptoPP::Weak::MD5 MD5hash;
        byte buffer[2 * CryptoPP::Weak::MD5::DIGESTSIZE];
        string digest;
        void getHash();
        static bool low_to_high(pair<string,int> herp, pair<string,int> derp)
{
    return herp.second < derp.second;
}

};

class Downloader : public ReadWriteInterface {
    public: Downloader(){};

        bool getFiles(string collection, mongo::BSONObj Regex);

        list<mongo::GridFile> getFileList(string collection);

        list<mongo::GridFile> getFileList(string collection, mongo::BSONObj Regex);

    private:
        ofstream out;
        list<mongo::GridFile> filelist;
        static bool low_to_high(mongo::GridFile file1, mongo::GridFile file2)
    {
        return file1.getContentLength() < file2.getContentLength();
    }

};

class Deleter : public ReadWriteInterface{
	public: Deleter(){};

		bool remove(mongo::BSONObj query, string collection);
		bool remove(string filename, string collection){mongo::GridFS(conn,dbName,collection).removeFile(filename); return true;}
};
