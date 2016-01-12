#include "chess-engine.h"

int main()
{

    chessboard.resize(8);
    for (int i = 0; i < 8; ++i)
        chessboard[i].resize(8);

     newgame(); // Umplem tabela cu piese

    cout << "Serverul a pornit"  << endl ;

    // Punem serverul sa asculte la portul 12345

    int server_fd = server_listen() ;
    if (server_fd == -1)
    {
        cout << "Eroare, inchid programul" ;
        return 1 ;
    }

    string gamer_msg =("Bine ati venit la acest server de sah.\n Sunteti jucator. Comenzile valide sunt:\n 1 pentru a printa tabla de joc, 2 pentru a iesi din joc\nsau '9' pentru a reseta tabla.\nPentru a misca piesele de joc, tastati coordonatele de pe tabla\nDe exemplu 'a2a4'\n");
    string spactator_msg=("Bine ati venit la aceasta server de sah.\n Puteti doar asista la joc sau iesi din joc.\n Apasati 1 pentru a asista sau 2 pentru a iesi\n");
    pthread_t threads[MAXFD]; // cream handles pt threaduri

    FD_ZERO(&the_state);

    while(1) // incepem bucla
    {
        int rfd;
        void *arg;

        //  Daca un client incearca sa se conecteze, il conectam si cream un fd pentru el
        rfd = connect_to_server(server_fd);

        if (rfd >=0)
        {
            cout << "Client conectat, bine ati venit, aveti descriptorul: " << rfd << endl;
            if (rfd > MAXFD)
            {
                cout << "Prea multi clienti sunt deja conectati." << endl;
                close(rfd);
                continue;
            }

         pthread_mutex_lock(&mlock);  // Ne asiguram ca 2 threaduri nu pot crea un fd in acelasi timp

          FD_SET(rfd, &the_state);  //  Adaugam un fd la multimea FD-urilor

          pthread_mutex_unlock(&mlock); // deblocam mutex

        arg = (void *) rfd;
                  //  Cream un nou thread pentru client care intercepteaza toate datele de la client
        pthread_create(&threads[rfd], NULL, treat, arg);
      }
        if(rfd>5)
            server_send(rfd, spactator_msg); // trimitem mesaj cu instructiuni
        else
            server_send(rfd, gamer_msg);

    }
    return 0;
}

int server_listen()
{

struct addrinfo hostinfo, *res;

int sock_fd;

int server_fd; //  fd-ul la care serverul asculta
int ret;
int yes = 1;

//   mai intai incarcam adresele struct cu getaddrinfo():

memset(&hostinfo, 0, sizeof(hostinfo));

hostinfo.ai_family = AF_UNSPEC;  //  folosim IPv4 sau IPv6, nu conteaza
hostinfo.ai_socktype = SOCK_STREAM;
hostinfo.ai_flags = AI_PASSIVE;     // completeaza IP-ul pentru mine

getaddrinfo(NULL, PORT, &hostinfo, &res);


    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(server_fd < 0)
    {
      perror("Eroare la socket()\n");
      return errno;
    }

    // Previne eroarea: "Error Address already in use"
    ret = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
     if(ret < 0)
     {
       perror("Eroare la setsockopt()\n");
       return errno;
     }

    ret = bind(server_fd, res->ai_addr, res->ai_addrlen);

    if(ret != 0)
    {
        cout << "error :" << strerror(errno) << endl;
        return -1 ;
    }

    ret = listen(server_fd, BACKLOG);
    if(ret < 0)
    {
      perror("Eroare la listen()\n");
      return errno;
    }



return server_fd;

}

int connect_to_server(int server_fd)
//   Aceasta functie va face conexiunea dintre server si client. Se va executa pentru fiecare
//   client nou care se conecteaza la server. Va returna filedescripterul socketului care citeste
//   datele de la clienti sau va returna o eroare.
{
    char ipstr[INET6_ADDRSTRLEN];
    int port;


    int new_sd;
    struct sockaddr_storage remote_info ;
    socklen_t addr_size;

    addr_size = sizeof(addr_size);
    new_sd = accept(server_fd, (struct sockaddr *) &remote_info, &addr_size);
    if (new_sd < 0)
    {
      perror("Eroare la accept()\n");
      return errno;
    }

    getpeername(new_sd, (struct sockaddr*)&remote_info, &addr_size);

   // Merge atat pentru IPv4 cat si pentru IPv6:
if (remote_info.ss_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&remote_info;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
} else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&remote_info;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
}

std::cout << "Conexiune acceptata de la "  << ipstr <<  " folosind portul " << port << endl;

    return new_sd;

}

int server_send(int fd, string data)
//  functie care trimite date la clienti.

{
    int ret;

    ret = send(fd, data.c_str(), strlen(data.c_str()),0);
    if(ret != strlen(data.c_str()))
    {
      perror("Eroare");
      return errno;
    }
    return 0;
}

void *treat(void *arg)
// Aceasta functie ruleaza intr-un thread pentru fiecare client si citeste datele trimise.
// De asemenea trimite datele la ceilalti clienti

{
    int rfd;
    int wfd;

    char buffer[MAXLEN];
    int buflen;

    rfd = (intptr_t)arg;
    for(;;)
    {
        // Citeste mesajele trimise
        buflen = read(rfd, buffer, sizeof(buffer));
        if (buflen <= 0)
        {
            cout << "Client deconectat. Eliberam fd. " << rfd << endl ;
            pthread_mutex_lock(&mlock);
            FD_CLR(rfd, &the_state);      // free fd's de la ceilalti clienti
            pthread_mutex_unlock(&mlock);
            close(rfd);
            pthread_exit(NULL);
        }

        // Trimite date la toti ceilalti clienti conectati
        pthread_mutex_lock(&mlock);

        for (wfd = 3; wfd < MAXFD; ++wfd)
        {
            if (FD_ISSET(wfd, &the_state) && (rfd != wfd))
            {
                  // add the users FD to the message to give it an unique ID.
                    string userfd;
                    stringstream out;
                    out << wfd;
                    userfd = out.str();
                    userfd = userfd + ": ";
                server_send(wfd, userfd);
                server_send(wfd, buffer);;
            }

        }

        pthread_mutex_unlock(&mlock);

        if(rfd>5)
                do_spectator_command(buffer,rfd);
            else
               do_gamer_command(buffer,rfd);
    }
    return NULL;
}
