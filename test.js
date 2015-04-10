var geo = require("./build/Release/geoip2c");
function resolve(ip)
{
    var ret = geo.lookupIp(ip);
    console.log(ip, '->', JSON.stringify(ret));
}

geo.load(process.argv[2],process.argv[3]);
console.log("==== Looking up IPv4: ====")
resolve("1.2.3.4");
console.log("==== Looking up IPv6: ====")
resolve("2001:1388:b47:df34:5ddf:cecb:4213:cf37");
console.log("Done");
