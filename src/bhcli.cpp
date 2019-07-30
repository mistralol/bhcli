
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <syslog.h>
#include <signal.h>

#include <string>
#include <sstream>

#include <json/json.h>
#include <curl/curl.h>

#include "bhcli.h"
#include "Rest.h"

bool debug = false;
int default_exitcode = 0;
unsigned int deadline = 10;
bool logging = false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
void Debug(const char *fmt, ...) {
    if (!debug) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    char *tmp = NULL;
    if (vasprintf(&tmp, fmt, ap) < 0)
        abort();
    if (logging)
        syslog(LOG_DEBUG, "%s", tmp);
    fprintf(stdout, "%s\n", tmp);
    free(tmp);
    va_end(ap);
}

void Log(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *tmp = NULL;
    if (vasprintf(&tmp, fmt, ap) < 0)
        abort();
    if (logging)
        syslog(LOG_INFO, "%s", tmp);
    fprintf(stdout, "%s\n", tmp);
    free(tmp);
    va_end(ap);
}

void LogError(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *tmp = NULL;
    if (vasprintf(&tmp, fmt, ap) < 0)
        abort();
    if (logging)
        syslog(LOG_ERR, "%s", tmp);
    fprintf(stdout, "%s\n", tmp);
    free(tmp);
    va_end(ap);
}
#pragma GCC diagnostic pop

static void print_help(FILE *fp, char *app) {
    fprintf(fp, "Usage: %s -s (ssh|smtp|pop3|etc..) [<options>] <ip address>\n", app);
    fprintf(fp, "\n");
    fprintf(fp, " -d --deadline       Set deadline for when to exit (current: %u)\n", deadline);
    fprintf(fp, " -e --exit-code      Set default exit code during failure / deadline (current: %d)\n", default_exitcode);
    fprintf(fp, " -l --log            Turn on loggin to syslog\n");
    fprintf(fp, " -u --url            Set upstream url default: https://api.banhosts.com\n");
    fprintf(fp, " -s --service        Service type to use\n");
    fprintf(fp, " -v --verbose        Enable debug messages\n");
    fprintf(fp, " -V --version        Print the current version\n");
    fprintf(fp, "\n");
}

static void sig_alarm(int num) {
    _exit(default_exitcode);
}

int main(int argc, char **argv) {
    std::string CheckURL = "https://api.banhosts.com/check";
    std::string Service = "";
    const char *opts = "d:e:hlu:s:vV";
    int longindex = 0;
    int c = 0;

    struct option loptions[] {
        {"help", 0, 0, 'h'},
        {"deadline", 1, 0, 'd'},
        {"exit-code", 1, 0, 'e'},
        {"log", 0, 0, 'l'},
        {"url", 1, 0, 'u'},
        {"verbose", 0, 0, 'v'},
        {"version", 0, 0, 'V'},
        {0, 0, 0, 0}
    };

    curl_global_init(CURL_GLOBAL_ALL);

    while( (c = getopt_long(argc, argv, opts, loptions, &longindex)) >= 0) {
        switch (c) {
            case 'd':
                deadline = atoi(optarg);
                Debug("Deadline setup as %d", deadline);
                break;
            case 'e':
                default_exitcode = atoi(optarg);
                Debug("Default Exit Code: %d", default_exitcode);
                break;
            case 'h':
                print_help(stdout, argv[0]);
                exit(EXIT_SUCCESS);
                break;
            case 'l':
                logging = true;
                break;
            case 's':
                Service = optarg;
                Debug("Setting Service to: %s", Service.c_str());
                break;
            case 'u':
                CheckURL = optarg;
                CheckURL += "/check";
                Debug("Setting URL to %s", CheckURL.c_str());
                break;
            case 'v':
                debug = true;
                Debug("Debug messages enabled");
                break;
            case 'V':
                printf("Version: %s\n", PACKAGE_VERSION);
                exit(EXIT_SUCCESS);
            default:
                exit(EXIT_FAILURE);
                break;
        }
    }

    if (deadline > 0) {
        struct sigaction salarm;
        memset(&salarm, 0, sizeof(salarm));
        salarm.sa_handler = sig_alarm;
        if (sigaction(SIGALRM, &salarm, NULL) < 0) {
            abort();
        }

        alarm(deadline);
    }

    if (optind == argc) {
        LogError("Missing arguments");
        exit(EXIT_FAILURE);
    }

    if (optind + 1 < argc) {
        LogError("Too many arguments");
        exit(EXIT_FAILURE);
    }

    if (Service == "") {
        LogError("The service name must be set using -s");
        exit(EXIT_FAILURE);
    }

    std::string IP = argv[optind];
    Debug("URL: %s", CheckURL.c_str());
    Debug("IP Address: %s", IP.c_str());
    Debug("Service: %s", Service.c_str());

    Json::Value Request, Response;
    Request["Address"] = IP;
    Request["Service"] = Service;
    
    int status = Rest::Post(CheckURL, Request, Response);
    switch(status) {
        case 200:
            break;
        default:
            LogError("Unhandled HTTP return code %d", status);
            break;
    }

    if (debug) {
       std::stringstream ss;
        ss << Response;
        Debug("Response: %s", ss.str().c_str());
    }

    if (Response.isMember("Accept") && Response.isMember("Reason") &&
        Response["Accept"].isBool() && Response["Reason"].isString()) {
        std::string Reason = Response["Reason"].asString();
        bool Accept = Response["Accept"].asBool();

        Log("IP: %s Accepted: %s Reason: %s", IP.c_str(),
            Accept ? "Yes" : "No",
            Reason.c_str() );

        Accept ? exit(EXIT_SUCCESS) : exit(EXIT_FAILURE);
    } else {
        LogError("Failed to parse service response");
    }

    curl_global_cleanup();
    return default_exitcode;
}
