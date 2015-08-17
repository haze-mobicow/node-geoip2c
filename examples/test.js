var geo = require("../build/Release/geoip2c");

files = {};
files[geo.LOOKUP_KEY_COUNTRY] = "/usr/src/app/maxmind/GeoIP2-Country.mmdb";
files[geo.LOOKUP_KEY_CITY] = "/usr/src/app/maxmind/GeoIP2-City.mmdb";
files[geo.LOOKUP_KEY_ISP] = "/usr/src/app/maxmind/GeoIP2-ISP.mmdb";
files[geo.LOOKUP_KEY_NETSPEED] = "/usr/src/app/maxmind/GeoIP2-Connection-Type.mmdb";
files[geo.LOOKUP_KEY_ANONYMOUS] = "/usr/src/app/maxmind/GeoIP2-Anonymous-IP.mmdb";

console.log("Load files -> ", files);
geo.load(files);

var opts = {
    "country": geo.T_COUNTRY_CODE | geo.T_COUNTRY_NAME,
    "isp": geo.T_ISP_ALL,
    "anonymous": geo.T_ANYM_ALL,
};
opts[geo.LOOKUP_KEY_NETSPEED] = geo.T_NETSPEED_ALL;

console.log("Test Options: ", opts);
console.log("-- [Test Start ] -------\n");

console.log("Lookup: 8.8.8.8")
result = geo.lookupIp("8.8.8.8", opts);
console.log("Lookup Result -> ", result);
console.log("\n\n");

console.log("(Round2) Public Proxy: 202.194.101.150");
result = geo.lookupIp("202.194.101.150", opts);
console.log("Lookup Result -> ", result);
console.log("\n\n");

console.log("(Round3) Tor IP 91.109.247.173");
result = geo.lookupIp("91.109.247.173", opts);
console.log("Lookup Result -> ", result);
console.log("\n\n");

console.log("(Round4) IPv6 2001:1388:b47:df34:5ddf:cecb:4213:cf37");
result = geo.lookupIp("2001:1388:b47:df34:5ddf:cecb:4213:cf37", opts);
console.log("Lookup Result -> ", result);
console.log("\n\n");


