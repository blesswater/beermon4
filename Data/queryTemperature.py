#!/usr/bin/python

import sys
import time
from datetime import datetime, timedelta

from beerChipDB import beerChipSQLiteDB as beerDB

def usage():
    print( '' )
    print( 'Usage:')
    print( '    python queryTemperature.py "DatasetName" ')
    print( '' )

if __name__ == '__main__':
    dbConn = beerDB()
    dbConn.connect( '/opt/beermon/data/beermonSQLite3DB.db' )

    if( len(sys.argv) != 2 ):
        print( 'ERROR: Usage' )
        usage()
        print( 'Datasets:')
        for proj in dbConn.getAvailableProj():
            print( '\t%s' % proj )
        dbConn.close()
        sys.exit(1)

    projName = sys.argv[1]

    if( dbConn.setProject( projName ) ):
        header = ['timestamp']
        for prb in dbConn.getProbes():
            header.append( prb[1].replace(' ', '') )
        print( ','.join(header) )
        for data in dbConn.fetchTemperatures( interval = 60):
            print( ','.join( [str(s) for s in data] ) )

    dbConn.close()
