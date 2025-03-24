#include "osc.h"
#include <stdarg.h>
#include <stdio.h>
#include <math.h>

int osc_init(osc_client_t* client, const char* ip_address, int port) {
    client->sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client->sock < 0) {
        return -1;
    }

    memset(&client->server_addr, 0, sizeof(client->server_addr));
    client->server_addr.sin_family = AF_INET;
    client->server_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, ip_address, &client->server_addr.sin_addr) <= 0) {
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