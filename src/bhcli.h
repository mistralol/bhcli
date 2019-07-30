
extern void Debug(const char *fmt, ...) __attribute__((format(printf, 1, 2)));;
extern void Log(const char *fmt, ...) __attribute__((format(printf, 1, 2)));;
extern void LogError(const char *fmt, ...) __attribute__((format(printf, 1, 2)));;

extern bool debug;
extern int default_exitcode;
extern unsigned int deadline;
extern bool logging;
