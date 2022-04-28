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

-- Install UDAs - Aggragations
CREATE OR REPLACE AGGREGATE FUNCTION count_if RETURNS INTEGER SONAME 'libvcputilities.so';
CREATE OR REPLACE AGGREGATE FUNCTION sum_if RETURNS REAL SONAME 'libvcputilities.so';
CREATE OR REPLACE AGGREGATE FUNCTION avg_if RETURNS REAL SONAME 'libvcputilities.so';


CREATE OR REPLACE FUNCTION time_convert RETURNS REAL SONAME 'libvcputilities.so';

