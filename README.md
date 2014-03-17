GridFS-Interface
================

A small wrapper for uploading and downloading a file or multiple files using GridFS.

Dependancies
============

+ libmongoclient
+ boost_filesystem
+ boost_thread
+ cryptopp

Example
=======

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
    if( boost::regex_match( i->path().filename().string(), my_filter ) )// File matches, store it
    all_matching_files.push_back( i->path().filename().string() );
}

//All files that were added to the list will be uploaded to a collection named "Warmachine"
    put.uploadList(all_matching_files,"Warmachine");

//terminate the connection
    put.logout();
```
