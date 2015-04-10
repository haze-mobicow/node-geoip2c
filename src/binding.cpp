#include <node.h>
#include <v8.h>
#include <string>
#include <errno.h>
#include <string.h> //for strerror
#include <nan.h>
extern "C" {
#include <maxminddb.h>
}

using namespace v8;
MMDB_s mmdbCountry;
MMDB_s mmdbCity;
MMDB_s mmdbIsp;
MMDB_s mmdbNetspeed;
static bool loaded = false;

#define TYPE_COUNTRY 1;
#define TYPE_CITY 2;
#define TYPE_ISP 3;
#define TYPE_NETSPEED 4;
//std::string dbType = std::string("");

int dbType = 0;

#define MY_THROW_EXCEP(msg) {                    \
    NanThrowError(msg);                         \
    NanReturnUndefined();                        \
}

static inline const char* nonNull(const char* msg)
{
    if (!msg)
        return "(Unknown error)";
    else
        return msg;
}
template <typename P, void(*freeFunc)(P)>
class AutoFree
{
protected:
    P mPtr;
public:
    AutoFree(P aPtr):mPtr(aPtr){}
    ~AutoFree()
    {
        if (mPtr)
            freeFunc(mPtr);
    }
};



NAN_METHOD(loadDb)
{
//    NanScope();

    if ((args.Length() < 1) || !args[0]->IsString())
        MY_THROW_EXCEP("No country filename specified");
    String::Utf8Value countryname(args[0]->ToString());

    if ((args.Length() < 2) || !args[1]->IsString())
        MY_THROW_EXCEP("No city filename specified");
    String::Utf8Value cityname(args[1]->ToString());

    if ((args.Length() < 3) || !args[2]->IsString())
        MY_THROW_EXCEP("No isp filename specified");
    String::Utf8Value ispname(args[2]->ToString());

    if ((args.Length() < 4) || !args[3]->IsString())
        MY_THROW_EXCEP("No netspeed filename specified");
    String::Utf8Value netspeedname(args[3]->ToString());


    // load the country db
    int status = MMDB_open(*countryname, MMDB_MODE_MMAP, &mmdbCountry);
    // load the city db
    int status2 = MMDB_open(*cityname, MMDB_MODE_MMAP, &mmdbCity);
    // load the isp db
    int status3 = MMDB_open(*ispname, MMDB_MODE_MMAP, &mmdbIsp);
     // load the netspeed db
    int status4 = MMDB_open(*netspeedname, MMDB_MODE_MMAP, &mmdbNetspeed);

    if (status != MMDB_SUCCESS && status2 != MMDB_SUCCESS && status3 != MMDB_SUCCESS  && status4 != MMDB_SUCCESS )
    {
        if (status == MMDB_IO_ERROR && status2 == MMDB_IO_ERROR && status3 == MMDB_IO_ERROR && status4 == MMDB_IO_ERROR)
            MY_THROW_EXCEP((std::string("I/O error opeining file '")+*countryname+"': "+
                            nonNull(strerror(errno))).c_str())
    else
            MY_THROW_EXCEP((std::string("Can't open database file '")+*countryname+"': "+
                            nonNull(MMDB_strerror(status))).c_str());
    }
    loaded = true;
    NanReturnUndefined();
}

NAN_METHOD(lookupIp)
{
    NanEscapableScope();
    if ((args.Length() < 1) || !args[0]->IsString())
        MY_THROW_EXCEP("No ip address specified");
    String::Utf8Value ip(args[0]->ToString());
    if (!*ip)
        MY_THROW_EXCEP("Empty ip address string provided");

    int gai_error, mmdb_error;
    MMDB_lookup_result_s countryResult = MMDB_lookup_string(&mmdbCountry, *ip, &gai_error, &mmdb_error);

    if (gai_error)
        MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")+*ip+
                        nonNull(gai_strerror(gai_error))).c_str());

    if (mmdb_error != MMDB_SUCCESS)
        MY_THROW_EXCEP((std::string("Error looking up ip: ")+nonNull(MMDB_strerror(mmdb_error))).c_str());

    if (!countryResult.found_entry)
        NanReturnNull();


    MMDB_lookup_result_s cityResult = MMDB_lookup_string(&mmdbCity, *ip, &gai_error, &mmdb_error);

    MMDB_lookup_result_s ispResult = MMDB_lookup_string(&mmdbIsp, *ip, &gai_error, &mmdb_error);

    MMDB_lookup_result_s netspeedResult = MMDB_lookup_string(&mmdbNetspeed, *ip, &gai_error, &mmdb_error);


Local<Object> IpData = NanNew<Object>();

//country
MMDB_entry_data_s ccode;
int status = MMDB_get_value(&countryResult.entry, &ccode, "country", "iso_code", NULL);
if (status != MMDB_SUCCESS || !ccode.has_data){
    IpData->Set(NanNew("countryCode"), NanNew("NA"));
    IpData->Set(NanNew("countryName"), NanNew("NA"));
    IpData->Set(NanNew("city"), NanNew("NA"));
    IpData->Set(NanNew("region"), NanNew("NA"));
    IpData->Set(NanNew("provider"), NanNew("NA"));
    IpData->Set(NanNew("netspeed"), NanNew("NA"));
    IpData->Set(NanNew("countryName"), NanNew("NA"));
    NanReturnValue(IpData);
}else{
    if (ccode.type != MMDB_DATA_TYPE_UTF8_STRING)
       MY_THROW_EXCEP("Unexpected data type of result for country code");
MMDB_entry_data_s name;
status = MMDB_get_value(&countryResult.entry, &name, "country", "names", "en", NULL);
if (status != MMDB_SUCCESS || !name.has_data){
    IpData->Set(NanNew("countryName"), NanNew("NA"));
    }
if (name.type != MMDB_DATA_TYPE_UTF8_STRING)
    MY_THROW_EXCEP("Unexpected data type of result for continent");


IpData->Set(NanNew<String>("countryCode"), NanNew<String>(ccode.utf8_string, ccode.data_size));
IpData->Set(NanNew<String>("countryName"), NanNew<String>(name.utf8_string, name.data_size));
}

if (cityResult.found_entry){
    MMDB_entry_data_s city;
    int status = MMDB_get_value(&cityResult.entry, &city, "city", "names", "en", NULL);
    if (status != MMDB_SUCCESS || !city.has_data){
    IpData->Set(NanNew("city"), NanNew("NA"));
    IpData->Set(NanNew("region"), NanNew("NA"));
    }else{
        if (city.type != MMDB_DATA_TYPE_UTF8_STRING)
           MY_THROW_EXCEP("Unexpected data type of result for country code");
        IpData->Set(NanNew<String>("city"), NanNew<String>(city.utf8_string, city.data_size));
    }
    MMDB_entry_data_s region;
    status = MMDB_get_value(&cityResult.entry, &region, "subdivisions","0","iso_code", NULL);
    if (status != MMDB_SUCCESS || !region.has_data){
        IpData->Set(NanNew("region"), NanNew("NA"));
    }else{
        if (region.type != MMDB_DATA_TYPE_UTF8_STRING)
            MY_THROW_EXCEP("Unexpected data type of result for region");
        IpData->Set(NanNew<String>("region"), NanNew<String>(region.utf8_string, region.data_size));
    }

}else{
    IpData->Set(NanNew("city"), NanNew("NA"));
    IpData->Set(NanNew("region"), NanNew("NA"));

}


if (ispResult.found_entry){
MMDB_entry_data_s isp;
int status = MMDB_get_value(&ispResult.entry, &isp, "isp",NULL);
if (status != MMDB_SUCCESS || !isp.has_data){
    IpData->Set(NanNew<String>("provider"), NanNew("NA"));
}else{
    if (isp.type != MMDB_DATA_TYPE_UTF8_STRING)
       MY_THROW_EXCEP("Unexpected data type of result for isp");
    IpData->Set(NanNew<String>("provider"), NanNew<String>(isp.utf8_string, isp.data_size));
}

}else{
IpData->Set(NanNew("provider"), NanNew("NA"));
}


if (netspeedResult.found_entry){
MMDB_entry_data_s netspeed;
int status = MMDB_get_value(&netspeedResult.entry, &netspeed, "connection_type",NULL);
if (status != MMDB_SUCCESS || !netspeed.has_data){
  IpData->Set(NanNew<String>("netspeed"), NanNew("NA"));
  IpData->Set(NanNew("netspeedError"), NanNew("NO Data"));
  }else{
    if (netspeed.type != MMDB_DATA_TYPE_UTF8_STRING)
       MY_THROW_EXCEP("Unexpected data type of result for netspeed");
    IpData->Set(NanNew<String>("netspeed"), NanNew<String>(netspeed.utf8_string, netspeed.data_size));
}
}else{
IpData->Set(NanNew("netspeed"), NanNew("NA"));
IpData->Set(NanNew("netspeedError"), NanNew("Not Found"));
}
NanReturnValue(IpData);

}

NAN_METHOD(unload)
{
    if (!loaded)
        MY_THROW_EXCEP("No database is loaded");
    MMDB_close(&mmdbCountry);
    MMDB_close(&mmdbCity);
    MMDB_close(&mmdbIsp);
    MMDB_close(&mmdbNetspeed);
    loaded = false;
    NanReturnUndefined();
}

void init(Handle<Object> exports)
{
  exports->Set(NanNew<String>("load"), NanNew<FunctionTemplate>(loadDb)->GetFunction());
  exports->Set(NanNew<String>("lookupIp"), NanNew<FunctionTemplate>(lookupIp)->GetFunction());
  exports->Set(NanNew<String>("unload"), NanNew<FunctionTemplate>(unload)->GetFunction());
}


NODE_MODULE(geoip2c, init)
