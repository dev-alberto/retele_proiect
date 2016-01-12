#ifndef HEADER_
#define HEADER_

#include <iostream>
#include <cstring> 	// folosit pentu memset.
#include <arpa/inet.h> 	// pentru funtia inet_ntop
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <vector>
#include <list>
#include <iterator>
#include <sstream>
#include <inttypes.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
using namespace std;

// Declaratii funtiile server
int server_start_listen() ;
int server_establish_connection(int server_fd);
int server_send(int fd, string data);
void *tcp_server_read(void *arg) ;

//constantele serverului
const  char * PORT = "12345" ;
const int MAXLEN = 1024 ;   // Lungimea maxima a unui mesaj.
const int MAXFD = 10 ;       // Maximum file descriptors to use. Equals maximum clients. Numarul maxim de descriptori. Este egala cu numarul maxim de clienti
const int BACKLOG = 5 ;     // Numarul maxim de conexiuni care pot astepta inainte de a fi acceptate


//   Trebuie sa fie declarata volatile pentru ca poate fi modificata de un alt thread. Adica un compilator nu poate
//   optimiza codulul pentru ca, este declarat asa fel incat nu numai programul poate schimba variabila dar si programe externe,
//   adica threaduri in cazul nostru

volatile fd_set the_state;

pthread_mutex_t mutex_state = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t boardmutex = PTHREAD_MUTEX_INITIALIZER; // mutex locker pentru vectorul tablei de sah.




// Declaratii functii sah
void newgame() ;
void printeaza_tabla_sah(int clientelnet);
bool isblack(int x , int y) ;
void muta_piesa(char * mutare) ;
void do_gamer_command(char * command, int client) ;
void print_all_and_verify_winner();
int quit(int x);
int winner (int x);
void  do_spectator_command(char * command, int client);
void reseteaza();
bool mutare_valida(char * mutare);
bool mutare_valida_alb(char * mutare);
bool mutare_valida_negru(char * mutare);
int move_validator=0;

vector<vector<string> > chessboard;     //  Folosim un vector 2D pentru tabla de sah

list <string> piese_capturate;                   // Folosim o lista inlantuita pentru a stoca piesele omorate
list <string>::iterator it_capt;     // Folosim un iterator pentru a lucra cu ele

// Culori
const string culoare_text ("\033[22;30m");
const string culoare_abc ("\033[01;32m");
const string culoare_neagra ("\033[22;30m");
const string culoare_alba ("\033[01;30m");
const string culoare_patrat_negru("\033[01;34m") ;
const string culoare_patrat_alb("\033[01;36m") ;

#endif
