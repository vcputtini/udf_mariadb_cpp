-- ****************************************************************************
--
-- A method to create a more suitable table for handling log data.
-- 
-- ****************************************************************************

DROP INDEX IF EXISTS log_squid_idx ON log_squid;
DROP TABLE log_squid;

CREATE TABLE IF NOT EXISTS log_squid (
    idx BIGINT NOT NULL AUTO_INCREMENT,
    timestamp BIGINT NOT NULL,
    response_time INTEGER NOT NULL,
    source_ip CHAR(15) NOT NULL,
    reqstatus_hierstatus VARCHAR(30) NOT NULL,
    total_size_reply INTEGER NOT NULL,
    request_method CHAR(10) NOT NULL,
    url TEXT NOT NULL,
    user_name VARCHAR(30) NOT NULL,
    hierstatus_ipaddress VARCHAR(50) NOT NULL,
    mime VARCHAR(50) NOT NULL,
    PRIMARY KEY (idx)
);

CREATE INDEX log_squid_idx ON log_squid(timestamp);


INSERT INTO log_squid( timestamp, response_time, source_ip,
        reqstatus_hierstatus, total_size_reply, request_method,
        url, user_name, hierstatus_ipaddress, mime)
        SELECT        
            slp_int("squid", log, "timestamp"),
            slp_int("squid", log, "response_time"),
            slp_str("squid", log, "source_ip_address"),
            slp_str("squid", log, "reqstatus_hierstatus"),
            slp_int("squid", log, "total_size_reply"),
            slp_str("squid", log, "request_method"),
            slp_str("squid", log, "url"),
            slp_str("squid", log, "username"),
            slp_str("squid", log, "hier_status_server_ip"),
            slp_str("squid", log, "mimetype")
       FROM logsquid


