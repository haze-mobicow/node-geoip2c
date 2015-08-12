var geo = require("../build/Release/geoip2c");

files = {};
files[geo.LOOKUP_KEY_COUNTRY] = "/usr/src/app/maxmind/GeoIP2-Country.mmdb";
files[geo.LOOKUP_KEY_CITY] = "/usr/src/app/maxmind/GeoIP2-City.mmdb";
files[geo.LOOKUP_KEY_ISP] = "/usr/src/app/maxmind/GeoIP2-ISP.mmdb";
files[geo.LOOKUP_KEY_NETSPEED] = "/usr/src/app/maxmind/GeoIP2-Connection-Type.mmdb";
files[geo.LOOKUP_KEY_ANONYMOUS] = "/usr/src/app/maxmind/GeoIP2-Anonymous-IP.mmdb";

geo.load(files);

var opts = {
};

opts[geo.LOOKUP_KEY_COUNTRY] = geo.T_COUNTRY_ALL;
opts[geo.LOOKUP_KEY_CITY] = geo.T_CITY_ALL;
opts[geo.LOOKUP_KEY_ISP] = geo.T_ISP_ALL;
opts[geo.LOOKUP_KEY_ANONYMOUS] = geo.T_ANYM_ALL;
opts[geo.LOOKUP_KEY_NETSPEED] = geo.T_NETSPEED_ALL;

console.log("Start: ", opts);
//rez = geo.lookupIp("202.194.101.150");
//console.log("rez -> ", rez)
for (var i = 0; i <= 1000000; i++) {
    rez = geo.lookupIp("202.194.101.150", opts);
}
//console.log("The End");
