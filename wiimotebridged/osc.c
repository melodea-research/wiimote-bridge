#include "osc.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static AvahiSimplePoll *simple_poll = NULL;
static int discovery_complete = 0;

// Forward declaration of the callback
static void service_type_browser_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void* userdata);

static void resolve_callback(
    AVAHI_GCC_UNUSED AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {
    
    printf("DEBUG: Entered resolve_callback\n");
    
    if (!userdata) {
        printf("ERROR: NULL userdata in resolve_callback\n");
        return;
    }

    osc_client_t* client = (osc_client_t*)userdata;
    
    if (!r) {
        printf("ERROR: NULL resolver in resolve_callback\n");
        return;
    }
    
    AvahiClient *c = avahi_service_resolver_get_client(r);
    if (!c) {
        printf("ERROR: NULL Avahi client in resolve_callback\n");
        return;
    }

    switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            printf("Failed to resolve service '%s': %s\n", 
                name ? name : "unknown", 
                avahi_strerror(avahi_client_errno(c)));
            break;

        case AVAHI_RESOLVER_FOUND: {
            printf("DEBUG: Service found, processing...\n");
            
            if (!name || !address || !host_name) {
                printf("ERROR: NULL parameters in resolve_callback\n");
                return;
            }

            char addr[AVAHI_ADDRESS_STR_MAX];
            if (avahi_address_snprint(addr, sizeof(addr), address) < 0) {
                printf("ERROR: Failed to convert address to string\n");
                return;
            }

            printf("DEBUG: Processing service '%s'\n", name);
            
            // Only process if this is our target service
            if (strcmp(name, TARGET_SERVICE_NAME) != 0) {
                printf("DEBUG: Ignoring non-target service: %s\n", name);
                return;
            }

            printf("DEBUG: Found target service at %s:%u\n", addr, port);

            // Store the IP and port first as fallback
            strncpy(client->host, addr, sizeof(client->host) - 1);
            client->host[sizeof(client->host) - 1] = '\0';
            client->port = port;

            // Try to get targetIp from TXT records
            if (txt) {
                printf("DEBUG: Processing TXT records\n");
                AvahiStringList *l;
                for (l = txt; l; l = l->next) {
                    char *key = NULL, *value = NULL;
                    
                    printf("DEBUG: Processing TXT record entry\n");
                    
                    if (avahi_string_list_get_pair(l, &key, &value, NULL) >= 0) {
                        if (key && value) {
                            printf("DEBUG: TXT record: %s = %s\n", key, value);
                            if (strcmp(key, "targetIp") == 0) {
                                strncpy(client->host, value, sizeof(client->host) - 1);
                                client->host[sizeof(client->host) - 1] = '\0';
                                printf("DEBUG: Using targetIp: %s\n", client->host);
                            }
                        }
                        if (key) avahi_free(key);
                        if (value) avahi_free(value);
                    }
                }
            }

            client->discovered = true;
            printf("DEBUG: Service discovery complete. Using %s:%u\n", client->host, client->port);
            break;
        }
    }
}

static void browse_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {
    
    printf("DEBUG: Entered browse_callback\n");
    
    if (!b) {
        printf("ERROR: NULL browser in browse_callback\n");
        return;
    }
    
    AvahiClient *c = avahi_service_browser_get_client(b);
    if (!c) {
        printf("ERROR: NULL client in browse_callback\n");
        return;
    }
    
    if (!userdata) {
        printf("ERROR: NULL userdata in browse_callback\n");
        return;
    }
    
    switch (event) {
        case AVAHI_BROWSER_FAILURE:
            printf("Browser failure: %s\n", avahi_strerror(avahi_client_errno(c)));
            if (simple_poll) {
                avahi_simple_poll_quit(simple_poll);
            }
            return;

        case AVAHI_BROWSER_NEW:
            printf("DEBUG: Found service: %s\n", name ? name : "NULL");
            if (!name) return;
            
            printf("DEBUG: Creating resolver...\n");
            if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, 
                AVAHI_PROTO_UNSPEC, 0, resolve_callback, userdata))) {
                printf("Failed to resolve service '%s': %s\n", 
                    name, avahi_strerror(avahi_client_errno(c)));
            }
            break;

        case AVAHI_BROWSER_REMOVE:
            printf("Service removed: %s\n", name ? name : "NULL");
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
            printf("Service discovery complete.\n");
            break;

        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            printf("Service cache exhausted.\n");
            break;
    }
}

// Implementation of the callback
static void service_type_browser_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
    void* userdata) {
    
    AvahiClient *c = avahi_service_browser_get_client(b);
    
    switch (event) {
        case AVAHI_BROWSER_NEW:
            printf("\nFound service instance: %s\n", name);
            printf("  Type: %s\n", type);
            printf("  Domain: %s\n", domain);
            
            if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, 
                AVAHI_PROTO_UNSPEC, 0, resolve_callback, userdata))) {
                printf("Failed to resolve service '%s': %s\n", 
                    name, avahi_strerror(avahi_client_errno(c)));
            }
            break;

        case AVAHI_BROWSER_REMOVE:
            printf("Service instance removed: %s\n", name);
            break;

        case AVAHI_BROWSER_ALL_FOR_NOW:
        case AVAHI_BROWSER_CACHE_EXHAUSTED:
            break;
    }
}

int osc_discover_server(osc_client_t* client) {
    printf("DEBUG: Starting osc_discover_server\n");
    
    if (!client) {
        printf("ERROR: NULL client passed to osc_discover_server\n");
        return -1;
    }

    // Initialize client structure
    memset(client, 0, sizeof(osc_client_t));
    
    printf("DEBUG: Creating simple poll object\n");
    if (!(simple_poll = avahi_simple_poll_new())) {
        printf("ERROR: Failed to create simple poll object\n");
        return -1;
    }

    printf("DEBUG: Creating Avahi client\n");
    int error;
    AvahiClient *avahi_client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, NULL, NULL, &error);
    if (!avahi_client) {
        printf("ERROR: Failed to create client: %s\n", avahi_strerror(error));
        goto fail;
    }

    printf("DEBUG: Creating service browser\n");
    AvahiServiceBrowser *sb = avahi_service_browser_new(avahi_client, AVAHI_IF_UNSPEC, 
        AVAHI_PROTO_UNSPEC, SERVICE_TYPE, NULL, 0, browse_callback, client);
    
    if (!sb) {
        printf("ERROR: Failed to create service browser\n");
        goto fail;
    }

    printf("DEBUG: Entering discovery loop\n");
    time_t start_time = time(NULL);
    const int timeout_seconds = 60;

    while (!client->discovered) {
        if (difftime(time(NULL), start_time) >= timeout_seconds) {
            printf("DEBUG: Discovery timed out\n");
            goto fail;
        }

        printf("Searching... (%d seconds remaining)\r", 
            (int)(timeout_seconds - difftime(time(NULL), start_time)));
        fflush(stdout);
        
        if (avahi_simple_poll_iterate(simple_poll, 1000) != 0) {
            printf("ERROR: Poll iteration failed\n");
            goto fail;
        }
    }

    printf("\nDEBUG: Discovery complete, cleaning up Avahi resources\n");
    
    if (sb) avahi_service_browser_free(sb);
    if (avahi_client) avahi_client_free(avahi_client);
    if (simple_poll) avahi_simple_poll_free(simple_poll);
    simple_poll = NULL;

    printf("DEBUG: Initializing OSC with %s:%d\n", client->host, client->port);
    
    if (osc_init(client) < 0) {
        printf("ERROR: Failed to initialize OSC client\n");
        return -1;
    }

    printf("DEBUG: OSC discovery and initialization complete\n");
    return 0;

fail:
    printf("DEBUG: Entering fail cleanup\n");
    if (sb) avahi_service_browser_free(sb);
    if (avahi_client) avahi_client_free(avahi_client);
    if (simple_poll) {
        avahi_simple_poll_free(simple_poll);
        simple_poll = NULL;
    }
    return -1;
}

int osc_init(osc_client_t* client) {
    client->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->sock < 0) {
        return -1;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(client->port);
    
    if (inet_pton(AF_INET, client->host, &client->server_addr.sin_addr) <= 0) {
        return -1;
    }

    return 0;
}

// Helper function to add OSC type tag
static void add_osc_type_tag(char* buffer, int* offset, char type) {
    buffer[*offset] = type;
    (*offset)++;
}

// Helper function to add OSC string with padding
static void add_osc_string(char* buffer, int* offset, const char* str) {
    int len = strlen(str);
    strcpy(buffer + *offset, str);
    *offset += len;
    // Add null terminator and pad to multiple of 4 bytes
    int pad = 4 - (*offset % 4);
    memset(buffer + *offset, 0, pad);
    *offset += pad;
}

// Helper function to add OSC float
static void add_osc_float(char* buffer, int* offset, float value) {
    uint32_t* ptr = (uint32_t*)(buffer + *offset);
    *ptr = htonl(*(uint32_t*)&value);
    *offset += 4;
}

// Helper function to add OSC integer
static void add_osc_int(char* buffer, int* offset, int32_t value) {
    uint32_t* ptr = (uint32_t*)(buffer + *offset);
    *ptr = htonl(value);
    *offset += 4;
}

int osc_send_message(osc_client_t* client, const char* address, const char* format, ...) {
    va_list args;
    int offset = 0;
    char* buffer = client->buffer;
    
    // Add OSC address
    add_osc_string(buffer, &offset, address);
    
    // Add type tag string
    buffer[offset++] = ',';
    const char* fmt = format;
    if (*fmt == ',') fmt++; // Skip leading comma if present
    strcpy(buffer + offset, fmt);
    offset += strlen(fmt);
    // Add null terminator and pad to multiple of 4 bytes
    int pad = 4 - (offset % 4);
    memset(buffer + offset, 0, pad);
    offset += pad;
    
    // Add arguments
    va_start(args, format);
    for (const char* p = fmt; *p != '\0'; p++) {
        switch (*p) {
            case 'f': {
                float value = (float)va_arg(args, double);
                add_osc_float(buffer, &offset, value);
                break;
            }
            case 'i': {
                int32_t value = va_arg(args, int32_t);
                add_osc_int(buffer, &offset, value);
                break;
            }
            case 's': {
                const char* str = va_arg(args, const char*);
                add_osc_string(buffer, &offset, str);
                break;
            }
            // Add more types as needed
        }
    }
    va_end(args);
    
    return sendto(client->sock, buffer, offset, 0,
                 (struct sockaddr*)&client->server_addr,
                 sizeof(client->server_addr));
}

void moving_average_init(moving_average_t* filter, float threshold) {
    memset(filter->values, 0, sizeof(filter->values));
    filter->index = 0;
    filter->sum = 0;
    filter->count = 0;
    filter->last_sent = 0;
    filter->threshold = threshold;
    filter->should_send = false;
}

float moving_average_update(moving_average_t* filter, float value) {
    // Subtract the oldest value from sum
    if (filter->count == OSC_MOVING_AVERAGE_WINDOW) {
        filter->sum -= filter->values[filter->index];
    }
    
    // Add new value
    filter->values[filter->index] = value;
    filter->sum += value;
    
    // Update count and index
    if (filter->count < OSC_MOVING_AVERAGE_WINDOW) {
        filter->count++;
    }
    filter->index = (filter->index + 1) % OSC_MOVING_AVERAGE_WINDOW;
    
    // Calculate average
    float avg = filter->sum / filter->count;
    
    // Check if change is significant
    filter->should_send = (fabs(avg - filter->last_sent) > filter->threshold);
    if (filter->should_send) {
        filter->last_sent = avg;
    }
    
    return avg;
} 