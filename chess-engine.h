#ifndef CHESS_
#define CHESS_

#include "Header.h"


void printeaza_tabla_sah(int clientelnet)

//  Functie care trimite tabla de sah la clientii telnet

{
    char char_y ;

    string abc1 =culoare_abc + "  a  b  c  d  e  f  g  h\n" ;
    server_send(clientelnet, "\n" + abc1);

    for (int y=7 ; y>-1 ; y--)
    {
        char_y = y+ 49;         // Convertim int la char.
        string char_y_str("");
        char_y_str += char_y ;  // apoi la string.
        server_send(clientelnet, char_y_str);

        for (int x=0 ; x<8 ; x++)
        {

            if (tablasah[x][y] == "X") // .
            {
                if (isblack(x+1,y+1) )
                {
                    tablasah[x][y] = culoare_patrat_negru + "\u25A0" ;
                }
                else
                    tablasah[x][y] = culoare_patrat_alb + "\u25A1" ;
            }

            server_send(clientelnet, " " + tablasah[x][y] + " ");

        }


        server_send(clientelnet,culoare_abc+ char_y_str+ "\n" );



    }



    server_send(clientelnet, abc1 + "\n\n");

    string temp("Piese capturate :") ;
    for (it_capt = piese_capturate.begin(); it_capt != piese_capturate.end(); ++it_capt)
        temp +=   *it_capt ;

    server_send(clientelnet, temp + culoare_text + "\n");
}


bool isblack(int x, int y) //  Fuctie care verifica daca o patratica e neagra sau nu
{
    bool black = false;
    if (((x & 1) && (y & 1)) || (!(x & 1) && !(y & 1))) black= true  ;
    return black;
}

void muta_piesa(char * mutare)  //  Functie care executa miscarile pe tabla de sah
{

    bool accept  = true ;  //Returneaza true daca o miscare este acceptata

    int a,b,c,d ;

    a = mutare[0] -97;
    b = mutare[1] -49 ;
    c = mutare[2] -97;
    d = mutare[3] -49;


   if (tablasah [a][b] ==culoare_patrat_negru + "\u25A0" || tablasah [a][b] ==culoare_patrat_alb + "\u25A1")
    {
       accept = false ;
    }
    else
    {

    // Facem mutarea
        pthread_mutex_lock(&mutextabla);  //  Blocam aici ca sa nu avem "race conditions"

        if (tablasah[c][d] != "X" && tablasah[c][d] != culoare_patrat_negru +"\u25A0"  && tablasah[c][d] != culoare_patrat_alb +"\u25A1")
            piese_capturate.insert(piese_capturate.begin() ,tablasah[c][d] ) ;
        tablasah[c][d] = tablasah [a][b] ;
        tablasah[a][b] = "X" ;

        pthread_mutex_unlock(&mutextabla); // deblocam
    }

}

void  do_gamer_command(char * command, int client) // functie pt comenzile jucatorilor
{

    if (command[0]== '1')printeaza_tabla_sah(client) ;  // Apare tabla de sah

    if (command[0]== '9')reseteaza() ;                // Reseteaza tabla de sah

    if(command[0]== '2') quit(client);              // deconecteaza un client

        // Verificam daca comanda este o comanda valida de miscare
  if (command[0] >= 'a' && command[0] <= 'h' && command[1] >= '1' && command[1] <= '8' && command[2] >= 'a' && command[2] <= 'h' && command[3] >= '1' && command[3] <= '8')
  {

   if(client==4 && mutare_valida(command) &&  mutare_valida_alb(command) && move_validator%2==0)
   {
    muta_piesa(command);
    move_validator++;
    printeaza_la_toti_si_verifica_castigator();
    server_send(client,  "\nMutare:");
    server_send(client, command);
    server_send(client,  "\n");
   }
   else
   {
    if(client==4 && mutare_valida(command) && move_validator%2==1)
        server_send(client,"nu este randul dumneavoastra sa mutati\n");
    if(client==4 && mutare_valida(command)==false && move_validator%2==0)
        server_send(client,"mutarea nu este valida,mai incercati\n");
    if(client==4 && mutare_valida(command) &&  mutare_valida_alb(command)==false && move_validator%2==0)
    server_send(client,"nu puteti muta piesa adversa\n");
   }

     if(client==5 && mutare_valida(command) && mutare_valida_negru(command) && move_validator%2==1)
   {
    muta_piesa(command);
    move_validator++;
    printeaza_la_toti_si_verifica_castigator();
    server_send(client,  "\nMutatare:");
    server_send(client, command);
    server_send(client,  "\n");
   }
   else
   {
    if(client==5 && mutare_valida(command) && move_validator%2==0)
        server_send(client,"nu este randul dumneavoastra sa mutati\n");
    if(client==5 && mutare_valida(command)==false && move_validator%2==1)
        server_send(client,"mutarea nu este valida,mai incercati\n");
    if (client==5 && mutare_valida(command) && mutare_valida_negru(command)==false && move_validator%2==1)
    server_send(client,"nu puteti muta piesa adversa\n");
   }
  }

}

void  do_spectator_command(char * command, int client) // functie pentru comenzile spectatorilor
{

    if (command[0]== '1')printeaza_tabla_sah(client) ;  // Apare tabla de sah

    if(command[0]== '2') quit(client);              // deconecteaza un client

        // Verificam daca comanda este o comanda valida de miscare
    if (command[0] >= 'a' && command[0] <= 'h' && command[1] >= '1' && command[1] <= '8' && command[2] >= 'a' && command[2] <= 'h' && command[3] >= '1' && command[3] <= '8')
    {
        server_send(client,"Nu puteti face miscari, sunteti doar spectator");
    }

}

void newgame()  //fuctie care seteaza tbla de sah la starea ei initiala
{
    pthread_mutex_lock(&mutextabla); // blocam mutexul

    for (int y=7 ; y>-1 ; y--)
    {
        for (int x=0 ; x<8 ; x++)
        {
            tablasah[x][y] = "X" ; //  X reprezinta spatiu gol
        }
    }

    //aici generam piesele de sah

    tablasah[7][7] = culoare_neagra + "\u265C";
    tablasah[6][7] = culoare_neagra + "\u265E";
    tablasah[5][7] = culoare_neagra + "\u265D";
    tablasah[4][7] = culoare_neagra + "\u265A";
    tablasah[3][7] = culoare_neagra + "\u265B";
    tablasah[2][7] = culoare_neagra + "\u265D";
    tablasah[1][7] = culoare_neagra + "\u265E";
    tablasah[0][7] = culoare_neagra + "\u265C";
    for (int x=0; x<8 ; x++)
        tablasah[x][6] = culoare_neagra + "\u265F";


    tablasah[7][0] = culoare_alba +"\u2656";
    tablasah[6][0] = culoare_alba +"\u2658";
    tablasah[5][0] = culoare_alba +"\u2657";
    tablasah[4][0] = culoare_alba +"\u2654";
    tablasah[3][0] = culoare_alba +"\u2655";
    tablasah[2][0] = culoare_alba +"\u2657";
    tablasah[1][0] = culoare_alba +"\u2658";
    tablasah[0][0] = culoare_alba +"\u2656";
    for (int x=0; x<8 ; x++)
        tablasah[x][1] = culoare_alba + "\u2659";
piese_capturate.clear();
    pthread_mutex_unlock(&mutextabla); // deblocam mutexul
}


void   printeaza_la_toti_si_verifica_castigator()
// Printam tabla la toti clientii si verificam conditia de castigare a jocului
{
    for (int wdf = 3; wdf < MAXFD; ++wdf)
    {
        if (FD_ISSET(wdf, &the_state)) //verfica daca descriptorul de fisier apartine multimii
        {
            printeaza_tabla_sah(wdf);
            winner(wdf);
        }

    }
}

int quit(int x)
{
    cout<< "Programul si-a terminat executia"<< endl;
    close(x);
    return 0;
}


int winner(int x)
{
    int white_king=0;
    int black_king=0;
    for (int y=7 ; y>-1 ; y--)
    {
        for (int x=0 ; x<8 ; x++)
        {
            if(tablasah[x][y]==culoare_alba +"\u2654")
                white_king++;
            else if(tablasah[x][y]==culoare_neagra + "\u265A")
                black_king++;
        }
    }
    if(white_king==0)
    {
        cout<<"Negru castiga!"<<endl;
        close(x);
    }
    else if(black_king==0)
    {
        cout<<"Alb castiga!"<<endl;
        close(x);
    }
  //  if (white_king==0 || black_king==0)
        //newgame();
    return 0;
}
#endif

void reseteaza()
{
newgame();
move_validator=0;
for (int wdf = 3; wdf < MAXFD; ++wdf)
    {
        if (FD_ISSET(wdf, &the_state)) //verfica daca descriptorul de fisier apartine multimii
        {
            printeaza_tabla_sah(wdf);
        }
    }
}

bool mutare_valida(char * mutare)
{
    bool accept  = true ;  //Returneaza true daca o miscare este acceptata

    int a,b,c,d ;

    a = mutare[0] -97;
    b = mutare[1] -49 ;
    c = mutare[2] -97;
    d = mutare[3] -49;


   if (tablasah [a][b] ==culoare_patrat_negru + "\u25A0" || tablasah [a][b] ==culoare_patrat_alb + "\u25A1")
    {
       accept = false ;
    }
    else
    {

        // Facem mutarea
        pthread_mutex_lock(&mutextabla);  // Blocam aici ca sa nu avem "race conditions"

        if (tablasah[c][d] != "X" && tablasah[c][d] != culoare_patrat_negru +"\u25A0"  && tablasah[c][d] != culoare_patrat_alb +"\u25A1")
            accept=true;

        pthread_mutex_unlock(&mutextabla); // deblocam
    }

    return accept;
}

bool mutare_valida_alb(char * mutare)
{
  bool accept=true;

  int a,b,c,d ;

  a = mutare[0] -97;
  b = mutare[1] -49;
  c = mutare[2] -97;
  d = mutare[3] -49;

  if(tablasah[a][b]==culoare_neagra + "\u265C" || tablasah[a][b]==culoare_neagra + "\u265E"  || tablasah[a][b]==culoare_neagra + "\u265D"  || tablasah[a][b]==culoare_neagra + "\u265A"  ||  tablasah[a][b]== culoare_neagra + "\u265B" || tablasah[a][b] ==culoare_neagra + "\u265F")
  accept=false;
  return accept;
}

bool mutare_valida_negru(char * mutare)
{
  bool accept=true;

  int a,b,c,d;

  a = mutare[0] -97;
  b = mutare[1] -49;
  c = mutare[2] -97;
  d = mutare[3] -49;

  if(tablasah[a][b]==culoare_alba +"\u2656" || tablasah[a][b]==culoare_alba +"\u2658"  || tablasah[a][b]==culoare_alba +"\u2657"  || tablasah[a][b]==culoare_alba +"\u2654"  ||  tablasah[a][b]== culoare_alba +"\u2655" || tablasah[a][b] ==culoare_alba + "\u2659")
  accept=false;
  return accept;
}
