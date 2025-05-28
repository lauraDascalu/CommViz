#define _CRT_SECURE_NO_WARNINGS

#ifdef _WIN32  //verifica daca programul e compilat pe windows
//include biblioteci specifice pt windows
#include <windows.h>
#include <winsock.h>
#else

#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

//pt socketuri UNIX
#include <sys/un.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <netdb.h>

    //structura sockaddr_in e utilizata pt a defini adrese IPv4, 
    // incluzand iP-ul si portul serverului
#define SOCKADDR_IN struct sockaddr_in
    //structura sockaddr e folosita pt manipulare adrese retea si e folosit
    //ca tip generalizat pt toate tipurile de adrese retea(ipv4, ipv6)
#define SOCKADDR struct sockaddr
    //structura hostnet contine informatii despre gazda( nume si adresa ip)
#define HOSTNET struct hostnet
    //redefineste tipul SOCKET ca fiind de tip int-pt portabilitate Windows-UNIX
#define SOCKET int

int WSAGetLastError() { return errno; }
int closesocket(int s) { return close(s); }
#endif

#include <stdio.h>
#include <stdlib.h>

void perr_exit(char const* msg, int ret_code)
{
    printf("%s, Error: ", msg);
    printf("%d\n", ret_code);
    exit(ret_code);
}

char* extract_filename(const char* site) {
    if (site == NULL || *site == '\0') {
        return _strdup("");
    }
    const char* last_slash = strrchr(site, '/'); 
    if (last_slash == NULL) {
        return _strdup(site);
    }
    else {
        return _strdup(last_slash + 1);
    }
}

int main(int argc, char** argv)
{
    char* site; //adresa unei portiuni din url, dupa numele de gazda
    char* host; //numele hostului din url
    const int bufsz = 4069; //dim max buffer
    char url[1024];
    char send_buf[bufsz];
    char recv_buf[bufsz];
    long rc; //valorile de return a fct de retea
    SOCKET s;//o variabila s de tip socket, folosita pt a comunica in retea
    SOCKADDR_IN addr; //adresa serverului

#ifdef _WIN32
    WSADATA wsa; //wsadata e o structura folosita pt a initializa biblioteca winsock (pt manipulare socket-uri)
#endif 
    HOSTENT* hent;

    //bevor man anfangen kann, muss man diese WSAStartup Funktion aufrufen
    //inainte sa incepi, tb apelata fct WSAStartup
#ifdef _WIN32
    //initialisiert TCPIP stack
    if (WSAStartup(MAKEWORD(2, 0), &wsa))
        perr_exit("WSAStartup failed", WSAGetLastError());
    //in windows, orice program care vrea sa foloseasca socket-uri tb mai intai
    //sa initializeze biblioteca Winsock cu fct WSAStartup
    //Winsock ofera interfata la protocoalele TCP IP
#endif

    addr.sin_family = AF_INET; //setam familia de adrese a socket-ului la AF_INET, adipa IPv4
    addr.sin_port = htons(80); //seteaza portul destinatie la 80, portul standard HTTP
    //htons: host to network short
    //converteste nr 8- din endianness-ul local (little ednian pe x86) in endianness-ul retelei(big endian)
    //portul 80 e standardul pt serverele web HTTP, folosite pt acces pagini web

    printf("\nURL: ");
    scanf("%s", url);
   

    //1. parse url into host and resource path(site)
    if (strncmp("http://", url, 7) == 0)//verifica daca string-ul url incepe asa
    {
        host = url + 7; //ignoram prefixul
    }
    else
    {
        host = url;
    }

    if ((site = strchr(host, '/')) != 0)
    {
        *site++ = '\0'; //seprare numle domeniului si resursa
        //ex: host="example.com/index.html"
        //host = "example.com"  site = "index.html"
    }
    else
    {
        site = host + strlen(host);
    }

    //2. extract filename from site
    char* filename = extract_filename(site);
    printf("Filename to save: %s\n", filename);


    printf("Host: %s\n", host);
    printf("Site: %s\n", site);
    printf("Connecting ... \n");

    if ((addr.sin_addr.s_addr = inet_addr((const char*)host)) == INADDR_NONE)
        //s_addr contine adresa ip a numelui de domeniu
    {
        if (!(hent = gethostbyname(host)))
        {
            perr_exit("Cannot resolve Host", WSAGetLastError());
        }

        strncpy((char*)&addr.sin_addr.s_addr, hent->h_addr, 4);
        if (addr.sin_addr.s_addr == INADDR_NONE)
        {
            perr_exit("Cannot resolve Host", WSAGetLastError());
        }      
    }

    //creare socket
    s = socket(AF_INET, SOCK_STREAM, 0); //ipv4, tcp, protoclul impplit 0(tcp)
#ifdef _WIN32
    if (s== INVALID_SOCKET)
#else
    if (s < 0)
#endif
        perr_exit("Cannot create socket", WSAGetLastError());

    //conectare server
    if (connect(s, (SOCKADDR*)&addr, sizeof(SOCKADDR)))
    {
        perr_exit("Cannot connect", WSAGetLastError());
    }

    printf("Connected to %s...\n", host);

    //trimitere cerere
    sprintf(send_buf, "GET /%s HTTP/1.1\r\nHost: %s:80\r\n\r\n", site, host);

    printf("Command sent to server: \n%s\n", send_buf);

    if ((send(s, send_buf, strlen(send_buf), 0)) < strlen(send_buf))
    {
        perr_exit("Cannot send Data", WSAGetLastError());
    }

    //primire raspuns
    printf("----Result----\n");

    //3. open file for writing the data
    FILE* file = fopen(filename, "wb"); 
    if (!file) {
        perr_exit("Cannot open output file", 1);
    }

    int header_parsed = 0;  // Flag: did we find the empty line yet?
    while ((rc = recv(s, recv_buf, bufsz - 2, 0)) > 0)
    {
        recv_buf[rc] = '\0';
        printf("%s", recv_buf);

        if (!header_parsed)
        {
            // look for empty line("\r\n\r\n")
            char* header_end = strstr(recv_buf, "\r\n\r\n");
            if (header_end)
            {
                header_parsed = 1;
                header_end += 4; // move pointer past "\r\n\r\n"

                // 4. write data without headers in file
                fwrite(header_end, 1, (rc - (header_end - recv_buf)), file);
            }
        }
        else {
            // Headers already parsed, write everything to file
            fwrite(recv_buf, 1, rc, file);
        }
    }


    printf("\nDONE!");
    closesocket(s);

    return 0;
}


//edit to write it in a file
//data after the empty line(we write to the file after the empty line)
