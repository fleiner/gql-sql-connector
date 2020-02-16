--
-- PostgreSQL database dump
--

-- Dumped from database version 9.6.7
-- Dumped by pg_dump version 9.6.7

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: plpgsql; Type: EXTENSION; Schema: -; Owner: 
--

CREATE EXTENSION IF NOT EXISTS plpgsql WITH SCHEMA pg_catalog;


--
-- Name: EXTENSION plpgsql; Type: COMMENT; Schema: -; Owner: 
--

COMMENT ON EXTENSION plpgsql IS 'PL/pgSQL procedural language';


SET search_path = public, pg_catalog;

--
-- Name: alltypes_enum; Type: TYPE; Schema: public; Owner: claudio
--

CREATE TYPE alltypes_enum AS ENUM (
    '1',
    '2',
    '3'
);


ALTER TYPE alltypes_enum OWNER TO claudio;

--
-- Name: alltypes_set; Type: TYPE; Schema: public; Owner: claudio
--

CREATE TYPE alltypes_set AS ENUM (
    '1',
    '2',
    '3'
);


ALTER TYPE alltypes_set OWNER TO claudio;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- Name: GR; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE "GR" (
    last character varying(30) NOT NULL,
    first character varying(30) NOT NULL,
    own bigint NOT NULL,
    home bigint NOT NULL
);


ALTER TABLE "GR" OWNER TO claudio;

--
-- Name: T B; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE "T B" (
    currency double precision NOT NULL,
    date timestamp with time zone,
    string character varying(40) NOT NULL,
    floats double precision NOT NULL,
    ints bigint NOT NULL,
    percents double precision NOT NULL,
    number integer
);


ALTER TABLE "T B" OWNER TO claudio;

--
-- Name: T1; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE "T1" (
    "EINS" bigint NOT NULL,
    "ZWEI" bigint NOT NULL,
    "DREI UND" bigint NOT NULL,
    "VIER" bigint NOT NULL
);


ALTER TABLE "T1" OWNER TO claudio;

--
-- Name: alltypes; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE alltypes (
    "varchar" character varying(20) DEFAULT 'hello'::character varying,
    tinyint smallint DEFAULT '120'::smallint NOT NULL,
    text text NOT NULL,
    date date DEFAULT '1980-01-01'::date,
    "smallint" smallint DEFAULT '30000'::smallint NOT NULL,
    mediumint integer DEFAULT 8000000 NOT NULL,
    "int" bigint DEFAULT '2000000000'::bigint NOT NULL,
    "bigint" bigint DEFAULT '9000000000000000000'::numeric NOT NULL,
    ubigint numeric NOT NULL,
    "float" double precision DEFAULT '1.125'::double precision NOT NULL,
    double double precision DEFAULT '2.375'::double precision NOT NULL,
    "decimal" numeric(9,4) DEFAULT 12345.6789 NOT NULL,
    datetime timestamp with time zone DEFAULT '1980-01-01 13:14:15.456+00'::timestamp with time zone,
    "timestamp" timestamp with time zone DEFAULT '1981-01-02 09:10:11+00'::timestamp with time zone,
    "time" time(3) without time zone DEFAULT '04:05:06.789'::time without time zone NOT NULL,
    year integer DEFAULT 1983 NOT NULL,
    "char" character(10) DEFAULT 'blah'::bpchar NOT NULL,
    tinyblob bytea NOT NULL,
    tinytext text NOT NULL,
    blob bytea NOT NULL,
    mediumblob bytea NOT NULL,
    mediumtext text NOT NULL,
    longblob bytea NOT NULL,
    longtext text NOT NULL,
    enum alltypes_enum DEFAULT '2'::alltypes_enum NOT NULL,
    set alltypes_set[] DEFAULT '{2,3}'::alltypes_set[] NOT NULL,
    bool boolean DEFAULT true NOT NULL,
    "binary" bytea DEFAULT '\x3000000000000000000000000000000000000000'::bytea NOT NULL,
    varbinary bytea NOT NULL,
    t1key bigint NOT NULL
);


ALTER TABLE alltypes OWNER TO claudio;

--
-- Name: counter; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE counter (
    counter bigint NOT NULL
);


ALTER TABLE counter OWNER TO claudio;

--
-- Name: pv; Type: TABLE; Schema: public; Owner: claudio
--

CREATE TABLE pv (
    name character varying(30) NOT NULL,
    dept character varying(30) NOT NULL,
    "lunchTime" time without time zone NOT NULL,
    salary numeric(5,0) NOT NULL,
    hiredate date NOT NULL,
    age bigint NOT NULL,
    issenior boolean NOT NULL,
    senioritystarttime timestamp with time zone
);


ALTER TABLE pv OWNER TO claudio;

--
-- Data for Name: GR; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY "GR" (last, first, own, home) FROM stdin;
Bear	Hello	12	4
Bear	do	10	10
Bear	how	44	13
Bear	there	15	3
Bee	black	22	14
Bee	yellow	112	31
\.


--
-- Data for Name: T B; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY "T B" (currency, date, string, floats, ints, percents, number) FROM stdin;
2	2017-02-28 23:59:13+00	space  space	1.19999999999999999e-10	2	0.400000000000000022	\N
4.54999999999999982	2017-02-10 00:00:00+00	hallo	1.23399999999999999	4	0.200000000000000011	\N
1	1400-01-01 01:01:01+00		1e+21	1	0.5	0
2.33000000000000007	2017-01-12 08:00:00+00	Hütätä	2.34567000000000014	3	0.299999999999999989	3
\.


--
-- Data for Name: T1; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY "T1" ("EINS", "ZWEI", "DREI UND", "VIER") FROM stdin;
2	32	1	34
3	11	1	12
5	4	31	23
7	24	23	3
\.


--
-- Data for Name: alltypes; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY alltypes ("varchar", tinyint, text, date, "smallint", mediumint, "int", "bigint", ubigint, "float", double, "decimal", datetime, "timestamp", "time", year, "char", tinyblob, tinytext, blob, mediumblob, mediumtext, longblob, longtext, enum, set, bool, "binary", varbinary, t1key) FROM stdin;
	0		1234-01-01	0	0	0	0	0	12.25	123.125	0.0000	\N	2022-12-31 00:00:00+00	00:00:00	2155	          	\\x		\\x	\\x		\\x		2	{2,3}	f	\\x0000000000000000000000000000000000000000	\\x	2
01234567890123456789	-128	something\r\nlong\r\nand with\r\nUTF 8\r\näöü	2980-01-01	-32768	0	4294967295	9223372036854775807	9223372036854775807	1.17548999999999994e-38	2.22507385850720138e-308	-99999.9999	1080-01-01 13:14:15.456+00	\N	23:59:59.999	1901	abcdefghij	\\x	asdadfasdfasdfasdfasdfasdf	\\x	\\x	asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf	\\x	asdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdfasdadfasdfasdfasdfasdfasdf	2	{2,3}	t	\\x3000000000000000000000000000000000000000	\\x30	7
hallo äöü अतुल	127	bla	\N	32767	16777215	2000000000	-9223372036854775808	18446744073709551615	3.40282000000000014e+38	1.79769313485999993e+308	12345.1235	1940-01-01 13:14:15.456+00	1970-01-01 00:00:01+00	04:05:06.089	1983	blah      	\\x	tttt	\\x	\\x	medium	\\x	and very long	2	{2,3}	t	\\x30	\\x30	3
hello	120	small	1980-12-31	30000	8000000	123123123	9000000000000000000	9000000000000000000	-1.17548999999999994e-38	-2.2250738585071999e-308	6789.0000	2980-12-31 23:59:59.999+00	1981-01-02 09:10:11.123+00	04:05:06.789	0	blah      	\\x		\\x	\\x		\\x		2	{2,3}	t	\\x30	\\x30	5
\.


--
-- Data for Name: counter; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY counter (counter) FROM stdin;
101
\.


--
-- Data for Name: pv; Type: TABLE DATA; Schema: public; Owner: claudio
--

COPY pv (name, dept, "lunchTime", salary, hiredate, age, issenior, senioritystarttime) FROM stdin;
Ben	Sales	12:00:00	400	2002-10-10	32	t	2005-03-09 12:30:00+00
Dana	Sales	12:00:00	350	2004-09-08	25	f	\N
Dave	Eng	12:00:00	500	2006-04-19	27	f	\N
John	Eng	12:00:00	1000	2005-03-19	35	t	2007-12-02 15:56:00+00
Mike	Marketing	13:00:00	800	2005-01-10	24	t	2007-12-30 14:40:00+00
Sally	Eng	13:00:00	600	2005-10-10	30	f	\N
\.


--
-- Name: alltypes idx_17327_primary; Type: CONSTRAINT; Schema: public; Owner: claudio
--

ALTER TABLE ONLY alltypes
    ADD CONSTRAINT idx_17327_primary PRIMARY KEY ("int");


--
-- Name: counter idx_17352_primary; Type: CONSTRAINT; Schema: public; Owner: claudio
--

ALTER TABLE ONLY counter
    ADD CONSTRAINT idx_17352_primary PRIMARY KEY (counter);


--
-- Name: GR idx_17355_primary; Type: CONSTRAINT; Schema: public; Owner: claudio
--

ALTER TABLE ONLY "GR"
    ADD CONSTRAINT idx_17355_primary PRIMARY KEY (last, first);


--
-- Name: pv idx_17358_primary; Type: CONSTRAINT; Schema: public; Owner: claudio
--

ALTER TABLE ONLY pv
    ADD CONSTRAINT idx_17358_primary PRIMARY KEY (name);


--
-- Name: T B idx_17361_primary; Type: CONSTRAINT; Schema: public; Owner: claudio
--

ALTER TABLE ONLY "T B"
    ADD CONSTRAINT idx_17361_primary PRIMARY KEY (ints);


--
-- Name: SCHEMA public; Type: ACL; Schema: -; Owner: postgres
--

GRANT USAGE ON SCHEMA public TO gqltest;


--
-- Name: TABLE "GR"; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE "GR" TO gqltest;


--
-- Name: TABLE "T B"; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE "T B" TO gqltest;


--
-- Name: TABLE "T1"; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE "T1" TO gqltest;


--
-- Name: TABLE alltypes; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE alltypes TO gqltest;


--
-- Name: TABLE counter; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE counter TO gqltest;


--
-- Name: TABLE pv; Type: ACL; Schema: public; Owner: claudio
--

GRANT SELECT ON TABLE pv TO gqltest;


--
-- PostgreSQL database dump complete
--

