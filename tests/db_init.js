
var geo = require("../build/Release/geoip2c");

DB_COUNTRY_PATH = "/usr/src/app/maxmind/GeoIP2-Country.mmdb";
DB_CITY_PATH = "/usr/src/app/maxmind/GeoIP2-City.mmdb";
DB_ISP_PATH = "/usr/src/app/maxmind/GeoIP2-ISP.mmdb";
DB_NETSPEED_PATH =  "/usr/src/app/maxmind/GeoIP2-Connection-Type.mmdb";
DB_ANONYMOUS_PATH = "/usr/src/app/maxmind/GeoIP2-Anonymous-IP.mmdb";

var TEST_IP = "8.8.8.8"

exports.test_load_country = function(test) {
    var files = {};
    files[geo.LOOKUP_KEY_COUNTRY] =  DB_COUNTRY_PATH;
    load_status = geo.load(files);
    test.ok(load_status[geo.LOOKUP_KEY_COUNTRY] == "", "Success load must be empty");

    var rez = geo.lookupIp(TEST_IP);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    geo.unload();
    test.done();
}

exports.test_load_city = function(test) {
    var files = {};
    files[geo.LOOKUP_KEY_CITY] =  DB_CITY_PATH;
    load_status = geo.load(files);
    test.ok(load_status[geo.LOOKUP_KEY_CITY] == "", "Success load must be empty");

    var rez = geo.lookupIp(TEST_IP);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    geo.unload();
    test.done();
}

exports.test_load_isp = function(test) {
    var files = {};
    files[geo.LOOKUP_KEY_ISP] =  DB_ISP_PATH;
    load_status = geo.load(files);
    test.ok(load_status[geo.LOOKUP_KEY_ISP] == "", "Success load must be empty");

    var rez = geo.lookupIp(TEST_IP);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    geo.unload();
    test.done();
}

exports.test_load_netspeed = function(test) {
    var files = {};
    files[geo.LOOKUP_KEY_NETSPEED] = DB_NETSPEED_PATH;
    geo.load(files);
    test.ok(load_status[geo.LOOKUP_KEY_ISP] == "", "Success load must be empty");

    var rez = geo.lookupIp(TEST_IP);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    geo.unload();
    test.done();
}

exports.test_load_anonymous = function(test) {
    var files = {};
    files[geo.LOOKUP_KEY_ANONYMOUS] = DB_ANONYMOUS_PATH;
    geo.load(files);
    test.ok(load_status[geo.LOOKUP_KEY_ISP] == "", "Success load must be empty");

    var rez = geo.lookupIp(TEST_IP);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    geo.unload();
    test.done();
}

