#ifndef OSC_H_INCLUDED
#define OSC_H_INCLUDED

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#define OSC_MAX_MESSAGE_SIZE 1024
#define OSC_MOVING_AVERAGE_WINDOW 5
#define SERVICE_TYPE "_osc._udp"
#define TARGET_SERVICE_NAME "AgapeKidAvatarBridge"

typedef struct {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[OSC_MAX_MESSAGE_SIZE];
    bool discovered;
    char host[256];
    int port;
} osc_client_t;

typedef struct {
    float values[OSC_MOVING_AVERAGE_WINDOW];
    int index;
    float sum;
    int count;
    float last_sent;
    float threshold;
    bool should_send;
} moving_average_t;

/**
 * @brief Initialize OSC client with discovered server info
 * 
 * @param client Pointer to osc_client_t structure
 * @return int 0 on success, -1 on failure
 */
int osc_init(osc_client_t* client);

/**
 * @brief Send OSC message
 * 
 * @param client Pointer to osc_client_t structure
 * @param address OSC address pattern
 * @param format OSC type tag string
 * @param ... Variable arguments based on format string
 * @return int Number of bytes sent or -1 on error
 */
int osc_send_message(osc_client_t* client, const char* address, const char* format, ...);

/**
 * @brief Initialize moving average filter
 * 
 * @param filter Pointer to moving_average_t structure
 * @param threshold Change threshold to trigger sending
 */
void moving_average_init(moving_average_t* filter, float threshold);

/**
 * @brief Update moving average filter with new value
 * 
 * @param filter Pointer to moving_average_t structure
 * @param value New value to add
 * @return float Current average if change > threshold, or 0 if no significant change
 */
float moving_average_update(moving_average_t* filter, float value);

/**
 * @brief Discover OSC server using Zeroconf
 * 
 * @param client Pointer to osc_client_t structure
 * @return int 0 on success, -1 on failure
 */
int osc_discover_server(osc_client_t* client);

#endif /* OSC_H_INCLUDED */ 