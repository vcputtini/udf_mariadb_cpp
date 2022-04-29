-- ****************************************************************************
--
-- A method to create a more suitable table for handling log data.
-- 
-- ****************************************************************************

-- logformat combined   %>a %[ui %[un [%tl] "%rm %ru HTTP/%rv" %>Hs %<st "%{Referer}>h" "%{User-Agent}>h" %Ss:%Sh

--DROP INDEX IF EXISTS log_combined_idx ON log_combined;
DROP TABLE log_combined;

CREATE TABLE IF NOT EXISTS log_combined (
    idx BIGINT NOT NULL AUTO_INCREMENT,
    source_ip CHAR(15) NOT NULL,
    user_name_ident VARCHAR(30) NOT NULL,
    user_name VARCHAR(30) NOT NULL,
    local_time VARCHAR(50) NOT NULL,
    request_method CHAR(10) NOT NULL,
    url TEXT NOT NULL,
    req_proto_version VARCHAR(50) NOT NULL,
    http_status INTEGER NOT NULL,
    total_size_reply INTEGER NOT NULL,
    referrer TEXT NOT NULL,
    user_agent TEXT NOT NULL,
    hierstatus_ipaddress VARCHAR(50) NOT NULL,
    PRIMARY KEY (idx)
);

CREATE INDEX log_combined_idx ON log_combined(source_ip);


INSERT INTO log_combined(source_ip, user_name_ident, user_name,
    local_time, request_method, url, req_proto_version,
    http_status, total_size_reply, referrer, user_agent,hierstatus_ipaddress)
        SELECT        
            slp_str("combined", log, "source_ip_address"),
            slp_str("combined", log, "usernameident"),
            slp_str("combined", log, "username"),
            slp_str("combined", log, "localtime"),
            slp_str("combined", log, "request_method"),
            slp_str("combined", log, "url"),
            slp_str("combined", log, "request_proto_ver"),
            slp_int("combined", log, "http_status"),
            slp_int("combined", log, "total_size_reply"),
            slp_str("combined", log, "referrer"),
            slp_str("combined", log, "useragent"),
            slp_str("combined", log, "hier_status_server_ip")
       FROM squid_combined;


