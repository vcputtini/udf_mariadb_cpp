-- =========================================================================
--
--
--
-- Syntax:
-- CREATE [OR REPLACE] [AGGREGATE] FUNCTION [IF NOT EXISTS] function_name
--    RETURNS {STRING|INTEGER|REAL|DECIMAL}
--    SONAME shared_library_name
--
-- Under MariaDB the shared_library must be Installed in /usr/lib64/mariadb/plugin/
-- =========================================================================

-- USE default;

-- Install UDFs
CREATE OR REPLACE FUNCTION geo_distance RETURNS REAL SONAME 'libvcplocation.so';
CREATE OR REPLACE FUNCTION geo_azimuth RETURNS REAL SONAME 'libvcplocation.so';

CREATE OR REPLACE FUNCTION unit_convert RETURNS REAL SONAME 'libvcplocation.so';
CREATE OR REPLACE FUNCTION is_valid_lat RETURNS INTEGER SONAME 'libvcplocation.so';
CREATE OR REPLACE FUNCTION is_valid_lon RETURNS INTEGER SONAME 'libvcplocation.so';
CREATE OR REPLACE FUNCTION utm_zone RETURNS INTEGER SONAME 'libvcplocation.so';

CREATE OR REPLACE FUNCTION coords_to_utm RETURNS STRING  SONAME 'libvcplocation.so';
CREATE OR REPLACE FUNCTION utm_to_coords RETURNS STRING  SONAME 'libvcplocation.so';

-- Install UDAs - Aggragations
CREATE OR REPLACE AGGREGATE FUNCTION geo_sumdist RETURNS REAL SONAME 'libvcplocation.so';

-- EXAMPLES
create table coords (
  pto varchar(5) not null,
  lat_a double(15,10) not null,
  lon_a double(15,10) not null
);

insert into coords (pto, lat_a, lon_a) values ('p00',-31.721238,-55.980449);
insert into coords (pto, lat_a, lon_a) values ('p01',-31.744598,-55.985942);
insert into coords (pto, lat_a, lon_a) values ('p02',-31.786047,-55.984569);


-- Used to calculate distances
create table coords (
  pto varchar(5) not null,
  lat_a double(15,10) not null,
  lon_a double(15,10) not null
  lat_b double(15,10) not null,
  lon_b double(15,10) not null
);

--Used to save the result of converting GPS coordinates to UTM in JSON format
create table utm_json (
  fld_j json
);
  
