/*
 * The httpd from ELKS, tuned to be portable to other platforms.
 *
 * Originally written by Harry Kalogirou. Massive shoutout to him.
 *
 * Copyright (c) 2026 vmunix
*/

/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*===================================*/
/* Include files                     */
/*===================================*/
#include <sys/types.h>
#include <sys/socket.h>
#define __USE_MISC
#include <sys/stat.h>
#undef __USE_MISC
#include <limits.h>
#include <fcntl.h>
#define __USE_POSIX2
#include <unistd.h>
#undef __USE_POSIX2
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#ifdef _TIME
#include <time.h>
#endif


/*===================================*/
/* Global definitions		     */
/*===================================*/
#define DEF_PORT	80
#define DEF_CONTENT     "text/html"
#define PATH_MAX	4096

#define WS(c)   ( ((c) == ' ') || ((c) == '\t') || ((c) == '\r') || ((c) == '\n') )

#define errmsg(str)     write(STDERR_FILENO, str, sizeof(str) - 1)

#define _PATH_DOCBASE	"/var/www"

#define SO_LISTEN_BUFSIZ	128

/*===================================*/
/* static variables                  */
/*===================================*/
#ifdef _FLAGS
static int port = DEF_PORT;
static const char *docbase = _PATH_DOCBASE;
#endif

#ifdef _CGI
static const char *cgibin = "/cgi-bin";
void process_cgi(int fd, char *script_path, char *script_name, char *query, int is_post, char *post_body, int post_len);
#endif

/* This is getting the MIME type of files, so the client can understand what type of file it is and show it. */

char* get_mime_type(char *name) {
    char *dot;

    dot = strrchr( name, '.' );
    if ( dot == (char*) 0 )
        return "application/octet-stream";
    if ( strcmp( dot, ".html" ) == 0 || strcmp( dot, ".htm" ) == 0 )
        return "text/html";
    if ( strcmp( dot, ".jpg" ) == 0 || strcmp( dot, ".jpeg" ) == 0 )
        return "image/jpeg";
    if ( strcmp( dot, ".gif" ) == 0 )
        return "image/gif";
    if ( strcmp( dot, ".png" ) == 0 )
        return "image/png";
    if ( strcmp( dot, ".css" ) == 0 )
        return "text/css";
    if ( strcmp( dot, ".svg" ) == 0 )
	    return "image/svg+xml";
    if ( strcmp( dot, ".bmp" ) == 0 )
	    return "image/bmp";
    if ( strcmp( dot, ".pbm" ) == 0 )
	    return "image/x-portable-bitmap";
    if ( strcmp( dot, ".m4a" ) == 0 )
	    return "audio/mp4";
    if ( strcmp( dot, ".mp4" ) == 0 )
	    return "video/mp4";
    if ( strcmp( dot, ".vrml" ) == 0 || strcmp( dot, ".wrl" ) == 0 )
        return "model/vrml";
    if ( strcmp( dot, ".midi" ) == 0 || strcmp( dot, ".mid" ) == 0 )
        return "audio/midi";
    return "application/octet-stream";
}

/* If the libc has support for time, then get the time of
 * connection with time.h in RFC 2822 format. */
#ifdef _TIME
void get_http_date(char *buf, size_t len) {
	time_t now;
	struct tm *gmt;

	time(&now);
	gmt = gmtime(&now);
	strftime(buf, len, "%a, %d %b %Y %H:%M:%S GMT", gmt);
}

/* This is sending the HTTP header to the client. */
void send_header(int fd, char *ct) {
	char buf[256]; /* increased from 128 to accept date */ 
	char datebuf[64];

	get_http_date(datebuf, sizeof(datebuf));
	sprintf(buf,
		"HTTP/1.0 200 OK\r\nServer: eshttpd\r\n"
		"Date: %s\r\n"
		"Content-Type: %s\r\n", datebuf, ct);
	write(fd, buf, strlen(buf));
}

#else

void send_header(int fd, char *ct) {
    char buf[128];

    sprintf(buf,
	"HTTP/1.0 200 OK\r\nServer: eshttpd\r\n"
        "Date: Mon Jun 8 15:37:46 GMT 2026\r\n"
        "Content-Type: %s\r\n",ct);
    write(fd, buf, strlen(buf));
}

#endif

/* This is sending the 404 Error if the file the client specified does not work */
#ifdef _TIME
void send_error(int fd, int errnum, char *str) {
	char buf[256]; /* increased from 128 */
	char datebuf[64];

	get_http_date(datebuf, sizeof(datebuf));
	sprintf(buf,
		"HTTP/1.0 %d %s\r\nContent-Type: %s\r\n", errnum, str, DEF_CONTENT);
	write(fd, buf, strlen(buf));
	sprintf(buf,
		"Connection: close\r\n"
		"Date: %s\r\n"
		"\r\n%s\r\n", datebuf, str);
	write(fd, buf, strlen(buf));
}
#else
void send_error(int fd, int errnum, char *str) {
    char buf[128];

    sprintf(buf,
	"HTTP/1.0 %d %s\r\nContent-type: %s\r\n", errnum, str, DEF_CONTENT);
    write(fd, buf, strlen(buf));
    sprintf(buf,
		"Connection: close\r\n"
                "Date: Thu Apr 26 15:37:46 GMT 2001\r\n"
                "\r\n%s\r\n", str);
    write(fd, buf, strlen(buf));
}
#endif

void process_request(int fd) {
    int fin, ret;
    char *c, *file;
    off_t size;
    struct stat st;
    char fullpath[PATH_MAX];
    char buf[1536+1];
#ifdef _CGI
    int method = 0; /* 0 = GET, 1 = POST */
    char *query = NULL;
    char *post_body = NULL;
    int post_len = 0;
    int head_end = -1;
    int total_read = 0;
#endif

    if ((ret = read(fd, buf, sizeof(buf)-1)) <= 0)
        return;
    c = buf;
    while (*c && !WS(*c) && (c - buf) < ret)
        c++;
    *c = '\0';

#ifdef _CGI
    if (strcasecmp(buf, "GET") == 0) {
        method = 0;
    } else if (strcasecmp(buf, "POST") == 0) {
        method = 1;
    } else {
        send_error(fd, 404, "Method not supported");
        return;     
    }
#else
    if (strcasecmp(buf, "GET") != 0) {
        send_error(fd, 404, "Method not supported");
        return;     
    }
#endif

    file = ++c;
    while (*c && !WS(*c) && (c - buf) < ret)
        c++;
    *c = '\0';

#ifdef _CGI
    /* split query string (file is null-terminated) */
    query = strchr(file, '?');
    if (query) {
        *query = '\0';
        query++;
    }

    /* locate header end (\r\n\r\n) and Content-Length in raw buffer */
    {
        int scan_start = (int)(c - buf) + 1;
        int i;
        head_end = -1;
        total_read = ret;
        while (head_end < 0 && total_read < (int)sizeof(buf)) {
            for (i = scan_start; i + 3 < total_read; i++) {
                if (buf[i] == '\r' && buf[i+1] == '\n' && buf[i+2] == '\r' && buf[i+3] == '\n') {
                    head_end = i + 4;
                    break;
                }
            }
            if (head_end >= 0)
                break;
            ret = read(fd, buf + total_read, sizeof(buf) - total_read);
            if (ret <= 0)
                break;
            total_read += ret;
        }
    }

    /* parse Content-Length for POST */
    if (method == 1 && head_end > 0) {
        int i;
        post_len = 0;
        for (i = (int)(c - buf) + 1; i + 15 < head_end; i++) {
            if (strncasecmp(buf + i, "Content-Length:", 15) == 0) {
                int j = i + 15;
                while (j < head_end && (buf[j] == ' ' || buf[j] == '\t'))
                    j++;
                post_len = atoi(buf + j);
                break;
            }
        }
    }
#endif

    /* vmunix: fixed a buffer overflow vulnerability here.
     * This actually saved some SLOC :) */

	/* vmunix 11/7/26
         * you f*cking dumb*ss, snprintf() ISN'T A THING IN C89!!!
	 * snprintf() WAS INVENTED IN C99!!! */
#ifdef _FLAGS
    sprintf(fullpath, "%s%s", docbase, file);
#else
    sprintf(fullpath, "%s%s", _PATH_DOCBASE, file);
#endif

    if (!stat(fullpath, &st) && (st.st_mode & S_IFMT) == S_IFDIR) {
        /* Fixed array index, was wrong before. -vmunix */
	    if (fullpath[strlen(fullpath) - 1] != '/')
            strcat(fullpath, "/");
        strcat(fullpath, "index.html");
    }

	/* This is for if somebody tries go get something out
	 * of the /var/www directory, such as trying to GET
	 * /../../../etc/passwd. I cannot test if this works
	 * or not. */
	if (strstr(fullpath, "..") != NULL) {
		send_error(fd, 403, "Forbidden");
		return;
	}

#ifdef _CGI
    /* CGI: URL prefix */
    if (strncmp(file, cgibin, strlen(cgibin)) == 0) {
        if (method == 1 && post_len > 0) {
            int already = total_read - head_end;
            post_body = malloc(post_len + 1);
            if (post_body) {
                int copied = already < post_len ? already : post_len;
                int got;
                memcpy(post_body, buf + head_end, copied);
                got = copied;
                while (got < post_len) {
                    ret = read(fd, post_body + got, post_len - got);
                    if (ret <= 0)
                        break;
                    got += ret;
                }
                post_body[got] = '\0';
                post_len = got;
            }
        }
        process_cgi(fd, fullpath, file, query, method, post_body, post_len);
        if (post_body)
            free(post_body);
        return;
    }
#endif

    fin = open(fullpath, O_RDONLY);
    if (fin < 0) {
        send_error(fd, 404, "Document (probably) not found");
        return;
    }
    size = lseek(fin, 0, SEEK_END);
    lseek(fin, 0, SEEK_SET);
    send_header(fd, get_mime_type(fullpath));
	sprintf(buf, "Content-Length: %ld\r\n\r\n", (long)size);
    write(fd, buf, strlen(buf));

    do {
        ret = read(fin, buf, sizeof(buf));
        if (ret > 0)
            ret = write(fd, buf, ret);
    } while (ret == sizeof(buf));
    close(fin);
}

#ifdef _CGI
/* This is running a CGI script and piping its output back to the client. */
void process_cgi(int fd, char *script_path, char *script_name, char *query, int is_post, char *post_body, int post_len) {
    int stdin_pipe[2], stdout_pipe[2];
    pid_t pid;
    char *envp[64];
    char *argv[2];
    char buf[1024];
    char hdr_buf[2048];
    int hdr_len = 0;
    int head_found = 0;
    int body_start = 0;
    int sep_len = 0;
    int n, i;
    int status_code = 200;
    char *status_text = "OK";
    char status_text_buf[64];
    char *content_type = "text/html";
    char content_type_buf[128];
    char *location = NULL;
    char location_buf[256];
    struct sockaddr_in peer;
    socklen_t peerlen;
    char remote_addr_buf[64];
    char server_port_buf[16];
    char script_name_buf[PATH_MAX];
    char query_string_buf[PATH_MAX];
    char content_length_buf[32];
    char path_translated_buf[PATH_MAX];

    strcpy(status_text_buf, "OK");
    status_text = status_text_buf;
    strcpy(content_type_buf, "text/html");
    content_type = content_type_buf;

    /* Build environment variables */
    i = 0;
    envp[i++] = "GATEWAY_INTERFACE=CGI/1.1";
    envp[i++] = "SERVER_SOFTWARE=eshttpd";
    envp[i++] = "SERVER_PROTOCOL=HTTP/1.0";
    envp[i++] = "SERVER_NAME=localhost";
#ifdef _FLAGS
    sprintf(server_port_buf, "SERVER_PORT=%d", port);
#else
    sprintf(server_port_buf, "SERVER_PORT=%d", DEF_PORT);
#endif
    envp[i++] = server_port_buf;
    envp[i++] = is_post ? "REQUEST_METHOD=POST" : "REQUEST_METHOD=GET";

    sprintf(script_name_buf, "SCRIPT_NAME=%s", script_name);
    envp[i++] = script_name_buf;
    envp[i++] = "PATH_INFO=";
    sprintf(path_translated_buf, "PATH_TRANSLATED=%s", script_path);
    envp[i++] = path_translated_buf;

    if (query) {
        sprintf(query_string_buf, "QUERY_STRING=%s", query);
        envp[i++] = query_string_buf;
    } else {
        envp[i++] = "QUERY_STRING=";
    }

    if (is_post) {
        sprintf(content_length_buf, "CONTENT_LENGTH=%d", post_len);
        envp[i++] = content_length_buf;
    }

    peerlen = sizeof(peer);
    if (getpeername(fd, (struct sockaddr *)&peer, &peerlen) == 0) {
        sprintf(remote_addr_buf, "REMOTE_ADDR=%s", inet_ntoa(peer.sin_addr));
        envp[i++] = remote_addr_buf;
    }

    envp[i] = NULL;

    argv[0] = script_path;
    argv[1] = NULL;

    if (access(script_path, X_OK) != 0) {
        send_error(fd, 404, "Not Found");
        return;
    }

    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0) {
        send_error(fd, 500, "Internal Server Error");
        return;
    }

    pid = fork();
    if (pid < 0) {
        close(stdin_pipe[0]);
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        close(stdout_pipe[1]);
        send_error(fd, 500, "Internal Server Error");
        return;
    }

    if (pid == 0) {
        /* child */
        close(stdin_pipe[1]);
        close(stdout_pipe[0]);
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);
        execve(script_path, argv, envp);
        _exit(127);
    }

    /* parent */
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    /* send POST body to script stdin */
    if (is_post && post_len > 0 && post_body)
        write(stdin_pipe[1], post_body, post_len);
    close(stdin_pipe[1]);

    /* read CGI output headers until blank line (CRLF or LF) */
    {
        while (!head_found && hdr_len < ((int)sizeof(hdr_buf) - 1)) {
            n = read(stdout_pipe[0], buf, sizeof(buf));
            if (n <= 0)
                break;
            {
                int space = (int)sizeof(hdr_buf) - hdr_len - 1;
                int copy = n < space ? n : space;
                memcpy(hdr_buf + hdr_len, buf, copy);
                hdr_len += copy;
                hdr_buf[hdr_len] = '\0';
            }
            {
                char *he = strstr(hdr_buf, "\r\n\r\n");
                if (he) {
                    head_found = 1;
                    sep_len = 4;
                    body_start = (int)(he - hdr_buf) + 4;
                } else {
                    he = strstr(hdr_buf, "\n\n");
                    if (he) {
                        head_found = 1;
                        sep_len = 2;
                        body_start = (int)(he - hdr_buf) + 2;
                    }
                }
            }
        }
    }

    /* parse headers */
    if (head_found) {
        char *line = hdr_buf;
        char *line_end = hdr_buf + body_start - sep_len;
        while (line < line_end) {
            char *le = memchr(line, '\n', (int)(line_end - line));
            if (!le)
                le = line_end;
            {
                char saved = *le;
                char *eol;
                *le = '\0';
                eol = line + strlen(line);
                while (eol > line && (eol[-1] == '\r' || eol[-1] == ' ' || eol[-1] == '\t'))
                    *--eol = '\0';
                if ((int)(eol - line) > 0) {
                    if (strncasecmp(line, "Status:", 7) == 0) {
                        char *v = line + 7;
                        while (*v == ' ' || *v == '\t')
                            v++;
                        status_code = atoi(v);
                        while (*v && *v != ' ')
                            v++;
                        while (*v == ' ')
                            v++;
                        strcpy(status_text_buf, *v ? v : "OK");
                        status_text = status_text_buf;
                    } else if (strncasecmp(line, "Content-Type:", 13) == 0) {
                        char *v = line + 13;
                        while (*v == ' ' || *v == '\t')
                            v++;
                        strcpy(content_type_buf, v);
                        content_type = content_type_buf;
                    } else if (strncasecmp(line, "Location:", 9) == 0) {
                        char *v = line + 9;
                        while (*v == ' ' || *v == '\t')
                            v++;
                        strcpy(location_buf, v);
                        location = location_buf;
                    }
                }
                *le = saved;
            }
            line = le + 1;
        }
    }

    /* send response headers */
    {
        char hdr[256];
        if (location) {
            sprintf(hdr, "HTTP/1.0 %d %s\r\nServer: eshttpd\r\nLocation: %s\r\n\r\n",
                status_code, status_text, location);
        } else {
            sprintf(hdr, "HTTP/1.0 %d %s\r\nServer: eshttpd\r\nContent-Type: %s\r\n\r\n",
                status_code, status_text, content_type);
        }
        write(fd, hdr, strlen(hdr));
    }

    /* send body: remainder of header buffer, then stream the pipe */
    {
        int body_left = hdr_len - body_start;
        if (body_left > 0)
            write(fd, hdr_buf + body_start, body_left);
        while ((n = read(stdout_pipe[0], buf, sizeof(buf))) > 0)
            write(fd, buf, n);
    }

    close(stdout_pipe[0]);
    waitpid(pid, NULL, 0);
}
#endif

int main(int argc, char **argv) {
    int ret, conn_sock, listen_sock;
#ifdef _FLAGS
    int opt;
#endif
    struct sockaddr_in localadr;

#ifdef _FLAGS
    while ((opt = getopt(argc, argv, "d:p:th")) != -1) { /* This is parsing all the flags if the user wants to tune anything */
	    switch (opt) {
		    case 'd':
			    docbase = optarg;
			    break;
		    case 'p':
			    port = atoi(optarg);
			    break;
		    case 'h':
			    puts("eshttpd - very small HTTP server");
			    puts("Usage: -p [PORT] -d [DOCBASE]");
			    puts("[-h|-help] This message");
		    default:
			    /* Unknown flag */
			    return 1;
	    }
    }
#endif
    (void)argc;(void)argv;

    if ((listen_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        errmsg("eshttpd: Network is down\n");
        return -1;
    }

    /* set local port reuse, allows server to be restarted in less than 10 secs */
    ret = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &ret, sizeof(int)) < 0)
        errmsg("http: SO_REUSEADDR");

    /* set small listen buffer to save ktcp memory */
    ret = SO_LISTEN_BUFSIZ;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_RCVBUF, &ret, sizeof(int)) < 0)
        errmsg("eshttpd: SO_RCVBUF");

    localadr.sin_family = AF_INET;
#ifdef _FLAGS
    localadr.sin_port = htons(port);
#else
    localadr.sin_port = htons(DEF_PORT);
#endif
    localadr.sin_addr.s_addr = INADDR_ANY;
    if (bind(listen_sock, (struct sockaddr *)&localadr, sizeof(struct sockaddr_in)) < 0) {
        errmsg("eshttpd: bind error (may already be running)\n");
	errmsg("Another reason for this is because you did not run it as root.\n");
	errmsg("On Linux, you need root privilages to access HTTP port 80.\n");
        return 1;
    }
    if (listen(listen_sock, 5) < 0) {
        errmsg("eshttpd: listen\n");
        return 1;
    }

#ifdef _FORKLESS
    puts("eshttpd compiled with -D_FORKLESS.");
    puts("Not forking...");
#else
    /* become daemon, debug output on 1 and 2*/
    if ((ret = fork()) == -1) {
        errmsg("eshttpd: No more processes\n");
        return 1;
    }

    if (ret) exit(0);
#endif
    ret = open("/dev/null", O_RDWR); /* our log file! */
    dup2(ret, 0);
    dup2(ret, 1);
    dup2(ret, 2);
    if (ret > 2)
        close(ret);
    setsid();

    while (1) {
        conn_sock = accept(listen_sock, NULL, NULL);
        if (conn_sock < 0) {
            if (errno == ENOTSOCK)
                exit(1);
            continue;
        }
#ifdef _FORKLESS
	process_request(conn_sock);
	close(conn_sock);
#else
        if ((ret = fork()) == -1) {
            close(conn_sock);
            errmsg("eshttpd: No more processes\n");
        } else if (ret) {
            close(conn_sock);
            waitpid(ret, NULL, 0);
        } else {
            close(listen_sock);
            process_request(conn_sock);
            close(conn_sock);
            exit(0);
        }
#endif
    }
}
