-- CREATE TABLES
CREATE TABLE COORDS (ID VARCHAR(10), LAT DOUBLE, LON DOUBLE);
INSERT INTO COORDS (ID, LAT, LON) ('P00', -31.721238, -55.980449);
INSERT INTO COORDS (ID, LAT, LON) ('P01', -31.744598, -55.985942);
INSERT INTO COORDS (ID, LAT, LON) ('P02', -31.786047, -55.984569);

-- TABLE WITH 'JSON' TYPE FIELD
CREATE TABLE UTM_JSON (FLD_J JSON CHECK (JSON_VALID(j)));

-- NOTE: IF THE VALUE OF THE UTM ZONE IS 0 THE coords_to_utm() FUNCTION IRA
-- CALCULATE THE CORRECT ZONE.

-- EXAMPLE OF CONVERSION FROM COORDINATES TO UTM WITH CONSTANT VALUES
-- MariaDB [test]> select coords_to_utm(cast(-31.721238 as double), cast(-55.980449 as double),0);
-- +----------------------------------------------------------------------------+
-- | coords_to_utm(cast(-31.721238 as double), cast(-55.980449 as double),0)    |
-- +----------------------------------------------------------------------------+
-- | {"LatLonToUTMXY":{"xE":"596594.560114","yN":"6490010.387716","zone":"21"}} |
-- +----------------------------------------------------------------------------+

-- EXAMPLE OF CONVERSION OF COORDINATES
-- MariaDB [test]> select coords_to_utm(LAT,LON,0) FROM COORDS;
-- +----------------------------------------------------------------------------+
-- | coords_to_utm(lat_a, lon_a,0)                                              |
-- +----------------------------------------------------------------------------+
-- | {"LatLonToUTMXY":{"xE":"596594.560114","yN":"6490010.387716","zone":"21"}} |
-- | {"LatLonToUTMXY":{"xE":"596050.010071","yN":"6487425.891019","zone":"21"}} |
-- | {"LatLonToUTMXY":{"xE":"596137.194143","yN":"6482830.215821","zone":"21"}} |
-- +----------------------------------------------------------------------------+


INSERT INTO UTM_JSON (FLD_J) SELECT coords_to_utm(LAT, LON, 0) FROM COORDS;
-- MariaDB [test]> SELECT * FROM UTM_JSON;
-- +----------------------------------------------------------------------------+
-- | FLD_J                                                                      |
-- +----------------------------------------------------------------------------+
-- | {"LatLonToUTMXY":{"xE":"596594.560114","yN":"6490010.387716","zone":"21"}} |
-- | {"LatLonToUTMXY":{"xE":"596050.010071","yN":"6487425.891019","zone":"21"}} |
-- | {"LatLonToUTMXY":{"xE":"596137.194143","yN":"6482830.215821","zone":"21"}} |
-- +----------------------------------------------------------------------------+
