#include "ReadWriteInterface.h"


void Uploader::getHash()
{
    CryptoPP::FileSource f(p.string().c_str(), true,
             new CryptoPP::HashFilter(MD5hash,
             new CryptoPP::HexEncoder(new CryptoPP::ArraySink(buffer,2 * CryptoPP::MD5::DIGESTSIZE))));

             digest = string((const char*)buffer,2 * CryptoPP::MD5::DIGESTSIZE);
}

bool Uploader::uploadList(list<string> List, string collection)
{
    if(List.empty())
    {
        cout << "Empty list!" << endl;
        return 0;
    }

    missedFiles = 0;

    mongo::GridFS fs = mongo::GridFS(conn,dbName.c_str(),collection.c_str());
    files = fs.list();

    while(files.get()->more()){
                    mongo::GridFile file = fs.findFile(files.get()->next());
                    hashlist.push_front(boost::algorithm::to_upper_copy(file.getMD5()));
             }

    for(list<string>::iterator it = List.begin(); it != List.end(); ++it)
    {
        p = *it;

        if(boost::filesystem::exists(p))
        {
            getHash();

             hashIter = find(hashlist.begin(),hashlist.end(),digest);

        if(hashIter == hashlist.end()){
            ifstream counter(*it);
            counter.seekg(0,ios::end);
            masterList.push_back(make_pair(*it,counter.tellg()));
            total += counter.tellg();
             }
             else
                ++missedFiles;
        }

    }

    hashlist.clear();

    meter.reset(total,1,1);

    masterList.sort(low_to_high);

    for(auto it:masterList){

    if(boost::filesystem::exists(p)){

        fs.storeFile(it.first,it.first);
        meter.hit(it.second);
    }
        else
            ++missedFiles;

    }

    if(missedFiles >= 1){

        return 1;
    }
    meter.finished();

    total = 0;

    return 1;
}

bool Uploader::upload(string filename, string collection)
{
    mongo::GridFS fs = mongo::GridFS(conn,dbName.c_str(),collection.c_str());

    p = filename;

    files = fs.list();

    while(files.get()->more()){
                    mongo::GridFile file = fs.findFile(files.get()->next());
                    hashlist.push_front(boost::algorithm::to_upper_copy(file.getMD5()));
             }

             getHash();

             cout << digest << endl;

             hashIter = find(hashlist.begin(),hashlist.end(),digest);


    if(!boost::filesystem::exists(p))
    {
        cout << "No such file!" << endl;
        return false;
    }

    if(hashIter == hashlist.end())
    fs.storeFile(filename.c_str(),filename.c_str());
    return 1;
}

bool Downloader::getFiles(string collection, mongo::BSONObj Regex)
{

    if(!filelist.empty())
        filelist.clear();

    mongo::GridFS fs = mongo::GridFS(conn,dbName.c_str(),collection.c_str());
    files = fs.list(Regex);

    while(files.get()->more()){
    mongo::GridFile file = fs.findFile(files.get()->next().getStringField("filename"));
    total += file.getContentLength();
    filelist.push_back(file);
    }

    filelist.sort(low_to_high);

    meter.reset(total,1,1);

    while(!filelist.empty()){

    out.open(filelist.front().getFilename());
    cout << filelist.front().getFilename() << endl;

    cout << filelist.front().getContentLength() << endl;
    filelist.front().write(out);

    out.close();

    cout << meter.hit(filelist.front().getContentLength()) << endl;
    filelist.pop_front();

    }
    meter.finished();

            return 1;
        }

list<mongo::GridFile> Downloader::getFileList(string collection)
{
            if(!filelist.empty())
                filelist.clear();

            mongo::GridFS fs = mongo::GridFS(conn,dbName.c_str(),collection.c_str());
            files = fs.list();

            while(files.get()->more()){
            mongo::GridFile file = fs.findFile(files.get()->next().getStringField("filename"));
            filelist.push_back(file);
            }

            return filelist;
        }

list<mongo::GridFile> Downloader::getFileList(string collection, mongo::BSONObj Regex)
{
            if(!filelist.empty())
                filelist.clear();

            mongo::GridFS fs = mongo::GridFS(conn,dbName.c_str(),collection.c_str());
            files = fs.list(Regex);

            while(files.get()->more()){
            mongo::GridFile file = fs.findFile(files.get()->next().getStringField("filename"));
            filelist.push_back(file);
            }

            return filelist;
        }

bool mongo::ProgressMeter::hit(int n){
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

void mongo::ProgressMeter::reset( unsigned long long total , int secondsBetween, int checkInterval) {
     _total = total;
           _secondsBetween = secondsBetween;
           _checkInterval = checkInterval;

           _done = 0;
           _hits = 0;
           _lastTime = (int)time(0);

             _active = 1;
}
