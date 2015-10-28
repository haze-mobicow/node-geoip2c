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

const unsigned int T_COUNTRY_CODE             = 1 << 0;
const unsigned int T_COUNTRY_CONTINENT        = 1 << 1;
const unsigned int T_COUNTRY_NAME             = 1 << 2;
const unsigned int T_COUNTRY_REGISTRED        = 1 << 3;
const unsigned int T_COUNTRY_ALL              = -1;

const char* LABEL_COUNTRY_CODE                = "countryCode";
const char* LABEL_COUNTRY_CONTINENT           = "countryContinent";
const char* LABEL_COUNTRY_NAME                = "countryName";
const char* LABEL_COUNTRY_REGISTRED           = "countryRegistred";

const unsigned int T_CITY_ZIP                 = 1 << 0;
const unsigned int T_CITY_NAME                = 1 << 1;
const unsigned int T_CITY_TZ                  = 1 << 2;
const unsigned int T_CITY_REGION              = 1 << 3;
const unsigned int T_CITY_METRO               = 1 << 4;
const unsigned int T_CITY_LAT                 = 1 << 5;
const unsigned int T_CITY_LON                 = 1 << 6;
const unsigned int T_CITY_ALL                 = -1;

const char* LABEL_CITY_ZIP                    = "cityZip";
const char* LABEL_CITY_NAME                   = "cityName";
const char* LABEL_CITY_TZ                     = "cityTimeZone";
const char* LABEL_CITY_REGION                 = "cityRegion";
const char* LABEL_CITY_METRO                  = "cityMetro";
const char* LABEL_CITY_LAT                    = "cityLatitude";
const char* LABEL_CITY_LON                    = "cityLongitude";

const unsigned int T_ISP_CODE                 = 1 << 0;
const unsigned int T_ISP_NAME                 = 1 << 1;
const unsigned int T_ISP_NORGANIZATION        = 1 << 2;
const unsigned int T_ISP_CORGANIZATION        = 1 << 3;
const unsigned int T_ISP_ALL                  = -1;

const char* LABEL_ISP_CODE                    = "ispCode";
const char* LABEL_ISP_NAME                    = "ispName";
const char* LABEL_ISP_CORGANIZATION           = "ispCodeOrganization";
const char* LABEL_ISP_NORGANIZATION           = "ispNameOrganization";


const unsigned int T_ANYM_IS_ANONYMOUS        = 1 << 0;
const unsigned int T_ANYM_IS_PUBPROXY         = 1 << 1;
const unsigned int T_ANYM_IS_ANONYMOUS_VPN    = 1 << 2;
const unsigned int T_ANYM_IS_HOSTING_PROVIDER = 1 << 3;
const unsigned int T_ANYM_IS_TOR_EXIT_NODE    = 1 << 4;
const unsigned int T_ANYM_ALL                 = -1;

const char* LABEL_ANYM_IS_ANONYMOUS           = "is_anonymous";
const char* LABEL_ANYM_IS_PUBPROXY            = "is_public_proxy";
const char* LABEL_ANYM_IS_VPN                 = "is_anonymous_vpn";
const char* LABEL_ANYM_IS_HOSTING             = "is_hosting_provider";
const char* LABEL_ANYM_IS_TOR                 = "is_tor_exit_node";

const unsigned int T_NETSPEED_TYPE            = 1 << 0;
const unsigned int T_NETSPEED_ALL             = -1;

const char* LABEL_NETSPEED_TYPE               = "connection_type";

const char* LOOKUP_KEY_COUNTRY                = "country";
const char* LOOKUP_KEY_CITY                   = "city";
const char* LOOKUP_KEY_ISP                    = "isp";
const char* LOOKUP_KEY_NETSPEED               = "netspeed";
const char* LOOKUP_KEY_ANONYMOUS              = "anonymous";

MMDB_s mmdbCountry;
MMDB_s mmdbCity;
MMDB_s mmdbIsp;
MMDB_s mmdbNetspeed;
MMDB_s mmdbAnonymous;
MMDB_s bonoba;

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

#define SET_MMDB_ENTRY(mmdb_entry, Object, key, defval) { \
    if (!mmdb_entry->has_data)\
        Object->Set(NanNew<String>(key), defval); \
    else {\
        if (mmdb_entry->type != MMDB_DATA_TYPE_UTF8_STRING) \
            MY_THROW_EXCEP("Unexpected data type of result for country code"); \
        Object->Set(NanNew<String>(key), \
                    NanNew<String>(mmdb_entry->utf8_string, mmdb_entry->data_size)); \
    }\
}\

#define SET_MMDB_ENTRY_INT16(mmdb_entry, Object, key, defval) { \
    if (!mmdb_entry->has_data)\
        Object->Set(NanNew<String>(key), defval); \
    else {\
        Object->Set(NanNew<String>(key), \
                    NanNew<Number>(mmdb_entry->uint16)); \
    }\
}\

#define SET_MMDB_ENTRY_BOOL(mmdb_entry, Object, key, defval) { \
    if (!mmdb_entry->has_data)\
        Object->Set(NanNew<String>(key), defval); \
    else {\
        Object->Set(NanNew<String>(key), \
                    NanNew<Boolean>(mmdb_entry->boolean)); \
    }\
}\

#define SET_MMDB_ENTRY_DOUBLE(mmdb_entry, Object, key, defval) { \
    if (!mmdb_entry->has_data)\
        Object->Set(NanNew<String>(key), defval); \
    else {\
        Object->Set(NanNew<String>(key), \
                    NanNew<Number>(mmdb_entry->double_value)); \
    }\
}\

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
    AutoFree(P aPtr):mPtr(aPtr) {}
    ~AutoFree()
    {
        if (mPtr)
            freeFunc(mPtr);
    }
};


/* Simple wrapper loads MMDB file
 * Returns status error string, if error. Empty string on success.
 * Status will be used for loadDb JS result object.
 * */
char* loadMMDB(char* fname, MMDB_s* db_s)
{
    char* result = new char[255]();
    int status = MMDB_open(fname, MMDB_MODE_MMAP, db_s);
    if (status != MMDB_SUCCESS)
    {
        if (status == MMDB_IO_ERROR) {
            strcat(result, "I/O error opeining file '");
            strcat(result, fname);
            strcat(result, "' :");
            strcat(result, strerror(errno));
            return result;
        }
        strcat(result, "Can't open database file '");
        strcat(result, fname);
        strcat(result, "' :");
        strcat(result, strerror(errno));
        return result;
    }

    return result;
}

/**
 * Inits module scope DB variables.
 * Returns JavaScript obejct with status error string for each load db key.
 * On success status string empty.
 **/
NAN_METHOD(loadDb)
{
    if (args[0]->IsUndefined())
        MY_THROW_EXCEP("No database filenames set");
    Local<Object> options  = args[0]->ToObject();
    Local<Object> db_stats = NanNew<Object>();
    Local<Value> val;

    // Use Country DB in case if City not set.
    if (val = options->Get(NanNew(LOOKUP_KEY_COUNTRY)), !val->IsUndefined()) {
        String::Utf8Value fname(val->ToString());
        char* tmp = loadMMDB(*fname, &mmdbCity);
        db_stats->Set(NanNew(LOOKUP_KEY_COUNTRY), NanNew<String>(tmp));
    }

    if (val = options->Get(NanNew(LOOKUP_KEY_CITY)), !val->IsUndefined()) {
        String::Utf8Value fname(val->ToString());
        char* tmp = loadMMDB(*fname, &mmdbCity);
        db_stats->Set(NanNew(LOOKUP_KEY_CITY), NanNew<String>(tmp));
    }

    if (val = options->Get(NanNew(LOOKUP_KEY_ISP)), !val->IsUndefined()) {
        String::Utf8Value fname(val->ToString());
        char* tmp = loadMMDB(*fname, &mmdbIsp);
        db_stats->Set(NanNew(LOOKUP_KEY_ISP), NanNew<String>(tmp));
    }

    if (val = options->Get(NanNew(LOOKUP_KEY_NETSPEED)), !val->IsUndefined()) {
        String::Utf8Value fname(val->ToString());
        char* tmp = loadMMDB(*fname, &mmdbNetspeed);
        db_stats->Set(NanNew(LOOKUP_KEY_NETSPEED), NanNew<String>(tmp));
    }

    if (val = options->Get(NanNew(LOOKUP_KEY_ANONYMOUS)), !val->IsUndefined()) {
        String::Utf8Value fname(val->ToString());
        char* tmp = loadMMDB(*fname, &mmdbAnonymous);
        db_stats->Set(NanNew(LOOKUP_KEY_ANONYMOUS), NanNew<String>(tmp));
    }

    NanReturnValue(db_stats);
}

NAN_METHOD(unload)
{
    if (mmdbCountry.filename != NULL)
        MMDB_close(&mmdbCountry);


    if (mmdbCity.filename != NULL)
        MMDB_close(&mmdbCity);

    if (mmdbIsp.filename != NULL)
        MMDB_close(&mmdbIsp);

    if (mmdbNetspeed.filename != NULL)
        MMDB_close(&mmdbNetspeed);

    if (mmdbAnonymous.filename != NULL)
        MMDB_close(&mmdbAnonymous);

    NanReturnUndefined();
}


/*
 * Build and return default lookup options
 * with loockup all types.
 * Enabled Country, City, ISP, Netspeed, Anonymous
 * Returns: Local<Object>
 **/
Local<Object> defaultLoockupOptions() {
    Local<Object> options = NanNew<Object>();
    options->Set(NanNew<String>(LOOKUP_KEY_COUNTRY), NanNew<Uint32>(T_COUNTRY_ALL));
    options->Set(NanNew<String>(LOOKUP_KEY_CITY), NanNew<Uint32>(T_CITY_ALL));
    options->Set(NanNew<String>(LOOKUP_KEY_ISP), NanNew<Uint32>(T_ISP_ALL));
    options->Set(NanNew<String>(LOOKUP_KEY_NETSPEED), NanNew<Uint32>(T_NETSPEED_ALL));
    options->Set(NanNew<String>(LOOKUP_KEY_ANONYMOUS), NanNew<Uint32>(T_ANYM_ALL));

    return options;
}

// arguments ==> [str IP, Dict LookupFlags]
// I think Dict could be omnited -> Get all data
// It's seems we will require default dict values for this case
// TODO: Define function arguments with constants
/* maxminddb nodejs extension wrapper.
 * Returns result JavaScript Object respect to required params.
 * Input lookup arguments:
 *      <ip>, [result_values]
 *
 *      ip - String contains IPv4 or IPv6 address delimited by
 *          "."(IPv4) or ":"(IPv6)
 *      result_value - JavaScript object(Dict).
 *          Key: describes mmdb lookup type (country/city/isp/netspeed).
 *          Value: mask of lookup type fields. (ex: city-> Name, Zip, TimeZone...)
 *      result_value keys and values have associations with module level constants.
 *      Ex keys: LOOKUP_TYPE_COUNTRY, LOOKUP_KEY_NETSPEED....
 *      Ex values for Country: T_COUNTRY_CODE, T_COUNTRY_NAME
 *      All association labels have keywords ends with "_ALL". It's alias for all values.
 *
 *      Method return JavaScript object (Dict) with string key and "string"/"int" values.
 *      Result keys depends from result_value parameter state. Respect with configuration
 *      paras method will return city, country... etc. info.
 *      Example result:
 *      Test IPv6:
 *      IPv6 2001:1388:b47:df34:5ddf:cecb:4213:cf37
 *       Lookup Result ->  { country:
 *       {
 *           countryCode: 'PE',
 *           countryName: 'Peru',
 *           countryContinent: 'South America',
 *           countryRegistred: 'Peru'
 *       },
 *       city:
 *       {
 *           cityZip: 'NA',
 *           cityName: 'Lima',
 *           cityTimeZone: 'America/Lima',
 *           cityMetro: 'NA'
 *       },
 *       isp:
 *       {
 *           ispCode: 'NA',
 *           ispName: 'Telefonica del Peru',
 *           ispNameOrganization: 'Telefonica del Peru',
 *           ispCodeOrganization: 'NA'
 *       },
 *       netspeed: { connection_type: 'Cable/DSL' },
 *       anonymous:
 *       {
 *           is_anonymous: 'NA',
 *           is_public_proxy: 'NA',
 *           is_anonymous_vpn: 'NA',
 *           is_hosting_provider: 'NA',
 *           is_tor_exit_node: 'NA'
 *       } }
 *
 **/
NAN_METHOD(lookupIp)
{
    MMDB_lookup_result_s mmdb_result;                // MMDB loockup result value
    MMDB_entry_data_s *mmdb_entry;                   // MMDB node value record from result
    Local<Value> valA, valB;                         // Temp JS Value container
    Local<Object> IpData = NanNew<Object>();         // Result Container
    Local<Value> default_val = NanNew<String>("NA");
    int status, gai_error, mmdb_error;               // MMDB status flags
    Local<String> keyA, keyB;
    Local<Object>lookupOptions;                      // Lookup Function Argument2

    // Result build MASK
    unsigned int t_ctry   = T_COUNTRY_ALL;
    unsigned int t_city   = T_CITY_ALL;
    unsigned int t_isp    = T_ISP_ALL;
    unsigned int t_net    = T_NETSPEED_ALL;
    unsigned int t_anonym = T_ANYM_ALL;

    NanEscapableScope();
    if ((args.Length() < 1) || !args[0]->IsString())
        MY_THROW_EXCEP("No ip address specified");
    String::Utf8Value ip(args[0]->ToString());

    if (args[1]->IsUndefined()) {
        lookupOptions = defaultLoockupOptions();
    } else {
        lookupOptions = args[1]->ToObject();
    }

    keyA = NanNew<String>(LOOKUP_KEY_COUNTRY);
    keyB = NanNew<String>(LOOKUP_KEY_CITY);
    valA = lookupOptions->Get(keyA);
    valB = lookupOptions->Get(keyB);

    // Lookup IP block for City OR Country
    if (mmdbCity.filename != NULL && (!valA->IsUndefined() || !valB->IsUndefined()))
    {
        mmdb_result = MMDB_lookup_string(&mmdbCity, *ip, &gai_error, &mmdb_error);

        if (gai_error)
            MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")
                        + *ip + nonNull(gai_strerror(gai_error))).c_str());

        if (mmdb_error != MMDB_SUCCESS)
            MY_THROW_EXCEP((std::string("Error looking up ip: ")
                        + nonNull(MMDB_strerror(mmdb_error))).c_str());

        if (!mmdb_result.found_entry)
            NanReturnValue(IpData); // Return empty Object
    }

    // -- Collect Country Info -----------------------------------------------
    if (mmdbCity.filename != NULL && !valA->IsUndefined())
    {
        t_ctry = valA->Uint32Value();

        Local<Object> CountryData = NanNew<Object>();   // Counter Container
        if (t_ctry & T_COUNTRY_CODE)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "country", "iso_code", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CountryData, LABEL_COUNTRY_CODE, default_val);
        }

        // Country name
        if (t_ctry & T_COUNTRY_NAME)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "country", "names", "en", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CountryData, LABEL_COUNTRY_NAME, default_val);
        }

        if (t_ctry & T_COUNTRY_CONTINENT)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "continent", "names", "en", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CountryData, LABEL_COUNTRY_CONTINENT, default_val);
        }

        // Country Registred
        if (t_ctry & T_COUNTRY_REGISTRED)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "registered_country", "names", "en", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CountryData, LABEL_COUNTRY_REGISTRED, default_val);
        }

        IpData->Set(keyA, CountryData);
    }

    // -- Collect City Info --------------------------------------------------
    if (mmdbCity.filename != NULL && !valB->IsUndefined())
    {
        t_city = valB->Uint32Value();
        Local<Object> CityData = NanNew<Object>();   // City Container

        if (t_city & T_CITY_ZIP)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry,  "postal", "code", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CityData, LABEL_CITY_ZIP, default_val);
        }

        if (t_city & T_CITY_NAME)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "city", "names", "en", NULL);
            SET_MMDB_ENTRY(mmdb_entry, CityData, LABEL_CITY_NAME, default_val);
        }

        if (t_city & T_CITY_TZ)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "location", "time_zone",  NULL);
            SET_MMDB_ENTRY(mmdb_entry, CityData, LABEL_CITY_TZ, default_val);
        }

        if (t_city & T_CITY_REGION)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "subdivisions", "0", "iso_code",  NULL);
            SET_MMDB_ENTRY(mmdb_entry, CityData, LABEL_CITY_REGION, default_val);
        }

        if (t_city & T_CITY_METRO)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "location", "metro_code",  NULL);
            SET_MMDB_ENTRY_INT16(mmdb_entry, CityData, LABEL_CITY_METRO, default_val);
        }

        if (t_city & T_CITY_LAT)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "location", "latitude",  NULL);
            SET_MMDB_ENTRY_DOUBLE(mmdb_entry, CityData, LABEL_CITY_LAT, default_val);
        }

        if (t_city & T_CITY_LON)
        {
            mmdb_entry = new MMDB_entry_data_s;
            status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "location", "longitude",  NULL);
            SET_MMDB_ENTRY_DOUBLE(mmdb_entry, CityData, LABEL_CITY_LON, default_val);
        }

        IpData->Set(keyB, CityData);
    }

    // -- Collect ISP Info ----------------------------------------------------
    keyA = NanNew<String>(LOOKUP_KEY_ISP);
    valA = lookupOptions->Get(keyA);
    if (mmdbIsp.filename != NULL && !valA->IsUndefined())
    {
        mmdb_result = MMDB_lookup_string(&mmdbIsp, *ip, &gai_error, &mmdb_error);

        if (gai_error)
            MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")
                        + *ip + nonNull(gai_strerror(gai_error))).c_str());

        if (mmdb_error != MMDB_SUCCESS)
            MY_THROW_EXCEP((std::string("Error looking up ip: ")
                        + nonNull(MMDB_strerror(mmdb_error))).c_str());

        if (mmdb_result.found_entry) {
            t_isp = valA->Uint32Value();
            Local<Object> IspData = NanNew<Object>();   // City Container

            if (t_isp & T_ISP_CODE)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "autonomous_system_number",  NULL);
                SET_MMDB_ENTRY_INT16(mmdb_entry, IspData, LABEL_ISP_CODE, default_val);
            }

            if (t_isp & T_ISP_NAME)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "isp",  NULL);
                SET_MMDB_ENTRY(mmdb_entry, IspData, LABEL_ISP_NAME, default_val);
            }

            if (t_isp & T_ISP_NORGANIZATION)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "organization",  NULL);
                SET_MMDB_ENTRY(mmdb_entry, IspData, LABEL_ISP_NORGANIZATION, default_val);
            }

            if (t_isp & T_ISP_CORGANIZATION)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "autonomous_system_organization",  NULL);
                SET_MMDB_ENTRY(mmdb_entry, IspData, LABEL_ISP_CORGANIZATION, default_val);
            }
            IpData->Set(keyA, IspData);
        }

    }

    // -- Anonymous IPs Info --------------------------------------------------
    keyA = NanNew<String>(LOOKUP_KEY_NETSPEED);
    valA = lookupOptions->Get(keyA);
    if (mmdbNetspeed.filename != NULL && !valA->IsUndefined())
    {
        mmdb_result = MMDB_lookup_string(&mmdbNetspeed, *ip, &gai_error, &mmdb_error);

        if (gai_error)
            MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")
                        + *ip + nonNull(gai_strerror(gai_error))).c_str());

        if (mmdb_error != MMDB_SUCCESS)
            MY_THROW_EXCEP((std::string("Error looking up ip: ")
                        + nonNull(MMDB_strerror(mmdb_error))).c_str());

        if (mmdb_result.found_entry) {
            t_net = valA->Uint32Value();
            Local<Object> NetData = NanNew<Object>();   // Anonymous Container

            if (t_net & T_NETSPEED_TYPE)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "connection_type",  NULL);
                SET_MMDB_ENTRY(mmdb_entry, NetData, LABEL_NETSPEED_TYPE, default_val);
            }

            IpData->Set(keyA, NetData);
        }
    }

    // -- Anonymous IPs Info --------------------------------------------------
    keyA = NanNew<String>(LOOKUP_KEY_ANONYMOUS);
    valA = lookupOptions->Get(keyA);
    if (mmdbAnonymous.filename != NULL && !lookupOptions->Get(keyA)->IsUndefined())
    {
        // TODO: Put into macros
        mmdb_result = MMDB_lookup_string(&mmdbAnonymous, *ip, &gai_error, &mmdb_error);
        if (gai_error)
            MY_THROW_EXCEP((std::string("Error from getaddrinfo for ")
                        + *ip + nonNull(gai_strerror(gai_error))).c_str());
        if (mmdb_error != MMDB_SUCCESS)
            MY_THROW_EXCEP((std::string("Error looking up ip: ")
                        + nonNull(MMDB_strerror(mmdb_error))).c_str());

        if (mmdb_result.found_entry) {
            t_anonym = valA->Uint32Value();
            Local<Object> AnymData = NanNew<Object>();   // Anonymous Container

            if (t_anonym & T_ANYM_IS_ANONYMOUS)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "is_anonymous",  NULL);
                SET_MMDB_ENTRY_BOOL(mmdb_entry, AnymData, LABEL_ANYM_IS_ANONYMOUS, default_val);
            }

            if (t_anonym & T_ANYM_IS_PUBPROXY)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "is_public_proxy",  NULL);
                SET_MMDB_ENTRY_BOOL(mmdb_entry, AnymData, LABEL_ANYM_IS_PUBPROXY, default_val);
            }

            if (t_anonym & T_ANYM_IS_ANONYMOUS_VPN)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "is_anonymous_vpn",  NULL);
                SET_MMDB_ENTRY_BOOL(mmdb_entry, AnymData, LABEL_ANYM_IS_VPN, default_val);
            }

            if (t_anonym & T_ANYM_IS_HOSTING_PROVIDER)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "is_hosting_provider",  NULL);
                SET_MMDB_ENTRY_BOOL(mmdb_entry, AnymData, LABEL_ANYM_IS_HOSTING, default_val);
            }

            if (t_anonym & T_ANYM_IS_TOR_EXIT_NODE)
            {
                mmdb_entry = new MMDB_entry_data_s;
                status = MMDB_get_value(&mmdb_result.entry, mmdb_entry, "is_tor_exit_node",  NULL);
                SET_MMDB_ENTRY_BOOL(mmdb_entry, AnymData, LABEL_ANYM_IS_TOR, default_val);
            }

            IpData->Set(keyA, AnymData);
        }
    }

    NanReturnValue(IpData);
}

void init(Handle<Object> exports, Handle<Object> module)
{
    exports->Set(NanNew<String>("load"), NanNew<FunctionTemplate>(loadDb)->GetFunction());
    exports->Set(NanNew<String>("lookupIp"), NanNew<FunctionTemplate>(lookupIp)->GetFunction());
    exports->Set(NanNew<String>("unload"), NanNew<FunctionTemplate>(unload)->GetFunction());

    exports->Set(NanNew<String>("LOOKUP_KEY_COUNTRY"), NanNew<String>(LOOKUP_KEY_COUNTRY));
    exports->Set(NanNew<String>("LOOKUP_KEY_CITY"), NanNew<String>(LOOKUP_KEY_CITY));
    exports->Set(NanNew<String>("LOOKUP_KEY_ISP"), NanNew<String>(LOOKUP_KEY_ISP));
    exports->Set(NanNew<String>("LOOKUP_KEY_NETSPEED"), NanNew<String>(LOOKUP_KEY_NETSPEED));
    exports->Set(NanNew<String>("LOOKUP_KEY_ANONYMOUS"), NanNew<String>(LOOKUP_KEY_ANONYMOUS));

    exports->Set(NanNew<String>("LABEL_COUNTRY_NAME"), NanNew<String>(LABEL_COUNTRY_NAME));
    exports->Set(NanNew<String>("LABEL_COUNTRY_CODE"), NanNew<String>(LABEL_COUNTRY_CODE));
    exports->Set(NanNew<String>("LABEL_COUNTRY_CONTINENT"), NanNew<String>(LABEL_COUNTRY_CONTINENT));
    exports->Set(NanNew<String>("LABEL_COUNTRY_REGISTRED"), NanNew<String>(LABEL_COUNTRY_REGISTRED));
    exports->Set(NanNew<String>("LABEL_CITY_ZIP"), NanNew<String>(LABEL_CITY_ZIP));
    exports->Set(NanNew<String>("LABEL_CITY_NAME"), NanNew<String>(LABEL_CITY_NAME));
    exports->Set(NanNew<String>("LABEL_CITY_TZ"), NanNew<String>(LABEL_CITY_TZ));
    exports->Set(NanNew<String>("LABEL_CITY_REGION"), NanNew<String>(LABEL_CITY_REGION));
    exports->Set(NanNew<String>("LABEL_CITY_METRO"), NanNew<String>(LABEL_CITY_METRO));
    exports->Set(NanNew<String>("LABEL_CITY_LAT"), NanNew<String>(LABEL_CITY_LAT));
    exports->Set(NanNew<String>("LABEL_CITY_LON"), NanNew<String>(LABEL_CITY_LON));
    exports->Set(NanNew<String>("LABEL_ISP_CODE"), NanNew<String>(LABEL_ISP_CODE));
    exports->Set(NanNew<String>("LABEL_ISP_NAME"), NanNew<String>(LABEL_ISP_NAME));
    exports->Set(NanNew<String>("LABEL_ISP_NORGANIZATION"), NanNew<String>(LABEL_ISP_NORGANIZATION));
    exports->Set(NanNew<String>("LABEL_ISP_CORGANIZATION"), NanNew<String>(LABEL_ISP_CORGANIZATION));
    exports->Set(NanNew<String>("LABEL_ANYM_IS_ANONYMOUS"), NanNew<String>(LABEL_ANYM_IS_ANONYMOUS));
    exports->Set(NanNew<String>("LABEL_ANYM_IS_PUBPROXY"), NanNew<String>(LABEL_ANYM_IS_PUBPROXY));
    exports->Set(NanNew<String>("LABEL_ANYM_IS_VPN"), NanNew<String>(LABEL_ANYM_IS_VPN));
    exports->Set(NanNew<String>("LABEL_ANYM_IS_HOSTING"), NanNew<String>(LABEL_ANYM_IS_HOSTING));
    exports->Set(NanNew<String>("LABEL_ANYM_IS_TOR"), NanNew<String>(LABEL_ANYM_IS_TOR));
    exports->Set(NanNew<String>("LABEL_NETSPEED_TYPE"), NanNew<String>(LABEL_NETSPEED_TYPE));

    exports->Set(NanNew<String>("T_COUNTRY_CODE"), NanNew<Uint32>(T_COUNTRY_CODE));
    exports->Set(NanNew<String>("T_COUNTRY_CONTINENT"), NanNew<Uint32>(T_COUNTRY_CONTINENT));
    exports->Set(NanNew<String>("T_COUNTRY_NAME"), NanNew<Uint32>(T_COUNTRY_NAME));
    exports->Set(NanNew<String>("T_COUNTRY_REGISTRED"), NanNew<Uint32>(T_COUNTRY_REGISTRED));
    exports->Set(NanNew<String>("T_COUNTRY_ALL"), NanNew<Uint32>(T_COUNTRY_ALL));

    exports->Set(NanNew<String>("T_CITY_ZIP"), NanNew<Uint32>(T_CITY_ZIP));
    exports->Set(NanNew<String>("T_CITY_NAME"), NanNew<Uint32>(T_CITY_NAME));
    exports->Set(NanNew<String>("T_CITY_TZ"), NanNew<Uint32>(T_CITY_TZ));
    exports->Set(NanNew<String>("T_CITY_ALL"), NanNew<Uint32>(T_CITY_ALL));

    exports->Set(NanNew<String>("T_ISP_CODE"), NanNew<Uint32>(T_ISP_CODE));
    exports->Set(NanNew<String>("T_ISP_NAME"), NanNew<Uint32>(T_ISP_NAME));
    exports->Set(NanNew<String>("T_ISP_CORGANIZATION"), NanNew<Uint32>(T_ISP_CORGANIZATION));
    exports->Set(NanNew<String>("T_ISP_NORGANIZATION"), NanNew<Uint32>(T_ISP_NORGANIZATION));
    exports->Set(NanNew<String>("T_ISP_ALL"), NanNew<Uint32>(T_ISP_ALL));

    exports->Set(NanNew<String>("T_NETSPEED_TYPE"), NanNew<Uint32>(T_NETSPEED_TYPE ));
    exports->Set(NanNew<String>("T_NETSPEED_ALL"), NanNew<Uint32>(T_NETSPEED_ALL));

    exports->Set(NanNew<String>("T_ANYM_IS_ANONYMOUS"), NanNew<Uint32>(T_ANYM_IS_ANONYMOUS_VPN));
    exports->Set(NanNew<String>("T_ANYM_IS_PUBPROXY"), NanNew<Uint32>(T_ANYM_IS_PUBPROXY));
    exports->Set(NanNew<String>("T_ANYM_IS_PUBPROXY"), NanNew<Uint32>(T_ANYM_IS_ANONYMOUS_VPN));
    exports->Set(NanNew<String>("T_ANYM_IS_HOSTING_PROVIDER"), NanNew<Uint32>(T_ANYM_IS_HOSTING_PROVIDER));
    exports->Set(NanNew<String>("T_ANYM_IS_TOR_EXIT_NODE"), NanNew<Uint32>(T_ANYM_IS_TOR_EXIT_NODE));
    exports->Set(NanNew<String>("T_ANYM_ALL"), NanNew<Uint32>(T_ANYM_ALL));
}

NODE_MODULE(geoip2c, init)

