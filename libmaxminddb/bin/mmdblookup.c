#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include "maxminddb.h"
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <malloc.h>
#define snprintf _snprintf
#undef UNICODE /* Use the non-UTF16 version of the gai_strerror */
#else
#include <libgen.h>
#include <unistd.h>
#endif

#define LOCAL

/* *INDENT-OFF* */
/* --prototypes automatically generated by dev-bin/regen-prototypes.pl - don't remove this comment */
LOCAL void usage(char *program, int exit_code, const char *error);
LOCAL const char **get_options(int argc, char **argv, char **mmdb_file,
                               char **ip_address, int *verbose, int *iterations,
                               int *lookup_path_length);
LOCAL MMDB_s open_or_die(const char *fname);
LOCAL void dump_meta(MMDB_s *mmdb);
LOCAL int lookup_and_print(MMDB_s *mmdb, const char *ip_address,
                           const char **lookup_path,
                           int lookup_path_length);
LOCAL int benchmark(MMDB_s *mmdb, int iterations);
LOCAL MMDB_lookup_result_s lookup_or_die(MMDB_s *mmdb, const char *ipstr);
LOCAL void random_ipv4(char *ip);
/* --prototypes end - don't remove this comment-- */
/* *INDENT-ON* */

int main(int argc, char **argv)
{
    char *mmdb_file = NULL;
    char *ip_address = NULL;
    int verbose = 0;
    int iterations = 0;
    int lookup_path_length = 0;

    const char **lookup_path =
        get_options(argc, argv, &mmdb_file, &ip_address, &verbose, &iterations,
                    &lookup_path_length);

    MMDB_s mmdb = open_or_die(mmdb_file);

    if (verbose) {
        dump_meta(&mmdb);
    }

    if (0 == iterations) {
        exit(lookup_and_print(&mmdb, ip_address, lookup_path,
                              lookup_path_length));
    } else {
        exit(benchmark(&mmdb, iterations));
    }
}

LOCAL void usage(char *program, int exit_code, const char *error)
{
    if (NULL != error) {
        fprintf(stderr, "\n  *ERROR: %s\n", error);
    }

    char *usage = "\n"
                  "  %s --file /path/to/file.mmdb --ip 1.2.3.4 [path to lookup]\n"
                  "\n"
                  "  This application accepts the following options:\n"
                  "\n"
                  "      --file (-f)     The path to the MMDB file. Required.\n"
                  "\n"
                  "      --ip (-i)       The IP address to look up. Required.\n"
                  "\n"
                  "      --verbose (-v)  Turns on verbose output. Specifically, this causes this\n"
                  "                      application to output the database metadata.\n"
                  "\n"
                  "      --version       Print the program's version number and exit.\n"
                  "\n"
                  "      --help (-h -?)  Show usage information.\n"
                  "\n"
                  "  If an IP's data entry resolves to a map or array, you can provide\n"
                  "  a lookup path to only show part of that data.\n"
                  "\n"
                  "  For example, given a JSON structure like this:\n"
                  "\n"
                  "    {\n"
                  "        \"names\": {\n"
                  "             \"en\": \"Germany\",\n"
                  "             \"de\": \"Deutschland\"\n"
                  "        },\n"
                  "        \"cities\": [ \"Berlin\", \"Frankfurt\" ]\n"
                  "    }\n"
                  "\n"
                  "  You could look up just the English name by calling mmdblookup with a lookup path of:\n"
                  "\n"
                  "    mmdblookup --file ... --ip ... names en\n"
                  "\n"
                  "  Or you could look up the second city in the list with:\n"
                  "\n"
                  "    mmdblookup --file ... --ip ... cities 1\n"
                  "\n"
                  "  Array numbering begins with zero (0).\n"
                  "\n"
                  "  If you do not provide a path to lookup, all of the information for a given IP\n"
                  "  will be shown.\n"
                  "\n";

    fprintf(stdout, usage, program);
    exit(exit_code);
}

LOCAL const char **get_options(int argc, char **argv, char **mmdb_file,
                               char **ip_address, int *verbose, int *iterations,
                               int *lookup_path_length)
{
    static int help = 0;
    static int version = 0;

    while (1) {
        static struct option options[] = {
            { "file",      required_argument, 0, 'f' },
            { "ip",        required_argument, 0, 'i' },
            { "verbose",   no_argument,       0, 'v' },
            { "version",   no_argument,       0, 'n' },
            { "benchmark", required_argument, 0, 'b' },
            { "help",      no_argument,       0, 'h' },
            { "?",         no_argument,       0, 1   },
            { 0,           0,                 0, 0   }
        };

        int opt_index;
        int opt_char = getopt_long(argc, argv, "f:i:b:vnh?", options,
                                   &opt_index);

        if (-1 == opt_char) {
            break;
        }

        if ('f' == opt_char) {
            *mmdb_file = optarg;
        } else if ('i' == opt_char) {
            *ip_address = optarg;
        } else if ('v' == opt_char) {
            *verbose = 1;
        } else if ('n' == opt_char) {
            version = 1;
        } else if ('b' == opt_char) {
            *iterations = strtol(optarg, NULL, 10);
        } else if ('h' == opt_char || '?' == opt_char) {
            help = 1;
        }
    }

#ifdef _WIN32
    char *program = alloca(strlen(argv[0]));
    _splitpath(argv[0], NULL, NULL, program, NULL);
    _splitpath(argv[0], NULL, NULL, NULL, program + strlen(program));
#else
    char *program = basename(argv[0]);
#endif

    if (help) {
        usage(program, 0, NULL);
    }

    if (version) {
        fprintf(stdout, "\n  %s version %s\n\n", program, VERSION);
        exit(0);
    }

    if (NULL == *mmdb_file) {
        usage(program, 1, "You must provide a filename with --file");
    }

    if (NULL == *ip_address && *iterations == 0) {
        usage(program, 1, "You must provide an IP address with --ip");
    }

    const char **lookup_path =
        malloc(sizeof(const char *) * ((argc - optind) + 1));
    int i;
    for (i = 0; i < argc - optind; i++) {
        lookup_path[i] = argv[i + optind];
        (*lookup_path_length)++;
    }
    lookup_path[i] = NULL;

    return lookup_path;
}

LOCAL MMDB_s open_or_die(const char *fname)
{
    MMDB_s mmdb;
    int status = MMDB_open(fname, MMDB_MODE_MMAP, &mmdb);

    if (MMDB_SUCCESS != status) {
        fprintf(stderr, "\n  Can't open %s - %s\n", fname,
                MMDB_strerror(status));

        if (MMDB_IO_ERROR == status) {
            fprintf(stderr, "    IO error: %s\n", strerror(errno));
        }

        fprintf(stderr, "\n");

        exit(2);
    }

    return mmdb;
}

LOCAL void dump_meta(MMDB_s *mmdb)
{
    const char *meta_dump = "\n"
                            "  Database metadata\n"
                            "    Node count:    %i\n"
                            "    Record size:   %i bits\n"
                            "    IP version:    IPv%i\n"
                            "    Binary format: %i.%i\n"
                            "    Build epoch:   %llu (%s)\n"
                            "    Type:          %s\n"
                            "    Languages:     ";

    char date[40];
    strftime(date, 40, "%F %T UTC",
             gmtime((const time_t *)&mmdb->metadata.build_epoch));

    fprintf(stdout, meta_dump,
            mmdb->metadata.node_count,
            mmdb->metadata.record_size,
            mmdb->metadata.ip_version,
            mmdb->metadata.binary_format_major_version,
            mmdb->metadata.binary_format_minor_version,
            mmdb->metadata.build_epoch,
            date,
            mmdb->metadata.database_type);

    for (size_t i = 0; i < mmdb->metadata.languages.count; i++) {
        fprintf(stdout, "%s", mmdb->metadata.languages.names[i]);
        if (i < mmdb->metadata.languages.count - 1) {
            fprintf(stdout, " ");
        }
    }
    fprintf(stdout, "\n");

    fprintf(stdout, "    Description:\n");
    for (size_t i = 0; i < mmdb->metadata.description.count; i++) {
        fprintf(stdout, "      %s:   %s\n",
                mmdb->metadata.description.descriptions[i]->language,
                mmdb->metadata.description.descriptions[i]->description);
    }
    fprintf(stdout, "\n");
}

LOCAL int lookup_and_print(MMDB_s *mmdb, const char *ip_address,
                           const char **lookup_path,
                           int lookup_path_length)
{

    MMDB_lookup_result_s result = lookup_or_die(mmdb, ip_address);
    MMDB_entry_data_list_s *entry_data_list = NULL;

    int exit_code = 0;

    if (result.found_entry) {
        int status;
        if (lookup_path_length) {
            MMDB_entry_data_s entry_data;
            status = MMDB_aget_value(&result.entry, &entry_data,
                                     lookup_path);
            if (MMDB_SUCCESS == status) {
                if (entry_data.offset) {
                    MMDB_entry_s entry =
                    { .mmdb = mmdb, .offset = entry_data.offset };
                    status = MMDB_get_entry_data_list(&entry,
                                                      &entry_data_list);
                } else {
                    fprintf(
                        stdout,
                        "\n  No data was found at the lookup path you provided\n\n");
                }
            }
        } else {
            status = MMDB_get_entry_data_list(&result.entry,
                                              &entry_data_list);
        }

        if (MMDB_SUCCESS != status) {
            fprintf(stderr, "Got an error looking up the entry data - %s\n",
                    MMDB_strerror(status));
            exit_code = 5;
            goto end;
        }

        if (NULL != entry_data_list) {
            fprintf(stdout, "\n");
            MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
            fprintf(stdout, "\n");
        }
    } else {
        fprintf(stderr,
                "\n  Could not find an entry for this IP address (%s)\n\n",
                ip_address);
        exit_code = 6;
    }

 end:
    MMDB_free_entry_data_list(entry_data_list);
    MMDB_close(mmdb);
    free(lookup_path);

    return exit_code;
}

LOCAL int benchmark(MMDB_s *mmdb, int iterations)
{
    char ip_address[16];
    int exit_code = 0;

    srand( time(NULL) );
    clock_t time = clock();

    for (int i = 0; i < iterations; i++) {
        random_ipv4(ip_address);

        MMDB_lookup_result_s result = lookup_or_die(mmdb, ip_address);
        MMDB_entry_data_list_s *entry_data_list = NULL;

        if (result.found_entry) {

            int status = MMDB_get_entry_data_list(&result.entry,
                                                  &entry_data_list);

            if (MMDB_SUCCESS != status) {
                fprintf(stderr, "Got an error looking up the entry data - %s\n",
                        MMDB_strerror(status));
                exit_code = 5;
                MMDB_free_entry_data_list(entry_data_list);
                goto end;
            }
        }

        MMDB_free_entry_data_list(entry_data_list);
    }

    time = clock() - time;
    double seconds = ((double)time / CLOCKS_PER_SEC);
    fprintf(
        stdout,
        "\n  Looked up %i addresses in %.2f seconds. %.2f lookups per second.\n\n",
        iterations, seconds, iterations / seconds);

 end:
    MMDB_close(mmdb);

    return exit_code;
}

LOCAL MMDB_lookup_result_s lookup_or_die(MMDB_s *mmdb, const char *ipstr)
{
    int gai_error, mmdb_error;
    MMDB_lookup_result_s result =
        MMDB_lookup_string(mmdb, ipstr, &gai_error, &mmdb_error);

    if (0 != gai_error) {
        fprintf(stderr,
                "\n  Error from call to getaddrinfo for %s - %s\n\n",
                ipstr, gai_strerror(gai_error));
        exit(3);
    }

    if (MMDB_SUCCESS != mmdb_error) {
        fprintf(stderr, "\n  Got an error from the maxminddb library: %s\n\n",
                MMDB_strerror(mmdb_error));
        exit(4);
    }

    return result;
}

LOCAL void random_ipv4(char *ip)
{
    // rand() is perfectly fine for this use case
    // coverity[dont_call]
    int ip_int = rand();
    uint8_t *bytes = (uint8_t *)&ip_int;

    snprintf(ip, 16, "%u.%u.%u.%u",
             *bytes, *(bytes + 1), *(bytes + 2), *(bytes + 3));
}