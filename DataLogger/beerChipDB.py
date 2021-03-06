import sqlite3

class beerChipDB:
    def connect(self, connInfo):
        raise NotImplementedError("Should have implemented this")

    def query(self, queryString ):
        raise NotImplementedError("Should have implemented this")

    def querySafe(self, queryString, valueTuple):
        raise NotImplementedError("Should have implemented this")

    def execute(self, sqlString):
        raise NotImplementedError("Should have implemented this")

    def fetchOne(self, queryString, valueTuple):
        raise NotImplementedError("Should have implemented this")

    #
    # Utility Functions
    #
    def setProject(self, projName ):
        raise NotImplementedError("Should have implemented this")

    def getAvailableProject(self):
        raise NotImplementedError("Should have implemented this")

    def getProbes( self ):
        raise NotImplementedError("Should have implemented this")

    def fetchTemperatures(self):
        raise NotImplementedError("Should have implemented this")

    def close(self):
        raise NotImplementedError("Should have implemented this")

class beerChipSQLiteDB( beerChipDB ):
    def __init__(self):
        self.conn = None
        self.projName = None
        self.projId = None

    def connect(self, dbFilename ):
        try:
            self.conn = sqlite3.connect( dbFilename )
        except:
            print( "ERROR: Cannot connect to SQLite3 DB file %s" % (dbFilename) )

    def query(self, queryString):
        if( self.conn == None ):
            print( "ERROR: Database NOT connected" )
            return

        cur = None
        try:
            cur = self.conn.cursor()
            cur.execute( queryString )
            for row in cur.fetchall():
                yield row
        except:
            print( "ERROR executing %s" % (queryString ))

        if( cur != None ):
            cur.close()

    def querySafe(self, queryString, valueTuple ):
        if( self.conn == None ):
            print("ERROR: Database NOT connected")
            return

        cur = None
        try:
            cur = self.conn.cursor()
            cur.execute(queryString, valueTuple )
            for row in cur.fetchall():
                yield row

        except:
            print("ERROR executing %s" % (queryString))

        if (cur != None):
            cur.close()


    def execute(self, sqlString):
        numRows = 0
        if (self.conn == None):
            print("ERROR: Database NOT connected")
            return

        cur = None
        try:
            cur = self.conn.cursor()
            cur.execute(sqlString)
            self.conn.commit()
            numRows = cur.rowcount
        except:
            print( 'ERROR: Error executing %s' % (sqlString) )

        if( cur != None ):
            cur.close()

        return numRows

    def fetchOne(self, queryString, valueTuple ):
        if (self.conn == None):
            print("ERROR: Database NOT connected")
            return

        cur = None
        try:
            cur = self.conn.cursor()
            cur.execute(queryString, valueTuple )
            row = cur.fetchone()

        except:
            print("ERROR executing %s" % (queryString))

        finally:
            if (cur != None):
                cur.close()

        return row

    def getAvailableProj(self):
        result = []
        if (self.conn == None):
            print("ERROR: Database NOT connected")
            return result
        cur = None
        try:
            cur = self.conn.cursor()
            sql  = "SELECT proj_name FROM Project"
            cur.execute( sql )
            for row in cur.fetchall():
                result.append( row[0] )
        except:
            print( "ERROR finding Projects" )
        finally:
            if( cur != None ):
                cur.close()
        return result

    def setProject(self, projName ):
        sql  = "SELECT id FROM Project WHERE proj_name = ?"
        projId = None
        result = False
        for row in self.querySafe( sql, (projName,) ):
            projId = row[0]
        if( projId != None ):
            self.projId = projId
            self.projName = projName
            result = True
        else:
            print( "ERROR: Cannot find project %s" % (projName) )
        return result

    def createProject( self, projName, probes ):
        # probes = { 'name' :
        #            'chan' :
        #            'type' : ['MANUAL', 'NTC_00', 'Setpoint']
        #             min_range :
        #             max_range
        sql  = "INSERT INTO Project ( proj_name, activity ) "
        sql += "VALUES ('%s', 1) " % (projName)
        try:
            projId = self.execute( sql )
            for prb in probes:
                prbInfo = {}
                prbInfo['proj_id'] = projId
                prbInfo['probe_name'] = "'%s'" % (prb[name])
                if( 'type' not in prb ):
                    prbInfo['type'] = "'%s'" % ('NTC_00')
                else:
                    prbInfo['type'] = "'%s'" % (prb['name'])
                if( 'min_range' not in prb ):
                    prbInfo['min_range'] = "%f" % (20.0)
                else:
                    prbInfo ['min_range'] = "%f" % (prb['min_range'])
                if( 'max_range' not in prb ):
                    prbInfo['max_range'] = "%f" % (90.0)
                else:
                    prbInfo ['max_range'] = "%f" % (prb['max_range'])

                sql  = "INSERT INTO Probes (%s) " % (','.join([str(val) for val in prbInfo.keys()]))
                sql += "VALUES (%s)" % (','.join([val for val in prbInfo.values()]))
                self.execute( sql )

                self.projId = projId
                self.projName = projName
        except:
            print( "ERROR: Cannot create project %s" % (projName))

    def getProbes( self ):
        result = []
        if( self.projId == None ):
            print( "ERROR: Must set project" )
            return result

        sql  = "SELECT id, probe_name FROM Probes WHERE proj_id = ?"
        for row in self.querySafe( sql, (self.projId,) ):
            result.append( (row[0], row[1]) )
        return result

    def fetchTemperatures(self, interval = 20 ):
        probes = self.getProbes()
        probeUnion = []
        for prb in probes:
            prbSql  = "SELECT CAST(strftime('%%s', temp_time) / %d AS INTEGER) AS time_group, " % (interval)
            prbSql += "datetime(avg(strftime('%s', temp_time)),'unixepoch') AS centerTime, "
            prb2List = []
            for prb2 in probes:
                if( prb2[0] == prb[0] ):
                    prb2Sql = "AVG(temp) AS %s" % (prb2[1].replace(' ', ''))
                else:
                    prb2Sql = "0.0 AS %s" % (prb2[1].replace(' ', '' ))
                prb2List.append( prb2Sql )
            prbSql += ", ".join(prb2List) + " "
            prbSql += "FROM Temperature WHERE probe_id=%d " % (prb[0])
            prbSql += "GROUP BY strftime('%%s', temp_time) / %d " % interval
            probeUnion.append( prbSql )
        # sql  = "SELECT x.time_group, "
        sql  = "SELECT y.timestamp, " + ', '.join(['y.' + x[1].replace(' ', '') for x in probes]) + ' '
        sql += "FROM ( "
        sql +=     "SELECT "
        sql +=     "DATETIME(AVG(STRFTIME('%s',x.centerTime)),'unixepoch') AS timestamp, "
        sql +=     ', '.join([str('MAX(x.' + x[1].replace(' ', '') + ') AS ' + x[1].replace(' ', '')) for x in probes]) + ' '
        sql +=     "FROM ( "
        sql +=         " UNION ".join(probeUnion)
        sql +=     ") x "
        sql +=     "GROUP BY x.time_group"
        sql += ") y "
        sql += "WHERE " + ' AND '.join( ['y.' + x[1].replace(' ','') + " != 0.0 " for x in probes] ) + ' '

        for row in self.query( sql ):
            yield row

    def close(self):
        if( self.conn != None ):
            self.conn.close()
        self.conn = None





