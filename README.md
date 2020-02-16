# Use Google query Language for various SQL database

Google Charts (https://developers.google.com/chart/) use their own query
language (GQL) and expect json output to allow for a variety of graphs.

This library GQL queries against SQL databases to make it easy to use data in
those databases for charts. Currently supported are MySQL/Mariadb and
PostgreSQL. The system is extensible so more adapters can be written.

The google references:
- https://developers.google.com/chart/
- https://developers.google.com/chart/interactive/docs/querylanguage
- https://developers.google.com/chart/interactive/docs/dev/implementing_data_source
- https://developers.google.com/chart/interactive/docs/reference

[TOC] 

# Installation


## Pre-requisite

Note that all install commands must be run as root or be prefixed with "sudo"

### Debian / Ubuntu based systems

The following packages are required on a Debian like system:

- libjsoncpp
- libmysql++
- libpqxx
- libboost1.62
- libicu
- libgflags
- libgoogle-glog

they can be installed as root using

    apt-get install libjsoncpp-dev libmysql++-dev libpqxx-dev libboost1.62-dev libicu-dev libgflags-dev libgoogle-glog-dev

To create the documentation install 

    apt-get install doxygen graphviz

In order to run the tests you need googletest:
    
    apt-get install googletest

you will have to compile gtest partly yourself using (see
/usr/share/doc/googletest/README.Debian for more details):

Depending on the distribution you are using the files are either in
/usr/src/gtest or in /usr/src/googletest/googletest. cd to the correct
directory and execute

    cmake CMakeLists.txt && make && cp *.a /usr/lib


### Redhat/Centos (rpm based)

An rpm based distro (Centos) requires access to the epel repository, if you
have not yet enabled it do so with

    yum --enablerepo=extras install epel-release

The libraries required are

- boost
- jsonpp
- mysql++
- libpqxx
- libicu
- glog
- gflags

you can install them as root using

    yum install boost-devel jsonpp-devel mysql++-devel libpqxx-devel libicu-devel glog-devel gflags-devel

In order to run the tests you will also need

    yum install gtest-devel

To create the documentation doxygen is needed:

    yum install doxygen graphviz



## Compile/Install


You will need either g++ or clang to compile this package

Once you have downloaded the code create a build directory and run configure
from there. Say you create the directory parallel to the libgqlsql directory
you can use the following commands to compile it:

    mkdir bld
    cd bld
    ../libgqlsql/configure
    make

In order to use clang if you have g++ and clang++ installed use this configure
command line instead:

    CC=clang CXX=clang++ ../libgqlsql/configure

To install the code run

    sudo make install

## Running the tests

The tests require a running mysql and postgresql server on the local host.
For each DB load the corresponding sql file in tests:

- MySQL:      mysql_gqltest.sql
- PostgreSQL: pg_gqltest.sql

Then create a user called 'gqltest' with password 'gqltest' that has read-only access
to those databases. Once ready the tests should all pass.

# Using the connector

In order to use google charts the data needs to be served via a web server.
If you have a web server that can execute cgi scripts you can use it, otherwise
you can start your own using a few lines of python as explained below.

However, first you need to create a user that has access to the database
you want to server, and make sure that it only has read access to those
tables you are willing to share

Warning: there is no easy way to secure access to the GQL connector, and anyone
that knows the URL can read out all the data that is made accessible. Unless this
is OK or the web server is confined to a local network make sure to NOT allow access
to any sensitive information! You have been warned!


## Preparing mysql/mariadb

(mysql and mariadb use the same syntax and work the same way)

Suppose there is a database called 'MyData' and GQL should have access to the
two tables 'Numbers' and 'Info'. The user created will be called 'GQL user'
using the password 'mysecretpw'. Further assume that the mysql server runs on the
same server as the web server. The following commands typed in at the mysql prompt
will create this user:

    $ mysql -u root -p

Type in the root password to access the mysql server if prompted.

    mysql> CREATE USER 'gqluser'@'localhost' IDENTIFIED BY 'mysecretpw';
    mysql> GRANT SELECT ON MyData.Numbers TO 'gqluser'@'localhost';
    mysql> GRANT SELECT ON MyData.Info TO 'gqluser'@'localhost';
    mysql> FLUSH PRIVILEGES;

You can check that this worked using this command

    $ gqldb -d Info -x 'select * LIMIT 10' 'mysql://gqluser:mysecretpw@localhost/MyData'

This should return a json string with the requested 10 lines of the database. 
Should an error be returned instead check that the access rights all work,
the mysql server is running, etc.

Using the command line above on a shared server is somewhat risky as others
can see the command line and in particular the password. Instead you can use
a configuration file in json format, name it gql.conf:

    {
        "type":"mysql",
        "user":"gqluser",
        "password":"mysecretpw",
        "port": 3306,
        "db":"MyData",
        "server":"localhost",
        "default":"Info",
        "tables":[]
    }

And use it as follows

    $ gqldb -c gql.conf -x 'select * LIMIT 10'

## Preparing PostgreSQL

Suppose there is a database called 'MyData' and GQL should have access to the
two tables 'Numbers' and 'Info'. The user created will be called 'gqluser'
using the password 'mysecretpw'. Further assume that the mysql server runs on the
same server as the web server. The following commands can be used to create the
user and grant access;

    $ psql postgres
    postgres=# CREATE ROLE gqluser WITH LOGIN PASSWORD 'mysecretpw'
    postgres-# NOSUPERUSER INHERIT NOCREATEDB NOCREATEROLE NOREPLICATION VALID UNTIL 'infinity';
    postgres-# \q
    $ psql MyData
    MyData=# GRANT CONNECT ON DATABASE gqltest to gqluser;
    MyData=# GRANT USAGE ON "Info" TO gqluser;
    MyData=# \q

Now this command should return some data:

    $ gqldb -d T1 -x 'select * LIMIT 10' 'postgresql://gqluser:mysecretpw!!@localhost/gqltest'

As with mysql/mariadb a configuration file works too:

    {
        "type":"postgresql",
        "user":"gqluser",
        "password":"mysecretpw",
        "db":"MyData",
        "server":"localhost",
        "default":"Info",
        "tables":[]
    }

And use it as follows

    $ gqldb -c gql.conf -x 'select * LIMIT 10'


## Setup cgi

In order to use this program it needs a web server to handle the communication.
Most web servers can be used that can execute so called 'cgi' scripts.

In most cases a configuration file like the ones shown above, with one additional
line at the beginning and the execute bit set is all that is needed (be sure to
use the correct path for the gqldb binary). The name should end in .cgi, say gql.cgi
and is usually in a directory called cgi-bin (this depends on the configuration
of the web server used).

    #!/usr/bin/gqldb
    {
        "type":"postgresql",
        "user":"gqluser",
        "password":"mysecretpw",
        "db":"MyData",
        "server":"localhost",
        "default":"Info",
        "tables":[]
    }

For a quick test try to execute this, you should get a one line error:

    $ ./gql.cgi
    CGI query not found (QUERY_STRING env variable not set)

If this works try to connect via the browser, appending '?tq=select+*' to the
url. If you get the configuration file instead of a json string the web server
is not configured correctly, please check the manual on how to fix this.

Note that if the above does not work a simple bash script or python script that
calls the gqldb executable works too, for example

    #!/bin/bash
    exec /usr/bin/gqldb -d alltypes mysql://gqltest:gqltest@localhost/gqltest

If there is no web server available most python installations come with a simple
web server that can be used. It needs to be started in the directory that contains
cgi-bin, in this example listening on port 8000:

    python -u -m CGIHTTPServer 8000

If you configured postgresql for testing and copy the test html page
test/chart.html to the directory in which the python web server runs you should
be able to get a nice little chart by opening the page

    http://ip-address-of-server:8000/chart.html

if you see a little pie-chart with three elements everything worked and you
are good to go!


# Extensions

The library supports two extensions: 

- It allows additional tables to be queried, using the syntax
  `tablename.columnname` as long as all those additional tables are listed when
  calling the parser (option --table when using gqldb, or added in
  GQL_SQL::DBQuery::DB::tableAdd() )

- By enabling extended function any function not recognized as a GQL function
  is passed unchecked through the SQL interface. While this increases the
  flexibility of the interface it may potentially allow access to functions
  that are not intended to be accessible through this interface, so use with
  caution.

# Limitations

- No clear definition of boolean in SQL, so boolean values are returned as
  number type unless a boolean format is used (a boolean format is defined as
  one that fails to parse as a number format and that contains a single ';')

- toDate() works only for date strings, it will not convert a timestamp to a
  date

- At this time it is not possible to compile only one of the backends
  supported.

- All data is read in memory, and the complete json tree is created before it
  is sent out. As a result the memory needed can be quite large. A streaming
  model at least for queries that do not use  the 'pivot' clause would be
  preferable.

- The data is currently not compressed when sent back, even if the browser
  indicates support for it.

- the 'version' field is always ignored, so that every request will send the
  data back even if it is the same query that would result in the same data as
  sent before.

- unsigned 64bit numbers that have the highest bit set will be converted to a
  64bit float before the pattern is applied (if there is any) and as a result
  some low bits will be lost. This is a restriction of the icu library that
  does not support 64bit unsigned integers.

