#include <iostream>
#include <fstream>
#include <list>
#include "files.h"
#include "md5.h"
#include "hex.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>
#include <mongo/util/progress_meter.h>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>
#include <mongo/bson/bsonobj.h>
#include <mongo/client/dbclientcursor.h>

using namespace std;

class ReadWriteInterface
{
public:
    bool connect(string db, mongo::HostAndPort server)
            {
    dbName = db;

    conn.connect(server, err);

    //conn.auth(BSON("mechanism" << "$external" << "userSource" << "projects1" << "pwd" << "panthermodern" << "user" << "DixieFlatline"));

    ok = conn.auth(dbName.c_str(),"DixieFlatline","panthermodern",err);
    if ( ! ok )
        cout << "DIDN'T WORK" << endl;
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
        mongo::ProgressMeter meter;
        unsigned long long total = 0;
        unique_ptr<mongo::DBClientCursor> files;
};

class Uploader : public ReadWriteInterface
{
    public: Uploader(){};
            bool upload(string filename, string collection);
            bool uploadList(list<string> List, string collection);

    private:

        boost::filesystem::path p;
        unsigned int missedFiles;
        list<pair<string,int>> masterList;
        list<string> hashlist;
        list<string>::iterator hashIter;
        CryptoPP::MD5 MD5hash;
        byte buffer[2 * CryptoPP::MD5::DIGESTSIZE];
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