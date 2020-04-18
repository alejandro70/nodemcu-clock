#include <Arduino.h>
#include <Timer.h>
#include <WiFiUdp.h>

extern const int NTP_PACKET_SIZE; // NTP time stamp is in the first 48 bytes of the message
extern byte packetBuffer[];       //buffer to hold incoming and outgoing packets

/* Don't hardwire the IP address or we won't get the benefits of the pool.
    Lookup the IP address for the host name instead */
extern const char *ntpServerName;
extern IPAddress timeServerIP; // time.nist.gov NTP server address
extern unsigned int localPort; // local port to listen for UDP packets
extern const int timeZone;     // Eastern Standard Time (USA)
extern WiFiUDP udp;            // A UDP instance to let us send and receive packets over UDP

time_t getNtpTime();
void sendNTPpacket(IPAddress &);
