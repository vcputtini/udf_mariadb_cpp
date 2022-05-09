-- =========================================================================
--
-- The library must be copied to the MySQL/MariaDB plugin directory.
-- Check your Linux distribution for the correct path.
-- To confirm the correct path use: show variables like '%plugin%';
--
-- Syntax:
-- CREATE [OR REPLACE] [AGGREGATE] FUNCTION [IF NOT EXISTS] function_name
--    RETURNS {STRING|INTEGER|REAL|DECIMAL}
--    SONAME shared_library_name
-- =========================================================================

-- USE test;

CREATE OR REPLACE FUNCTION slp_int RETURNS INTEGER SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE FUNCTION slp_str RETURNS STRING SONAME 'libvcpsquidlogparser.so';

CREATE OR REPLACE FUNCTION slp_urldecode RETURNS STRING SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE FUNCTION slp_urlparts RETURNS STRING SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE FUNCTION slp_toUnixTs RETURNS INTEGER SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE FUNCTION slp_toSquidTs RETURNS STRING SONAME 'libvcpsquidlogparser.so';

CREATE OR REPLACE AGGREGATE FUNCTION slp_sum RETURNS INTEGER SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE AGGREGATE FUNCTION slp_countbyrm RETURNS INTEGER SONAME 'libvcpsquidlogparser.so';
CREATE OR REPLACE AGGREGATE FUNCTION slp_countbyhttpcode RETURNS INTEGER SONAME 'libvcpsquidlogparser.so';
