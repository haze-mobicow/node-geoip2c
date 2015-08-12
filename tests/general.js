
var geo = require("../build/Release/geoip2c");
files = {};
files[geo.LOOKUP_KEY_COUNTRY] = "/usr/src/app/maxmind/GeoIP2-Country.mmdb";
files[geo.LOOKUP_KEY_CITY] = "/usr/src/app/maxmind/GeoIP2-City.mmdb";
files[geo.LOOKUP_KEY_ISP] = "/usr/src/app/maxmind/GeoIP2-ISP.mmdb";
files[geo.LOOKUP_KEY_NETSPEED] = "/usr/src/app/maxmind/GeoIP2-Connection-Type.mmdb";
files[geo.LOOKUP_KEY_ANONYMOUS] = "/usr/src/app/maxmind/GeoIP2-Anonymous-IP.mmdb";

geo.load(files);
var TEST_IP = "8.8.8.8"

// Review tests
exports.test_country_key = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_COUNTRY] = geo.T_COUNTRY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(rez.hasOwnProperty("country"), "country - property");
    test.ok(!rez.hasOwnProperty("city"), "city - absent");
    test.ok(!rez.hasOwnProperty("isp"), "isp - absent");
    test.ok(!rez.hasOwnProperty("netspeed"), "netspeed - absent");
    test.ok(!rez.hasOwnProperty("anonymous"), "anonymous - absent");

    test.done();
}

// Review tests
exports.test_city_key = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_CITY] = geo.T_COUNTRY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_TYPE_COUNTRY), "country - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    test.done();
}

exports.test_isp_key = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_ISP] = geo.T_ISP_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_TYPE_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    test.done();
}

exports.test_netspeed_key = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_NETSPEED] = geo.T_NETSPEED_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_TYPE_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    test.done();
}

exports.test_anonymous_key = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_ANONYMOUS] = geo.T_ANYM_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_TYPE_COUNTRY), "country - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - absent");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - absent");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - absent");

    test.done();
}

// Test Result Types fields
exports.test_country_fields = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_COUNTRY] = geo.T_COUNTRY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - property");

    var country = rez[geo.LOOKUP_KEY_COUNTRY];
    test.ok(country.hasOwnProperty(geo.LABEL_COUNTRY_NAME), "country name");
    test.ok(country.hasOwnProperty(geo.LABEL_COUNTRY_CODE), "country code");
    test.ok(country.hasOwnProperty(geo.LABEL_COUNTRY_CONTINENT), "country conktinent");
    test.ok(country.hasOwnProperty(geo.LABEL_COUNTRY_REGISTRED), "country registred");

    test.done();
}

exports.test_city_fields = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_CITY] = geo.T_CITY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - property");

    var city = rez[geo.LOOKUP_KEY_CITY];
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_NAME), "city name");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_ZIP), "city code");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_TZ), "citty timezone");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_REGION), "city region");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_METRO), "city metro");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_LAT), "city latitude");
    test.ok(city.hasOwnProperty(geo.LABEL_CITY_LON), "city longitude");

    test.done();
}

exports.test_isp_fields = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_ISP] = geo.T_CITY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");

    var isp = rez[geo.LOOKUP_KEY_ISP];
    test.ok(isp.hasOwnProperty(geo.LABEL_ISP_NAME), "isp name");
    test.ok(isp.hasOwnProperty(geo.LABEL_ISP_CODE), "isp code");
    test.ok(isp.hasOwnProperty(geo.LABEL_ISP_CORGANIZATION), "isp code organization");
    test.ok(isp.hasOwnProperty(geo.LABEL_ISP_NORGANIZATION), "isp name organization");

    test.done();
}

exports.test_anonymous_fields  = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_ANONYMOUS] = geo.T_ANYM_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");

    var anonym = rez[geo.LOOKUP_KEY_ANONYMOUS];
    test.ok(anonym.hasOwnProperty(geo.LABEL_ANYM_IS_ANONYMOUS), "is anonym");
    test.ok(anonym.hasOwnProperty(geo.LABEL_ANYM_IS_PUBPROXY), "is pub proxy");
    test.ok(anonym.hasOwnProperty(geo.LABEL_ANYM_IS_VPN), "is vpn");
    test.ok(anonym.hasOwnProperty(geo.LABEL_ANYM_IS_HOSTING), "is hosting");
    test.ok(anonym.hasOwnProperty(geo.LABEL_ANYM_IS_TOR), "is tor");

    test.done();
}


exports.test_netspeed_fields = function(test) {
    var opts = {};
    opts[geo.LOOKUP_KEY_NETSPEED] = geo.T_COUNTRY_ALL;

    var rez = geo.lookupIp(TEST_IP, opts);
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_COUNTRY), "country - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_CITY), "city - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ISP), "isp - property");
    test.ok(!rez.hasOwnProperty(geo.LOOKUP_KEY_ANONYMOUS), "anonymous - property");
    test.ok(rez.hasOwnProperty(geo.LOOKUP_KEY_NETSPEED), "netspeed - property");

    var netspeed = rez[geo.LOOKUP_KEY_NETSPEED];
    test.ok(netspeed.hasOwnProperty(geo.LABEL_NETSPEED_TYPE), "connection_type");

    test.done();
}

