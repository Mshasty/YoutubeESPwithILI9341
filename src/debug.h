#define DBG_OUTPUT_PORT Serial
#ifndef RELEASE
#define DEBUGLOG(...) DBG_OUTPUT_PORT.printf(__VA_ARGS__)
#else
#define DEBUGLOG(...)
#endif

#ifndef RELEASE
void ptintdeb(String s)
{
  Serial.println(s);
}
#define DEBG ptintdeb
#else
void ptintdeb(String s) {}
#define DEBG ptintdeb
#endif