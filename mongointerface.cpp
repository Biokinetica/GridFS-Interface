#include "ReadWriteInterface.h"


void Uploader::getHash()
{
    CryptoPP::FileSource f(p.string().c_str(), true,
             new CryptoPP::HashFilter(MD5hash,
             new CryptoPP::HexEncoder(new CryptoPP::ArraySink(buffer,2 * CryptoPP::Weak::MD5::DIGESTSIZE))));

             digest = string((const char*)buffer,2 * CryptoPP::Weak::MD5::DIGESTSIZE);
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
                    hashlist.emplace_front(boost::algorithm::to_upper_copy(file.getMD5()));
             }

    for(list<string>::iterator it = List.begin(); it != List.end(); ++it)
    {
        p = *it;
        ifstream counter(*it);
        counter.seekg(0,ios::end);

        if(boost::filesystem::exists(p))
        {
            getHash();

             hashIter = find(hashlist.begin(),hashlist.end(),digest);


        if(hashIter == hashlist.end()){

            masterList.emplace_back(*it,counter.tellg());
            total += counter.tellg();
             }
             else{
            missedList.emplace_back(*it,counter.tellg());
            ++missedFiles;
            }
        }

    }

    hashlist.clear();

    meter.reset(total,1,1);

    masterList.sort(low_to_high);

    if(!masterList.empty())
    for(auto it:masterList){

    p = it.first;

    if(boost::filesystem::exists(p)){

        fs.storeFile(it.first,p.filename().string());
        meter.hit(it.second);
    }
        else
        {

        missedList.push_back(it);
            ++missedFiles;
            }

    }
    else
    {
    cout << "All matching files are already in database." << endl;
    cout << "Files were not uploaded: " << endl;

    for(auto it:missedList)
    cout << it.first << ": " << it.second << " bytes" << endl;

    return true;
    }

    if(missedFiles >= 1){

    cout << "Some files were not uploaded:" << endl;

    for(auto it:missedList)
    cout << it.first << ": " << it.second << " bytes" << endl;

        return 0;
    }
    meter.finished();

    masterList.clear();

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

bool Deleter::remove(mongo::BSONObj query, string collection)
{
    mongo::GridFS fs = mongo::GridFS(conn,dbName,collection);
    fs.removeFile(fs.findFile(query).getFilename());
    return true;
}
