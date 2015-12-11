/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
#                                                                              #
#      Copyright (C) 2007 Tom Stöveken                                         #
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#                                                                              #
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#                                                                              #
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <pthread.h>
#include <syslog.h>

#include "urobot.h"
#include "device.h"
#include "utils.h"
#include "devices/streamer.h"
#include "sts.h"

#define OUTPUT_PLUGIN_NAME "http web steamer"

/*
 * keep context for each server
 */
static struct urobot_t *ut;
context servers[MAX_DEVICE_PLUGINS];

/******************************************************************************
Description.: initializes the iobuffer structure properly
Input Value.: pointer to already allocated iobuffer
Return Value: iobuf
******************************************************************************/
void init_iobuffer(iobuffer *iobuf)
{
    memset(iobuf->buffer, 0, sizeof(iobuf->buffer));
    iobuf->level = 0;
}

/******************************************************************************
Description.: initializes the request structure properly
Input Value.: pointer to already allocated req
Return Value: req
******************************************************************************/
void init_request(request *req)
{
    req->type        = A_UNKNOWN;
    req->type        = A_UNKNOWN;
    req->parameter   = NULL;
    req->client      = NULL;
    req->credentials = NULL;
}

/******************************************************************************
Description.: If strings were assigned to the different members free them
              This will fail if strings are static, so always use strdup().
Input Value.: req: pointer to request structure
Return Value: -
******************************************************************************/
void free_request(request *req)
{
    if(req->parameter != NULL) free(req->parameter);
    if(req->client != NULL) free(req->client);
    if(req->credentials != NULL) free(req->credentials);
}

/******************************************************************************
Description.: read with timeout, implemented without using signals
              tries to read len bytes and returns if enough bytes were read
              or the timeout was triggered. In case of timeout the return
              value may differ from the requested bytes "len".
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
int _read(int fd, iobuffer *iobuf, void *buffer, size_t len, int timeout)
{
    int copied = 0, rc, i;
    fd_set fds;
    struct timeval tv;

    memset(buffer, 0, len);

    while((copied < len)) {
        i = MIN(iobuf->level, len - copied);
        memcpy(buffer + copied, iobuf->buffer + IO_BUFFER - iobuf->level, i);

        iobuf->level -= i;
        copied += i;
        if(copied >= len)
            return copied;

        /* select will return in case of timeout or new data arrived */
        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        if((rc = select(fd + 1, &fds, NULL, NULL, &tv)) <= 0) {
            if(rc < 0)
                exit(EXIT_FAILURE);

            /* this must be a timeout */
            return copied;
        }

        init_iobuffer(iobuf);

        /*
         * there should be at least one byte, because select signalled it.
         * But: It may happen (very seldomly), that the socket gets closed remotly between
         * the select() and the following read. That is the reason for not relying
         * on reading at least one byte.
         */
        if((iobuf->level = read(fd, &iobuf->buffer, IO_BUFFER)) <= 0) {
            /* an error occured */
            return -1;
        }

        /* align data to the end of the buffer if less than IO_BUFFER bytes were read */
        memmove(iobuf->buffer + (IO_BUFFER - iobuf->level), iobuf->buffer, iobuf->level);
    }

    return 0;
}

/******************************************************************************
Description.: Read a single line from the provided fildescriptor.
              This funtion will return under two conditions:
              * line end was reached
              * timeout occured
Input Value.: * fd.....: fildescriptor to read from
              * iobuf..: iobuffer that allows to use this functions from multiple
                         threads because the complete context is the iobuffer.
              * buffer.: The buffer to store values at, will be set to zero
                         before storing values.
              * len....: the length of buffer
              * timeout: seconds to wait for an answer
Return Value: * buffer.: will become filled with bytes read
              * iobuf..: May get altered to save the context for future calls.
              * func().: bytes copied to buffer or -1 in case of error
******************************************************************************/
/* read just a single line or timeout */
int _readline(int fd, iobuffer *iobuf, void *buffer, size_t len, int timeout)
{
    char c = '\0', *out = buffer;
    int i;

    memset(buffer, 0, len);

    for(i = 0; i < len && c != '\n'; i++) {
        if(_read(fd, iobuf, &c, 1, timeout) <= 0) {
            /* timeout or error occured */
            return -1;
        }
        *out++ = c;
    }

    return i;
}

/******************************************************************************
Description.: Decodes the data and stores the result to the same buffer.
              The buffer will be large enough, because base64 requires more
              space then plain text.
Hints.......: taken from busybox, but it is GPL code
Input Value.: base64 encoded data
Return Value: plain decoded data
******************************************************************************/
void decodeBase64(char *data)
{
    const unsigned char *in = (const unsigned char *)data;
    /* The decoded size will be at most 3/4 the size of the encoded */
    unsigned ch = 0;
    int i = 0;

    while(*in) {
        int t = *in++;

        if(t >= '0' && t <= '9')
            t = t - '0' + 52;
        else if(t >= 'A' && t <= 'Z')
            t = t - 'A';
        else if(t >= 'a' && t <= 'z')
            t = t - 'a' + 26;
        else if(t == '+')
            t = 62;
        else if(t == '/')
            t = 63;
        else if(t == '=')
            t = 0;
        else
            continue;

        ch = (ch << 6) | t;
        i++;
        if(i == 4) {
            *data++ = (char)(ch >> 16);
            *data++ = (char)(ch >> 8);
            *data++ = (char) ch;
            i = 0;
        }
    }
    *data = '\0';
}

/******************************************************************************
Description.: convert a hexadecimal ASCII character to integer
Input Value.: ASCII character
Return Value: corresponding value between 0 and 15, or -1 in case of error
******************************************************************************/
int hex_char_to_int(char in)
{
    if(in >= '0' && in <= '9')
        return in - '0';

    if(in >= 'a' && in <= 'f')
        return (in - 'a') + 10;

    if(in >= 'A' && in <= 'F')
        return (in - 'A') + 10;

    return -1;
}

/******************************************************************************
Description.: replace %XX with the character code it represents, URI
Input Value.: string to unescape
Return Value: 0 if everything is ok, -1 in case of error
******************************************************************************/
int unescape(char *string)
{
    char *source = string, *destination = string;
    int src, dst, length = strlen(string), rc;

    /* iterate over the string */
    for(dst = 0, src = 0; src < length; src++) {

        /* is it an escape character? */
        if(source[src] != '%') {
            /* no, so just go to the next character */
            destination[dst] = source[src];
            dst++;
            continue;
        }

        /* yes, it is an escaped character */

        /* check if there are enough characters */
        if(src + 2 > length) {
            return -1;
            break;
        }

        /* perform replacement of %## with the corresponding character */
        if((rc = hex_char_to_int(source[src+1])) == -1) return -1;
        destination[dst] = rc * 16;
        if((rc = hex_char_to_int(source[src+2])) == -1) return -1;
        destination[dst] += rc;

        /* advance pointers, here is the reason why the resulting string is shorter */
        dst++; src += 2;
    }

    /* ensure the string is properly finished with a null-character */
    destination[dst] = '\0';

    return 0;
}

/******************************************************************************
Description.: Send a complete HTTP response and a single JPG-frame.
Input Value.: fildescriptor fd to send the answer to
Return Value: -
******************************************************************************/
void send_snapshot(int fd, int input_number)
{
    unsigned char *frame = NULL;
    int frame_size = 0;
    char buffer[BUFFER_SIZE] = {0};
    struct timeval timestamp;

    /* wait for a fresh frame */
    pthread_mutex_lock(&ut->comm[input_number]->mutex);
    pthread_cond_wait(&ut->comm[input_number]->cond, &ut->comm[input_number]->mutex);

    /* read buffer */
    frame_size = ut->comm[input_number]->size;

    /* allocate a buffer for this single frame */
    if((frame = malloc(frame_size + 1)) == NULL) {
        free(frame);
        pthread_mutex_unlock(&ut->comm[input_number]->mutex);
        send_error(fd, 500, "not enough memory");
        return;
    }
    /* copy v4l2_buffer timeval to user space */
    timestamp = ut->comm[input_number]->timestamp;

    memcpy(frame, ut->comm[input_number]->buf, frame_size);
    DBG("got frame (size: %d kB)\n", frame_size / 1024);

    pthread_mutex_unlock(&ut->comm[input_number]->mutex);

    /* write the response */
    sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            STD_HEADER \
            "Content-type: image/jpeg\r\n" \
            "X-Timestamp: %d.%06d\r\n" \
            "\r\n", (int) timestamp.tv_sec, (int) timestamp.tv_usec);

    /* send header and image now */
    if(write(fd, buffer, strlen(buffer)) < 0 || \
            write(fd, frame, frame_size) < 0) {
        free(frame);
        return;
    }

    free(frame);
}

/******************************************************************************
Description.: Send a complete HTTP response and a stream of JPG-frames.
Input Value.: fildescriptor fd to send the answer to
Return Value: -
******************************************************************************/
void send_stream(int fd, int input_number)
{
    unsigned char *frame = NULL, *tmp = NULL;
    int frame_size = 0, max_frame_size = 0;
    char buffer[BUFFER_SIZE] = {0};
    struct timeval timestamp;

    DBG("preparing header\n");
    sprintf(buffer, "HTTP/1.0 200 OK\r\n" \
            STD_HEADER \
            "Content-Type: multipart/x-mixed-replace;boundary=" BOUNDARY "\r\n" \
            "\r\n" \
            "--" BOUNDARY "\r\n");

    if(write(fd, buffer, strlen(buffer)) < 0) {
        free(frame);
        return;
    }

    DBG("Headers send, sending stream now\n");

    while(!ut->stop) {

        /* wait for fresh frames */
        pthread_mutex_lock(&ut->comm[input_number]->mutex);
        pthread_cond_wait(&ut->comm[input_number]->cond, &ut->comm[input_number]->mutex);

        /* read buffer */
        frame_size = ut->comm[input_number]->size;

        /* check if framebuffer is large enough, increase it if necessary */
        if(frame_size > max_frame_size) {
            DBG("increasing buffer size to %d\n", frame_size);

            max_frame_size = frame_size + TEN_K;
            if((tmp = realloc(frame, max_frame_size)) == NULL) {
                free(frame);
                pthread_mutex_unlock(&ut->comm[input_number]->mutex);
                send_error(fd, 500, "not enough memory");
                return;
            }

            frame = tmp;
        }

        /* copy v4l2_buffer timeval to user space */
        timestamp = ut->comm[input_number]->timestamp;

        memcpy(frame, ut->comm[input_number]->buf, frame_size);
        DBG("got frame (size: %d kB)\n", frame_size / 1024);

        pthread_mutex_unlock(&ut->comm[input_number]->mutex);

        /*
         * print the individual mimetype and the length
         * sending the content-length fixes random stream disruption observed
         * with firefox
         */
        sprintf(buffer, "Content-Type: image/jpeg\r\n" \
                "Content-Length: %d\r\n" \
                "X-Timestamp: %d.%06d\r\n" \
                "\r\n", frame_size, (int)timestamp.tv_sec, (int)timestamp.tv_usec);
        DBG("sending intemdiate header\n");
        if(write(fd, buffer, strlen(buffer)) < 0) break;

        DBG("sending frame\n");
        if(write(fd, frame, frame_size) < 0) break;

        DBG("sending boundary\n");
        sprintf(buffer, "\r\n--" BOUNDARY "\r\n");
        if(write(fd, buffer, strlen(buffer)) < 0) break;
    }

    free(frame);
}

/******************************************************************************
Description.: Send error messages and headers.
Input Value.: * fd.....: is the filedescriptor to send the message to
              * which..: HTTP error code, most popular is 404
              * message: append this string to the displayed response
Return Value: -
******************************************************************************/
void send_error(int fd, int which, char *message)
{
    char buffer[BUFFER_SIZE] = {0};

    if(which == 401) {
        sprintf(buffer, "HTTP/1.0 401 Unauthorized\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "WWW-Authenticate: Basic realm=\"MJPG-Streamer\"\r\n" \
                "\r\n" \
                "401: Not Authenticated!\r\n" \
                "%s", message);
    } else if(which == 404) {
        sprintf(buffer, "HTTP/1.0 404 Not Found\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "404: Not Found!\r\n" \
                "%s", message);
    } else if(which == 500) {
        sprintf(buffer, "HTTP/1.0 500 Internal Server Error\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "500: Internal Server Error!\r\n" \
                "%s", message);
    } else if(which == 400) {
        sprintf(buffer, "HTTP/1.0 400 Bad Request\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "400: Not Found!\r\n" \
                "%s", message);
    } else {
        sprintf(buffer, "HTTP/1.0 501 Not Implemented\r\n" \
                "Content-type: text/plain\r\n" \
                STD_HEADER \
                "\r\n" \
                "501: Not Implemented!\r\n" \
                "%s", message);
    }

    if(write(fd, buffer, strlen(buffer)) < 0) {
        DBG("write failed, done anyway\n");
    }
}

/******************************************************************************
Description.: Serve a connected TCP-client. This thread function is called
              for each connect of a HTTP client like a webbrowser. It determines
              if it is a valid HTTP request and dispatches between the different
              response options.
Input Value.: arg is the filedescriptor and server-context of the connected TCP
              socket. It must have been allocated so it is freeable by this
              thread function.
Return Value: always NULL
******************************************************************************/
/* thread for clients that connected to this server */
void *client_thread(void *arg)
{
    int cnt;
    char input_suffixed = 0;
    int input_number = 0;
    char buffer[BUFFER_SIZE] = {0}, *pb = buffer;
    iobuffer iobuf;
    request req;
    cfd lcfd; /* local-connected-file-descriptor */

    /* we really need the fildescriptor and it must be freeable by us */
    if(arg != NULL) {
        memcpy(&lcfd, arg, sizeof(cfd));
        free(arg);
    } else
        return NULL;

    /* initializes the structures */
    init_iobuffer(&iobuf);
    init_request(&req);

    /* What does the client want to receive? Read the request. */
    memset(buffer, 0, sizeof(buffer));
    if((cnt = _readline(lcfd.fd, &iobuf, buffer, sizeof(buffer) - 1, 5)) == -1) {
        close(lcfd.fd);
        return NULL;
    }

    /* determine what to deliver */
    if(strstr(buffer, "GET /?action=snapshot") != NULL) {
        req.type = A_SNAPSHOT;
#ifdef WXP_COMPAT
    } else if((strstr(buffer, "GET /cam") != NULL) && (strstr(buffer, ".jpg") != NULL)) {
        req.type = A_SNAPSHOT;
#endif
        input_suffixed = 255;
    } else if(strstr(buffer, "GET /?action=stream") != NULL) {
        input_suffixed = 255;
        req.type = A_STREAM;
#ifdef WXP_COMPAT
    } else if((strstr(buffer, "GET /cam") != NULL) && (strstr(buffer, ".mjpg") != NULL)) {
        req.type = A_STREAM;
#endif
        input_suffixed = 255;
    }

    /*
     * Since when we are working with multiple input plugins
     * there are some url which could have a _[plugin number suffix]
     * For compatibility reasons it could be left in that case the output will be
     * generated from the 0. input plugin
     */
    if(input_suffixed) {
        char *sch = strchr(buffer, '_');
        if(sch != NULL) {  // there is an _ in the url so the input number should be present
            DBG("sch %s\n", sch + 1); // FIXME if more than 10 input plugin is added
            char numStr[3];
            memset(numStr, 0, 3);
            strncpy(numStr, sch + 1, 1);
            input_number = atoi(numStr);
        }
        DBG("input plugin_no: %d\n", input_number);
    }

    /*
     * parse the rest of the HTTP-request
     * the end of the request-header is marked by a single, empty line with "\r\n"
     */
    do {
        memset(buffer, 0, sizeof(buffer));

        if((cnt = _readline(lcfd.fd, &iobuf, buffer, sizeof(buffer) - 1, 5)) == -1) {
            free_request(&req);
            close(lcfd.fd);
            return NULL;
        }

        if(strstr(buffer, "User-Agent: ") != NULL) {
            req.client = strdup(buffer + strlen("User-Agent: "));
        } else if(strstr(buffer, "Authorization: Basic ") != NULL) {
            req.credentials = strdup(buffer + strlen("Authorization: Basic "));
            decodeBase64(req.credentials);
            DBG("username:password: %s\n", req.credentials);
        }

    } while(cnt > 2 && !(buffer[0] == '\r' && buffer[1] == '\n'));

    /* check for username and password if parameter -c was given */
    if(lcfd.pc->conf.credentials != NULL) {
        if(req.credentials == NULL || strcmp(lcfd.pc->conf.credentials, req.credentials) != 0) {
            DBG("access denied\n");
            send_error(lcfd.fd, 401, "username and password do not match to configuration");
            close(lcfd.fd);
            if(req.parameter != NULL) free(req.parameter);
            if(req.client != NULL) free(req.client);
            if(req.credentials != NULL) free(req.credentials);
            return NULL;
        }
        DBG("access granted\n");
    }

    /* now it's time to answer */

    if(!(input_number < ut->devcnt)) {
        DBG("Input number: %d out of range (valid: 0..%d)\n", input_number, ut->devcnt-1);
        send_error(lcfd.fd, 404, "Invalid input plugin number");
        req.type = A_UNKNOWN;
    }

    switch(req.type) {
    case A_SNAPSHOT:
        DBG("Request for snapshot from input: %d\n", input_number);
        send_snapshot(lcfd.fd, input_number);
        break;
    case A_STREAM:
        DBG("Request for stream from input: %d\n", input_number);
        send_stream(lcfd.fd, input_number);
        break;
    default:
        DBG("unknown request\n");
    }

    close(lcfd.fd);
    free_request(&req);

    DBG("leaving HTTP client thread\n");
    return NULL;
}

/******************************************************************************
Description.: This function cleans up ressources allocated by the server_thread
Input Value.: arg is not used
Return Value: -
******************************************************************************/
void server_cleanup(void *arg)
{
    context *pcontext = arg;
    int i;

    DLOG("cleaning up ressources allocated by server thread #%02d\n", pcontext->id);

    for(i = 0; i < MAX_SD_LEN; i++)
        close(pcontext->sd[i]);
}

/******************************************************************************
Description.: Open a TCP socket and wait for clients to connect. If clients
              connect, start a new thread for each accepted connection.
Input Value.: arg is a pointer to the globals struct
Return Value: always NULL, will only return on exit
******************************************************************************/
void *server_thread(void *arg)
{
    int on;
    pthread_t client;
    struct addrinfo *aip, *aip2;
    struct addrinfo hints;
    struct sockaddr_storage client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_storage);
    fd_set selectfds;
    int max_fds = 0;
    char name[NI_MAXHOST];
    int err;
    int i;

    context *pcontext = arg;
    ut = pcontext->ut;

    /* set cleanup handler to cleanup ressources */
    pthread_cleanup_push(server_cleanup, pcontext);

    bzero(&hints, sizeof(hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    snprintf(name, sizeof(name), "%d", ntohs(pcontext->conf.port));
    if((err = getaddrinfo(NULL, name, &hints, &aip)) != 0) {
        perror(gai_strerror(err));
        exit(EXIT_FAILURE);
    }

    for(i = 0; i < MAX_SD_LEN; i++)
        pcontext->sd[i] = -1;

    /* open sockets for server (1 socket / address family) */
    i = 0;
    for(aip2 = aip; aip2 != NULL; aip2 = aip2->ai_next) {
        if((pcontext->sd[i] = socket(aip2->ai_family, aip2->ai_socktype, 0)) < 0) {
            continue;
        }

        /* ignore "socket already in use" errors */
        on = 1;
        if(setsockopt(pcontext->sd[i], SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
            perror("setsockopt(SO_REUSEADDR) failed");
        }

        /* IPv6 socket should listen to IPv6 only, otherwise we will get "socket already in use" */
        on = 1;
        if(aip2->ai_family == AF_INET6 && setsockopt(pcontext->sd[i], IPPROTO_IPV6, IPV6_V6ONLY,
                (const void *)&on , sizeof(on)) < 0) {
            perror("setsockopt(IPV6_V6ONLY) failed");
        }

        /* perhaps we will use this keep-alive feature oneday */
        /* setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)); */

        if(bind(pcontext->sd[i], aip2->ai_addr, aip2->ai_addrlen) < 0) {
            perror("bind");
            pcontext->sd[i] = -1;
            continue;
        }

        if(listen(pcontext->sd[i], 10) < 0) {
            perror("listen");
            pcontext->sd[i] = -1;
        } else {
            i++;
            if(i >= MAX_SD_LEN) {
                DLOG("%s(): maximum number of server sockets exceeded", __FUNCTION__);
                i--;
                break;
            }
        }
    }

    pcontext->sd_len = i;

    if(pcontext->sd_len < 1) {
        DLOG("%s(): bind(%d) failed", __FUNCTION__, htons(pcontext->conf.port));
        closelog();
        exit(EXIT_FAILURE);
    }

    /* create a child for every client that connects */
    while(!ut->stop) {
        //int *pfd = (int *)malloc(sizeof(int));
        cfd *pcfd = malloc(sizeof(cfd));

        if(pcfd == NULL) {
            fprintf(stderr, "failed to allocate (a very small amount of) memory\n");
            exit(EXIT_FAILURE);
        }

        DBG("waiting for clients to connect\n");

        do {
            FD_ZERO(&selectfds);

            for(i = 0; i < MAX_SD_LEN; i++) {
                if(pcontext->sd[i] != -1) {
                    FD_SET(pcontext->sd[i], &selectfds);

                    if(pcontext->sd[i] > max_fds)
                        max_fds = pcontext->sd[i];
                }
            }

            err = select(max_fds + 1, &selectfds, NULL, NULL, NULL);

            if(err < 0 && errno != EINTR) {
                perror("select");
                exit(EXIT_FAILURE);
            }
        } while(err <= 0);

        for(i = 0; i < max_fds + 1; i++) {
            if(pcontext->sd[i] != -1 && FD_ISSET(pcontext->sd[i], &selectfds)) {
                pcfd->fd = accept(pcontext->sd[i], (struct sockaddr *)&client_addr, &addr_len);
                pcfd->pc = pcontext;

                /* start new thread that will handle this TCP connected client */
                DBG("create thread to handle client that just established a connection\n");

#if 0
/* commented out as it fills up syslog with many redundant entries */

                if(getnameinfo((struct sockaddr *)&client_addr, addr_len, name, sizeof(name), NULL, 0, NI_NUMERICHOST) == 0) {
                    syslog(LOG_INFO, "serving client: %s\n", name);
                }
#endif
                if(pthread_create(&client, NULL, &client_thread, pcfd) != 0) {
                    DBG("could not launch another client thread\n");
                    close(pcfd->fd);
                    free(pcfd);
                    continue;
                }
                pthread_detach(client);
            }
        }
    }

    DBG("leaving server thread, calling cleanup function now\n");
    pthread_cleanup_pop(1);

    return NULL;
}

/*** plugin interface functions ***/
/******************************************************************************
Description.: Initialize this plugin.
              parse configuration parameters,
              store the parsed values in global variables
Input Value.: All parameters to work with.
              Among many other variables the "param->id" is quite important -
              it is used to distinguish between several server instances
Return Value: 0 if everything is OK, other values signal an error
******************************************************************************/
int streamer_init(device_argument *args, int id)
{
    int i;
    int  port;
    char *credentials, *www_folder;
    char nocommands;

    DBG("output #%02d\n", args->id);

    port = htons(8008);
    credentials = NULL;
    www_folder = NULL;
    nocommands = 0;

    args->argv[0] = OUTPUT_PLUGIN_NAME;

    /* show all parameters for DBG purposes */
    for(i = 0; i < args->argc; i++) {
        DBG("argv[%d]=%s\n", i, args->argv[i]);
    }

    reset_getopt();
    while(1) {
        int option_index = 0, c = 0;
        static struct option long_options[] = {
            {"p", required_argument, 0, 0},
            {"port", required_argument, 0, 0},
            {"c", required_argument, 0, 0},
            {"credentials", required_argument, 0, 0},
            {"n", no_argument, 0, 0},
            {"nocommands", no_argument, 0, 0},
            {0, 0, 0, 0}
        };

        c = getopt_long_only(args->argc, args->argv, "", long_options, &option_index);

        /* no more options to parse */
        if(c == -1) break;

        /* unrecognized option */
        if(c == '?') {
            return 1;
        }

        switch(option_index) {
            /* p, port */
        case 0:
        case 1:
            DBG("case 0,1\n");
            port = htons(atoi(optarg));
            break;

            /* c, credentials */
        case 2:
        case 3:
            DBG("case 2,3\n");
            credentials = strdup(optarg);
            break;

            /* n, nocommands */
        case 4:
        case 5:
            DBG("case 4,5\n");
            nocommands = 1;
            break;
        }
    }

    servers[args->id].id = args->id;
    servers[args->id].ut = args->ut;
    servers[args->id].conf.port = port;
    servers[args->id].conf.credentials = credentials;
    servers[args->id].conf.nocommands = nocommands;

    DLOG("HTTP TCP port.....: %d\n", ntohs(port));
    DLOG("username:password.: %s\n", (credentials == NULL) ? "disabled" : credentials);
    DLOG("commands..........: %s\n", (nocommands) ? "disabled" : "enabled");
    return 0;
}

/******************************************************************************
Description.: this will stop the server thread, client threads
              will not get cleaned properly, because they run detached and
              no pointer is kept. This is not a huge issue, because this
              funtion is intended to clean up the biggest mess on shutdown.
Input Value.: id determines which server instance to send commands to
Return Value: always 0
******************************************************************************/
int streamer_stop(int id)
{

    DBG("will cancel server thread #%02d\n", id);
    pthread_cancel(servers[id].threadID);

    return 0;
}

/******************************************************************************
Description.: This creates and starts the server thread
Input Value.: id determines which server instance to send commands to
Return Value: always 0
******************************************************************************/
int streamer_run(int id)
{
    DBG("launching server thread #%02d\n", id);

    /* create thread and pass context to thread function */
    pthread_create(&(servers[id].threadID), NULL, server_thread, &(servers[id]));
    pthread_detach(servers[id].threadID);

    return 0;
}

/******************************************************************************
Description.: This is just an example function, to show how the output
              plugin could implement some special command.
              If you want to control some GPIO Pin this is a good place to
              implement it. Dont forget to add command types and a mapping.
Input Value.: cmd is the command type
              id determines which server instance to send commands to
Return Value: 0 indicates success, other values indicate an error
******************************************************************************/
int streamer_cmd(struct service_t *service, unsigned int command_id, action_arg_t * input, action_arg_t *output)
{
    int ret = -1;
    int i = 0, index, type;
    device *dev= (device *) service->dev;
    //v4l2priv *priv = (v4l2priv *) dev->priv_data;
    DBG("command (%d) triggered for device instance #%02d\n", command_id, input[0].i);

    if (strstr(service->type, "Streaming")) {
        switch(command_id) {
        case GET_INFO:
            index = input[0].i;
            type = input[1].i;
            break;
        }
    }
    return 0;
}
