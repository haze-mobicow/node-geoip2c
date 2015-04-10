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
MMDB_s mmdb;
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
        MY_THROW_EXCEP("No filename specified");
    String::Utf8Value fname(args[0]->ToString());

    if ((args.Length() < 2) || !args[1]->IsString())
        MY_THROW_EXCEP("No type specified");
    String::Utf8Value ndbType(args[1]->ToString());

    int status = MMDB_open(*fname, MMDB_MODE_MMAP, &mmdb);

    if (status != MMDB_SUCCESS)
    {
        if (status == MMDB_IO_ERROR)
            MY_THROW_EXCEP((std::string("I/O error opeining file '")+*fname+"': "+
                            nonNull(strerror(errno))).c_str())
    else
            MY_THROW_EXCEP((std::string("Can't open database file '")+*fname+"': "+
                            nonNull(MMDB_strerror(status))).c_str());
    }
        std::string dbTest = std::string(*ndbType);
        if(dbTest =="country"){
        dbType = 1;
        }
        if(dbTest =="city"){
        dbType = 2;
        }
        if(dbTest =="isp"){
        dbType = 3;
        }
        if(dbTest =="netspeed"){
        dbType = 4;
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
    MMDB_lookup_result_s result = MMDB_lookup_string(&mmdb, *ip, &gai_error, &mmdb_error);

    if (gai_error)
        MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")+*ip+
                        nonNull(gai_strerror(gai_error))).c_str());

    if (mmdb_error != MMDB_SUCCESS)
        MY_THROW_EXCEP((std::string("Error looking up ip: ")+nonNull(MMDB_strerror(mmdb_error))).c_str());

    if (!result.found_entry)
        NanReturnNull();

    // add switch for DB type

        switch (dbType) {
            case 1:{
                Local<Object> countryInfo = NanNew<Object>();
                MMDB_entry_data_s ccode;
                int status = MMDB_get_value(&result.entry, &ccode, "country", "iso_code", NULL);
                if (status != MMDB_SUCCESS || !ccode.has_data){
                    countryInfo->Set(NanNew("countryCode"), NanNew("NA"));
                    countryInfo->Set(NanNew("countryName"), NanNew("NA"));
                    NanReturnValue(countryInfo);
                }else{
                    if (ccode.type != MMDB_DATA_TYPE_UTF8_STRING)
                       MY_THROW_EXCEP("Unexpected data type of result for country code");
                MMDB_entry_data_s name;
                status = MMDB_get_value(&result.entry, &name, "country", "names", "en", NULL);
                if (status != MMDB_SUCCESS || !name.has_data){
                    countryInfo->Set(NanNew("countryName"), NanNew("NA"));
                     NanReturnValue(countryInfo);
                    }
                if (name.type != MMDB_DATA_TYPE_UTF8_STRING)
                    MY_THROW_EXCEP("Unexpected data type of result for continent");

                countryInfo->Set(NanNew<String>("countryCode"), NanNew<String>(ccode.utf8_string, ccode.data_size));
                countryInfo->Set(NanNew<String>("countryName"), NanNew<String>(name.utf8_string, name.data_size));
                NanReturnValue(countryInfo);
                }
                }
                break;
            case 2:{
                Local<Object> cityInfo = NanNew<Object>();
                MMDB_entry_data_s city;
                int status = MMDB_get_value(&result.entry, &city, "city", "names", "en", NULL);
                if (status != MMDB_SUCCESS || !city.has_data){
                cityInfo->Set(NanNew("city"), NanNew("NA"));
                cityInfo->Set(NanNew("region"), NanNew("NA"));
                NanReturnValue(cityInfo);
                }else{
                    if (city.type != MMDB_DATA_TYPE_UTF8_STRING)
                       MY_THROW_EXCEP("Unexpected data type of result for country code");
                    cityInfo->Set(NanNew<String>("city"), NanNew<String>(city.utf8_string, city.data_size));
                }
                MMDB_entry_data_s region;
                status = MMDB_get_value(&result.entry, &region, "subdivisions","0","iso_code", NULL);
                if (status != MMDB_SUCCESS || !region.has_data){
                    cityInfo->Set(NanNew("region"), NanNew("NA"));
                    NanReturnValue(cityInfo);
                }else{
                    if (region.type != MMDB_DATA_TYPE_UTF8_STRING)
                        MY_THROW_EXCEP("Unexpected data type of result for region");
                    cityInfo->Set(NanNew<String>("region"), NanNew<String>(region.utf8_string, region.data_size));
                }
                NanReturnValue(cityInfo);
                }
                break;

            case 3:{
                Local<Object> providerInfo = NanNew<Object>();
                MMDB_entry_data_s isp;
                int status = MMDB_get_value(&result.entry, &isp, "isp",NULL);
                if (status != MMDB_SUCCESS || !isp.has_data){
                    providerInfo->Set(NanNew<String>("provider"), NanNew("NA"));
                }else{
                    if (isp.type != MMDB_DATA_TYPE_UTF8_STRING)
                       MY_THROW_EXCEP("Unexpected data type of result for isp");
                    providerInfo->Set(NanNew<String>("provider"), NanNew<String>(isp.utf8_string, isp.data_size));
                }
                NanReturnValue(providerInfo);
                }
                break;

                case 4:{
                    Local<Object> netInfo = NanNew<Object>();
                    MMDB_entry_data_s netspeed;
                    int status = MMDB_get_value(&result.entry, &netspeed, "connection_type",NULL);
                    if (status != MMDB_SUCCESS || !netspeed.has_data){
                      netInfo->Set(NanNew<String>("netspeed"), NanNew("NA"));
                      }else{
                        if (netspeed.type != MMDB_DATA_TYPE_UTF8_STRING)
                           MY_THROW_EXCEP("Unexpected data type of result for netspeed");
                        netInfo->Set(NanNew<String>("netspeed"), NanNew<String>(netspeed.utf8_string, netspeed.data_size));
                    }
                    NanReturnValue(netInfo);
                    }
                    break;

            default:{
                MY_THROW_EXCEP("switch Error");
                }
                break;
        }
// end of switch

}

NAN_METHOD(unload)
{
    if (!loaded)
        MY_THROW_EXCEP("No database is loaded");
    MMDB_close(&mmdb);
    loaded = false;
    NanReturnUndefined();
}

void init(Handle<Object> exports)
{
  exports->Set(NanNew<String>("load"), NanNew<FunctionTemplate>(loadDb)->GetFunction());
  exports->Set(NanNew<String>("lookupIp"), NanNew<FunctionTemplate>(lookupIp)->GetFunction());
  exports->Set(NanNew<String>("unload"), NanNew<FunctionTemplate>(unload)->GetFunction());
}

void Initialize (Handle<Object> exports);

void init(Handle<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
}

NODE_MODULE(geoip2c, Initialize)
