-- ***************************************************************************
-- Examples of using the slp_urlparts() function
--
-- See "dosc/ex-table-01.sql" for log_squid table structure.
-- ***************************************************************************

-- #1
SELECT  slp_urlparts(url,"protocol") AS _PROTOCOL_, 
        slp_urlparts(url,"domain") AS _DOMAIN_,
        slp_urlparts(url,"path") AS _PATH_,
        slp_urlparts(url,"query") AS _QUERY_,
        slp_urlparts(url,"fragment") AS _FRAGMENT_
FROM log_squid
HAVING _PROTOCOL_ = "http";

-- #2
SELECT  slp_urlparts(url,"protocol") AS _PROTOCOL_, 
        slp_urlparts(url,"domain") AS _DOMAIN_,
        slp_urlparts(url,"path") AS _PATH_,
        slp_urlparts(url,"query") AS _QUERY_,
        slp_urlparts(url,"fragment") AS _FRAGMENT_
FROM log_squid
WHERE source_ip = "192.168.1.106"  AND slp_urlparts(url,"protocol") = "https;



-- ***************************************************************************
--
-- Possible output: #1
-- +------------+--------------------------------+----------------------------------------------------------------+---------+------------+
-- | _PROTOCOL_ | _DOMAIN_                       | _PATH_                                                         | _QUERY_ | _FRAGMENT_ |
-- +------------+--------------------------------+----------------------------------------------------------------+---------+------------+
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/authrootstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/pinrulesstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/authrootstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/pinrulesstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/authrootstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/disallowedcertstl.cab | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/pinrulesstl.cab       | ?       |            |
-- | http       | ctldl.windowsupdate.com        | /msdownload/update/v3/static/trustedr/en/pinrulesstl.cab       | ?       |            |
-- | http       | r3.o.lencr.org                 | /                                                              |         |            |
-- | http       | r3.o.lencr.org                 | /                                                              |         |            |
-- | http       | r3.o.lencr.org                 | /                                                              |         |            |
-- +------------+--------------------------------+----------------------------------------------------------------+---------+------------+


-- Possible output: #2
-- ------------+-------------------+--------------------------------------------------------------+---------+------------+
-- | _PROTOCOL_ | _DOMAIN_          | _PATH_                                                       | _QUERY_ | _FRAGMENT_ |
-- +------------+-------------------+--------------------------------------------------------------+---------+------------+
-- | https      | webtuneup.awg.com | /partners/wtu/4.0.9.626/IEPackages/remote_configuration.aspx | ?       |            |
-- | https      | mysearch.awg.com  | /                                                            | ?       |            |
-- | https      | webtuneup.awg.com | /partners/wtu/4.0.9.626/IEPackages/remote_configuration.aspx | ?       |            |
-- | https      | mysearch.awg.com  | /                                                            | ?       |            |
-- | https      | webtuneup.awg.com | /partners/wtu/4.0.9.626/IEPackages/remote_configuration.aspx | ?       |            |
-- | https      | mysearch.awg.com  | /                                                            | ?       |            |
-- | https      | webtuneup.awg.com | /partners/wtu/4.0.9.626/IEPackages/remote_configuration.aspx | ?       |            |
-- | https      | mysearch.awg.com  | /                                                            | ?       |            |
-- | https      | webtuneup.awg.com | /partners/wtu/4.0.9.626/IEPackages/remote_configuration.aspx | ?       |            |
-- | https      | mysearch.awg.com  | /                                                            | ?       |            |
-- +------------+-------------------+--------------------------------------------------------------+---------+------------+
