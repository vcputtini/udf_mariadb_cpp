SELECT 
    count(*) as _TOTAL_ENTRIES_,
    slp_countbyrm("combined", log, "get") AS _GET_,
    slp_countbyrm("combined", log, "put") AS _PUT_,
    slp_countbyrm("combined", log, "post") AS _POST_,
    slp_countbyrm("combined", log, "connect") AS _CONNECT_,
    slp_countbyrm("combined", log, "head") AS _HEAD_,
    slp_countbyrm("combined", log, "delete") AS _DELETE_,
    slp_countbyrm("combined", log, "options") AS _OPTIONS_,
    slp_countbyrm("combined", log, "patch") AS _PATCH_,
    slp_countbyrm("combined", log, "trace") AS _TRACE_,
    slp_countbyrm("combined", log, "others") AS _OTHERS
FROM 
    squid_combined
WHERE 
   slp_str("combined", log, "source_ip_address") = "192.168.1.22";
  
  
-- Possible output:
--
-- With "WHERE" clausule:
--
-- MariaDB [test]> source vcpsquidlogparser/docs/ex-select-01.sql;
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- | _TOTAL_ENTRIES_ | _GET_ | _PUT_ | _POST_ | _CONNECT_ | _HEAD_ | _DELETE_ | _OPTIONS_ | _PATCH_ | _TRACE_ | _OTHERS |
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- |            7912 |  4746 |     0 |    537 |      2467 |    116 |        0 |        26 |       0 |       0 |       0 |
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- 1 row in set (9.233 sec)


-- Without "WHERE" clausule (all log entries)
--
-- MariaDB [test]> source vcpsquidlogparser/docs/ex-select-01.sql;
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- | _TOTAL_ENTRIES_ | _GET_ | _PUT_ | _POST_ | _CONNECT_ | _HEAD_ | _DELETE_ | _OPTIONS_ | _PATCH_ | _TRACE_ | _OTHERS |
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- |           24462 | 11633 |     1 |   1863 |      9704 |    141 |        0 |        56 |       0 |       0 |       0 |
-- +-----------------+-------+-------+--------+-----------+--------+----------+-----------+---------+---------+---------+
-- 1 row in set (21.616 sec)

    
