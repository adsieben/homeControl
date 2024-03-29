#!/bin/bash

ARCHIVE_DB_FILENAME="data/values.20240127.sqlite"
#https://stackoverflow.com/a/30518622
#Pathname Expansion — After word splitting, *unless the -f option has been set,
#bash scans each word for the characters *, ?, and [. If one of these characters 
#appears, then the word is regarded as a pattern, and replaced with an 
#alphabetically sorted list of file names matching the pattern.

#https://stackoverflow.com/a/27391558
#for making sqlite quiet 

sqlite3 $ARCHIVE_DB_FILENAME << \EOF
.output /dev/null
PRAGMA busy_timeout = 20000;
.output stdout
ATTACH DATABASE "data/values.sqlite" AS srcDB;
insert into tPvSec select * from srcDB.tPvSec where fdatetime > (select ifnull( max( fdatetime ), 0 ) from tPvSec );
insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select ifnull( max( fdatetime ), 0 ) from tPvTenSec );
delete  from srcDB.tpvsec where srcdb.tpvsec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
delete  from srcDB.tpvTensec where srcdb.tpvTensec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
EOF


#turn of bash expansion of "*"
# set -f
# SQL=$(cat << \EOF
# ATTACH DATABASE "values.sqlite" AS srcDB;
# insert into tPvSec select * from srcDB.tPvSec where fdatetime > (select max( fdatetime ) from tPvSec );
# insert into tPvTenSec select * from srcDB.tPvTenSec where fdatetime > (select max( fdatetime ) from tPvTenSec );
# delete  from srcDB.tpvsec where srcdb.tpvsec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
# delete  from srcDB.tpvTensec where srcdb.tpvTensec.fdatetime < (select CAST(strftime('%s', 'now', '-24 hour') as INTEGER) as time) ;
# EOF
# )

#no pathname expansion when variable in "" 
# echo "$SQL"

# select min(fDateTime) as time, count( fDateTime ) as cnt from srcDB.tpvsec tp group by fDateTime/10 having fdatetime > (select CAST(strftime('%s', 'now', '-24 minute') as INTEGER) as time) order by cnt desc;

#no pathname expansion when variable in with set -f 
# set -f
# echo $SQL
# set +f
# echo $SQL


