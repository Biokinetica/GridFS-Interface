GridFS-Interface
================

A small wrapper for uploading and downloading a file or multiple files using GridFS.

Dependancies
============

+ libmongoclient (required for any mongodb communication)
+ boost_filesystem
+ boost_thread
+ cryptopp

Features
========

One of the most useful features in this wrapper is its ability to first compare the files listed for upload with files that are already in the database. This is achieved using cryptopp, a c++ encryption library. Whenever a file is uploaded, its md5 hash is taken and stored inside the document in the .files collection. This wrapper will first download the list of files from the database, then take the md5 hash of every file to be uploaded. It will then compare each locally computed hash against the list of hashes downloaded to decide if they need to be uploaded in the first place. No duplicate files!

Examples
========
The following is an example of how one might upload a list of files using this interface
```c++
mongo::HostAndPort server("server","port");

    Uploader put;

    put.connect("projects1",server);

    string target_path;

    if(argv[1] == NULL)
    target_path = boost::filesystem::current_path().string();
    else
        target_path = argv[1];

//Our regular expression will match any pdf that starts with the word "forces"

    const boost::regex my_filter( "(?i)forces.*\\.pdf" );

list<string> all_matching_files;

boost::filesystem::directory_iterator end_itr;
for( boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i )
{
    // Skip if no match
    if( boost::regex_match( i->path().filename().string(), my_filter ) )
    all_matching_files.push_back( i->path().filename().string() );
}

//All files added to the list will be uploaded to a collection named "Warmachine"
    put.uploadList(all_matching_files,"Warmachine");

//terminate the connection
    put.logout();
```
Similarly, we can download files with just as much ease

```c++
Downloader getter;

getter.connect("projects1",server);

string expr;

expr = ".*";

expr.append("forces").append(".*.pdf");

mongo::BSONObj reg = mongo::BSONObjBuilder().appendRegex("filename",expr,"i").obj();

getter.getFiles("Warmachine",reg);

getter.logout();
```
