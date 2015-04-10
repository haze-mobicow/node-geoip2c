var geo = require("./build/Release/geoip2c");
function resolve(ip)
{
    var ret = geo.lookupIp(ip);
    console.log(ip, '->', JSON.stringify(ret));
}
console.log('loding',process.argv[2],process.argv[3],process.argv[4],process.argv[5]);


geo.load(process.argv[2],process.argv[3],process.argv[4],process.argv[5]);
console.log("==== Looking up IPv4: ====")
resolve("24.200.3.4");
console.log("==== Looking up IPv6: ====")
resolve("2001:1388:b47:df34:5ddf:cecb:4213:cf37");
console.log("Done");
