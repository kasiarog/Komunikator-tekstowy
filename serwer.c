// SERWER

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <signal.h>

#define DLUGOSC_WIADOMOSCI 300
#define ROZMIAR_STRUCTA 326
#define KLUCZ_KOLEJKI_SERWERA 12345


typedef struct zapytanie{
    int id_nadawcy, id_adresata, error, polecenie;    //  16 bajtów   // id_nadawcy - id kolejki z pid procesu nadawcy jako kulcz
    char wiadomosc[DLUGOSC_WIADOMOSCI], nick[10];   // 110 bajtów
}zapytanie;
// dlugość tego structa - 126 bajtów

typedef struct klient{
    char nick[10];
    int id_kolejki, haslo;  // haslo nie może zaczynać się od 0
}klient;

typedef struct grupa{  // grupa 0 ma w sobie wszystkich użytkowników - @everyone
    int id_grupy, ile_czlonkow;
    char nazwa[10];
    struct klient tab_czlonkow[100];   //tablica trzymająca członków grupy
}grupa;



// wczytywanie pliku konfiguracyjnego
grupa plik(FILE *file, grupa *tablica_grup){
    tablica_grup[0].id_grupy = 0;

    if(file == NULL)
        printf("Plik jest pusty, lub nie istnieje");

    for(int i=0; i<50; i++){   
        tablica_grup[i].ile_czlonkow = -1;  // ustawiam domyślnie -1, żeby móc potem znaleźć miejsce, gdzie kończą sie zapisane grupy
    }

    for(int i=0; i<9; i++){
        char nick1[10];
        int haslo1;
        fscanf(file, "%s %d", nick1, &haslo1);
        strcpy(tablica_grup[0].tab_czlonkow[i].nick, nick1);
        tablica_grup[0].tab_czlonkow[i].haslo = haslo1;
        tablica_grup[0].ile_czlonkow = i+1;
    }

    for(int i=2; i<5; i++){
        int id_gr, ile_wgr;
        char nick1[10], nazw[10];
        fscanf(file, "%d %d %s", &id_gr, &ile_wgr, nazw);
        tablica_grup[i].id_grupy = id_gr;
        tablica_grup[i].ile_czlonkow = ile_wgr;
        strcpy(tablica_grup[i].nazwa, nazw);

        for (int j=0; j<tablica_grup[i].ile_czlonkow; j++){
            fscanf(file, "%s", nick1);
            strcpy(tablica_grup[i].tab_czlonkow[j].nick, nick1);
            for(int k=0; k<tablica_grup[0].ile_czlonkow; k++){
                if (strcmp(tablica_grup[0].tab_czlonkow[k].nick, tablica_grup[i].tab_czlonkow[j].nick)==0){
                    tablica_grup[i].tab_czlonkow[j].haslo = tablica_grup[0].tab_czlonkow[k].haslo;
                    break;
                }
            }
        }
    }

    tablica_grup[1].ile_czlonkow = 0;   // tablica_grup[1] - zalogowani użytkownicy

    return *tablica_grup;
}


int odpowiedz_serwera(zapytanie odebrany, int error, zapytanie odpowiedz){

    odpowiedz.polecenie = odebrany.polecenie;
    odpowiedz.id_adresata = odebrany.id_nadawcy;
    odpowiedz.id_nadawcy = msgget(KLUCZ_KOLEJKI_SERWERA, IPC_CREAT|0644);  // wysyłana wiadomość jest od serwera
    odpowiedz.error = error;
    strcpy(odpowiedz.nick, "serwer");

    switch(error){
        case 0:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d powiodło się :)", odebrany.polecenie);
            break;
        case 1:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d nie powiodło się :(\nUżytkownik jest już w tej grupie.", odebrany.polecenie);
            break;
        case 2:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d nie powiodło się :(\nNie ma takiej grupy.", odebrany.polecenie);
            break;
        case 3:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d nie powiodło się :(\nUżytkownika nie ma w tej grupie.", odebrany.polecenie);
            break;
        case 4:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d nie powiodło się :(\nNie jesteś zapisany do tej grupy, więc nie możesz wysłać tam wiadomości.", odebrany.polecenie);
            break;
        default:
            sprintf(odpowiedz.wiadomosc, "Wykonanie polecenia %d nie powiodło się :(\nSpróbuj jeszcze raz.", odebrany.polecenie);
    }
   
    int success = msgsnd(odpowiedz.id_adresata, &odpowiedz, ROZMIAR_STRUCTA, 0);  // wysyłany jest struct z odpowiedzia 

    return success;
}


// przy rejestracji dodaje się nowego użytkownika do tablica_grup[0]
int Rejestracja(zapytanie odebrany, grupa *tablica_grup){
        
    int ktory_z_kolei = tablica_grup[0].ile_czlonkow;  // indeks jaki będzie miał w tablica_grup[0] nowy użytkownik

    strcpy(tablica_grup[0].tab_czlonkow[ktory_z_kolei].nick, odebrany.nick);
    tablica_grup[0].tab_czlonkow[ktory_z_kolei].haslo = odebrany.id_adresata; // w wiadomości hasło będzie pod zmienną klucz_adresata
    tablica_grup[0].tab_czlonkow[ktory_z_kolei].id_kolejki = odebrany.id_nadawcy; // klucz nadawcy = jego id = jego pid() który będzie przekazywany przez klienta

    tablica_grup[0].ile_czlonkow += 1;

    return 0;
}


int Login(zapytanie odebrany, grupa *tablica_grup){
    klient uzytkownik;
    int success = 0, czy = 1, ktory_z_kolei = tablica_grup[1].ile_czlonkow;


    for (int i=0; i<tablica_grup[0].ile_czlonkow; i++){ // przejście przez wszystkich użytkowinków w bazie, w celu zweryfikowania danych
        if (strcmp(tablica_grup[0].tab_czlonkow[i].nick, odebrany.nick) == 0){  // sprawdzenie loginu
            if (tablica_grup[0].tab_czlonkow[i].haslo == odebrany.id_adresata){ // haslo jest w polu  id_adresata

                for(int x=0; x<tablica_grup[1].ile_czlonkow; x++){  // sprawdzenie, czy użytkownik nie jest już zalogowany
                    if (strcmp(odebrany.nick, tablica_grup[1].tab_czlonkow[x].nick) == 0){
                        czy = 0;
                        return 6;
                    }
                    else{
                        czy = 1;
                    }
                }

                if(czy){
                    strcpy(uzytkownik.nick, odebrany.nick);
                    uzytkownik.haslo = odebrany.id_adresata;
                    uzytkownik.id_kolejki = odebrany.id_nadawcy;

                    tablica_grup[1].tab_czlonkow[ktory_z_kolei] = uzytkownik;   // tablica_grup[1] - zalogowani
                    tablica_grup[1].ile_czlonkow += 1;
                    success = 0;
                }

                // dodwanie danych klienta do grup w których jest
                for (int j=0; j<50; j++){
                    if (tablica_grup[j].ile_czlonkow != -1 && j != 1){
                        for (int k=0; k<tablica_grup[j].ile_czlonkow; k++){
                            if( strcmp(uzytkownik.nick, tablica_grup[j].tab_czlonkow[k].nick) == 0 ){
                                tablica_grup[j].tab_czlonkow[k].id_kolejki = uzytkownik.id_kolejki;
                                success = 0;
                            }
                            
                        }
                    }
                }
                break;

            }
            else{
                success = 4;
            }
        }
        else{
            success = 5;
        }
    }

    return success;

}


int Logout(zapytanie odebrany, grupa *tablica_grup){
    int error = 0, delete = 0, i;  // zmienna mówiąca o errorach: -1 nie odnaleziono użytkownika, ktory chce się wylogować
    for(i=0; i<tablica_grup[1].ile_czlonkow; i++){

        if(tablica_grup[1].tab_czlonkow[i].id_kolejki == odebrany.id_nadawcy){
            for (int j=i; j<tablica_grup[1].ile_czlonkow - 1; j++){
                tablica_grup[1].tab_czlonkow[j] = tablica_grup[1].tab_czlonkow[j+1];
            }

            //usuwanie ostatniego uzytkownika
            int index_to_delete = tablica_grup[1].ile_czlonkow - 1;
            strcpy(tablica_grup[1].tab_czlonkow[index_to_delete].nick, "");
            tablica_grup[1].tab_czlonkow[index_to_delete].haslo = 0;
            tablica_grup[1].tab_czlonkow[index_to_delete].id_kolejki = 0;
            tablica_grup[1].ile_czlonkow -= 1;

            delete = msgctl(tablica_grup[1].tab_czlonkow[i].id_kolejki, IPC_RMID, NULL);
            error = 0;
            break;
        }
        else{
            error = 1;  // nie znaleziono podanego użytkownika
        }
    }
 
    if (error == 0){    // dlatego że jeśli istnieje już jakiś error, to nie nadpisujemy go
        error = delete;
    }

    return error;
}


int przeslij_wiadomosc(zapytanie odebrany, grupa *tablica_grup, zapytanie odpowiedz){

    int success = -1;
    odpowiedz.id_nadawcy = odebrany.id_nadawcy;
    odpowiedz.id_adresata = odebrany.id_adresata;
    odpowiedz.polecenie = odebrany.polecenie;
    strcpy(odpowiedz.wiadomosc, odebrany.wiadomosc);

    for (int i=0; i<tablica_grup[1].ile_czlonkow; i++) {  // przeszukujemy wszystich zalogowanych użytkowników

        if (odpowiedz.id_nadawcy == tablica_grup[1].tab_czlonkow[i].id_kolejki){
            strcpy(odpowiedz.nick, tablica_grup[1].tab_czlonkow[i].nick);
            break;
        }
    }

    for (int i=0; i<tablica_grup[1].ile_czlonkow; i++) {  // przeszukujemy wszystich zalogowanych użytkowników
        
        if (odpowiedz.id_adresata == tablica_grup[1].tab_czlonkow[i].id_kolejki){
            success = msgsnd(odpowiedz.id_adresata, &odpowiedz, ROZMIAR_STRUCTA, 0);
            break;
        }
    }

    return success;
}


int przeslij_wiadomosc_do_grupy(zapytanie odebrany, grupa *tablica_grup, zapytanie odpowiedz){

    int success = -1, czy = 0; // czy - mówi o tym czy użytkownik jest w grupie której chce coś wysłać
    odpowiedz.polecenie = odebrany.polecenie;
    odpowiedz.id_nadawcy = odebrany.id_nadawcy;
    strcpy(odpowiedz.wiadomosc, odebrany.wiadomosc);

    for (int x=0; x<tablica_grup[1].ile_czlonkow; x++) {  // przeszukujemy wszystich zalogowanych użytkowników

        if (odebrany.id_nadawcy == tablica_grup[1].tab_czlonkow[x].id_kolejki){
            strcpy(odpowiedz.nick, tablica_grup[1].tab_czlonkow[x].nick);
            break;
        }
    }
    
    int i = 2;
    while (tablica_grup[i].ile_czlonkow != -1){    // przeszukujemy wszystie grupy (ten while po to, aby dojśc maks do ostatniej grupy)

        if (odebrany.id_adresata == tablica_grup[i].id_grupy){

            for(int j=0; j<tablica_grup[i].ile_czlonkow; j++){  // sprawdzamy czy użytkownik jest w grupie do której wysyła wiadomość
                if(odebrany.id_nadawcy == tablica_grup[i].tab_czlonkow[j].id_kolejki){
                    czy = 1;
                    break;
                }
            }

            if(czy){
                for(int j=0; j<tablica_grup[i].ile_czlonkow; j++){
                    for(int k=0; k<tablica_grup[1].ile_czlonkow; k++){

                        // sprawdzamy czy użytkownik do którego wysyłamy jest w bazie zalogowanych
                        if(tablica_grup[1].tab_czlonkow[k].id_kolejki == tablica_grup[i].tab_czlonkow[j].id_kolejki){
                            if(tablica_grup[1].tab_czlonkow[k].id_kolejki != odebrany.id_nadawcy){  // dodane żeby nie wysłać wiadomości do samego siebie
                                
                                odpowiedz.id_adresata = tablica_grup[i].tab_czlonkow[j].id_kolejki;
                                if(odpowiedz.id_adresata > 0){   // upewniamy się czy adresat jest zalogowany
                                    success = msgsnd(odpowiedz.id_adresata, &odpowiedz, ROZMIAR_STRUCTA, 0);
                                }
                            }
                        }
                    }
                }
            }
            else{
                success = 4;
            }
            break;
        }
        else{
            success = -1;
        }
        i++; 
    }
    
    return success;
}


// funkcja zwracająca listę zalogowanych osób
int KtoTam(zapytanie odebrany, grupa *tablica_grup, zapytanie odpowiedz){
    int success = -1;
    char uzytkownik[50];
    odpowiedz.id_nadawcy = odebrany.id_adresata;  
    odpowiedz.id_adresata  = odebrany.id_nadawcy;
    strcpy(odpowiedz.nick, "serwer");

    strcpy(odpowiedz.wiadomosc, "Lista zalogowanych użytkowników: \n");

    for(int i=0; i<tablica_grup[1].ile_czlonkow; i++){
        strcpy(uzytkownik, "");

        sprintf(uzytkownik, "nick: %s, id: %d\n", tablica_grup[1].tab_czlonkow[i].nick, tablica_grup[1].tab_czlonkow[i].id_kolejki);
        strcat(odpowiedz.wiadomosc, uzytkownik);
    }
    success = msgsnd(odpowiedz.id_adresata, &odpowiedz, ROZMIAR_STRUCTA, 0); 

    return success;
}


int KtoTamWGrupie(zapytanie odebrany, grupa *tablica_grup, zapytanie odpowiedz){
    odpowiedz.error = odebrany.error;  // w tej zmiennej przechowywany jest numer grupy
    
    odpowiedz.id_adresata = odebrany.id_nadawcy;
    odpowiedz.id_nadawcy = odebrany.id_adresata;
    odpowiedz.polecenie = odebrany.polecenie;
    strcpy(odpowiedz.nick, "serwer");

    if (odebrany.error == 0 || odebrany.error == 1 || odebrany.error > 4){
        return 2;
    }

    char komunikat[70];
    sprintf(komunikat, "Lista użytkowników w grupie %d: \n", odpowiedz.error);
    strcpy(odpowiedz.wiadomosc, komunikat);

    for(int i=0; i<tablica_grup[odpowiedz.error].ile_czlonkow; i++){
        char uzytkownik[50];
        sprintf(uzytkownik, " - nick: %s, id: %d\n", tablica_grup[odebrany.error].tab_czlonkow[i].nick, tablica_grup[odebrany.error].tab_czlonkow[i].id_kolejki);
        strcat(odpowiedz.wiadomosc, uzytkownik);
    }

    int success = msgsnd(odebrany.id_nadawcy, &odpowiedz, ROZMIAR_STRUCTA, 0); 

    return success;
}


int DopiszDoGrupy(zapytanie odebrany, grupa *tablica_grup){
    int id_grupy = odebrany.error;
    int index = tablica_grup[id_grupy].ile_czlonkow;    // indeks na który trzeba zapisać nowego członka grupy
    int success = -1;

    if(id_grupy == 0 || id_grupy == 1 || id_grupy > 4){
        return 2;
    }

    for(int j=0; j<tablica_grup[id_grupy].ile_czlonkow; j++){
        if (odebrany.id_nadawcy == tablica_grup[id_grupy].tab_czlonkow[j].id_kolejki ){
            return 1;    
        }
    }

    for(int i=0; i<tablica_grup[1].ile_czlonkow; i++){   // wyszukiwanie użytkownika w bazie
        if(odebrany.id_nadawcy ==  tablica_grup[1].tab_czlonkow[i].id_kolejki){
            
            tablica_grup[id_grupy].ile_czlonkow += 1;
            tablica_grup[id_grupy].tab_czlonkow[index].haslo = tablica_grup[1].tab_czlonkow[i].haslo;
            tablica_grup[id_grupy].tab_czlonkow[index].id_kolejki = tablica_grup[1].tab_czlonkow[i].id_kolejki;
            strcpy(tablica_grup[id_grupy].tab_czlonkow[index].nick, tablica_grup[1].tab_czlonkow[i].nick);

            success = 0;    // poprawnie znaleziono klienta w bazie
            break;
        }
    }    

    return success;
}


int WypiszZGrupy(zapytanie odebrany, grupa *tablica_grup){
    int id_grupy = odebrany.error;    // id grupy przechowywane jest w errorze
    int success = 0;

    if(id_grupy == 0 || id_grupy == 1 || id_grupy > 4){
        return 2;
    }

    for(int j=0; j<tablica_grup[id_grupy].ile_czlonkow; j++){
        if (odebrany.id_nadawcy != tablica_grup[id_grupy].tab_czlonkow[j].id_kolejki ){
            success = 3;
        }
        else{
            success = 0; 
            break;
        }
    }

    for(int i=0; i<tablica_grup[id_grupy].ile_czlonkow; i++){
        
        if(odebrany.id_nadawcy == tablica_grup[id_grupy].tab_czlonkow[i].id_kolejki){
            for(int j=i; j<tablica_grup[id_grupy].ile_czlonkow - 1; j++){
                tablica_grup[id_grupy].tab_czlonkow[j] = tablica_grup[id_grupy].tab_czlonkow[j+1];
            }

            int index_to_delete = tablica_grup[id_grupy].ile_czlonkow - 1;
            strcpy(tablica_grup[id_grupy].tab_czlonkow[index_to_delete].nick, "");
            tablica_grup[id_grupy].tab_czlonkow[index_to_delete].haslo = 0;
            tablica_grup[id_grupy].tab_czlonkow[index_to_delete].id_kolejki = 0;
            tablica_grup[id_grupy].ile_czlonkow -= 1;

            success = 0;
            break;
        }
        else{
            success = -1;  // nie znaleziono podanego użytkownika
        }
    }
    
    return success;
}


int JakieGrupy(zapytanie odebrany, grupa *tablica_grup, zapytanie odpowiedz){
    int success = -1;
    odpowiedz.id_nadawcy = odebrany.id_adresata;  
    odpowiedz.id_adresata = odebrany.id_nadawcy;
    strcpy(odpowiedz.nick, "serwer");

    strcpy(odpowiedz.wiadomosc, "Lista dostępnych grup: \n");

    for(int i=2; i<50; i++){
        if(tablica_grup[i].ile_czlonkow != -1){
            char gruppa[50];
            sprintf(gruppa, "id grupy: %d, nazwa grupy: %s\n", tablica_grup[i].id_grupy, tablica_grup[i].nazwa);
            strcat(odpowiedz.wiadomosc, gruppa);
        }
        else{
            break;
        }
    }
    success = msgsnd(odpowiedz.id_adresata, &odpowiedz, ROZMIAR_STRUCTA, 0);

    return success;
}


int Sprzataj(grupa *tablica_grup){
    int success = 0, success2 = 0, success3 = 0;
    int error = 0;  // error przyjmie wartość -2, gdy którejkolwiek z kolejek nie uda się usunąć

    // tablica_grup[1] - zalogowani
    for(int i=0; i<tablica_grup[1].ile_czlonkow; i++){
        tablica_grup[1].tab_czlonkow[i].haslo = 0;
        tablica_grup[1].tab_czlonkow[i].id_kolejki = 0;
        strcpy(tablica_grup[1].tab_czlonkow[i].nick, "");
        tablica_grup[1].ile_czlonkow -= 1;

        success = msgctl(tablica_grup[1].tab_czlonkow[i].id_kolejki, IPC_RMID, NULL);
        if (success != 0){
            success2 = -1;
        }
    }

    int id_serwera = msgget(KLUCZ_KOLEJKI_SERWERA, IPC_CREAT|0644);
    success3 = msgctl(id_serwera, IPC_RMID, NULL);

    if (success2 != 0 || success3 != 0){ 
        error = -1;
    }
    
    return error;
}



int main(){
    FILE *fp; 
    fp = fopen("plik_konfiguracyjny.txt", "r");

    int ID_KOLEJKI_DO_SERWERA = msgget(KLUCZ_KOLEJKI_SERWERA, IPC_CREAT|0644);

    grupa *tablica_grup = (grupa*)malloc(50*sizeof(grupa));
    *tablica_grup = plik(fp, tablica_grup);
    
    
    int rozmiar = 0, typ = -1, retu; // typ - polecenie z zapytania, retu - powodzenie zapytania  
    zapytanie odebrana;

    while(typ != 99)   //typ = 99 oznacza zamykanie serwera
    {
        rozmiar = msgrcv(ID_KOLEJKI_DO_SERWERA, &odebrana, ROZMIAR_STRUCTA, 0, IPC_NOWAIT);

        if (rozmiar > 0){
            // printf("Rozmiar: %d", rozmiar);

            zapytanie odpowiedz;
            typ = odebrana.polecenie;
            
            switch (typ) {
                case 0:
                    retu = Logout(odebrana, tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 1:
                    retu = przeslij_wiadomosc(odebrana, tablica_grup, odpowiedz);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 2:
                    retu = przeslij_wiadomosc_do_grupy(odebrana, tablica_grup, odpowiedz);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 3:
                    retu = KtoTam(odebrana, tablica_grup, odpowiedz);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 4:
                    retu = JakieGrupy(odebrana, tablica_grup, odpowiedz);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 5:
                    retu = DopiszDoGrupy(odebrana, tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 6:
                    retu = WypiszZGrupy(odebrana, tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 7:
                    retu = KtoTamWGrupie(odebrana, tablica_grup, odpowiedz);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 8:
                    retu = Rejestracja(odebrana, tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);  // zwraca nadawcy info zwrotne
                    break;
                case 9:
                    retu = Login(odebrana, tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
                case 99:
                    retu = Sprzataj(tablica_grup);
                    odpowiedz_serwera(odebrana, retu, odpowiedz);
                    break;
            }
           
        }
    }
    
    free(tablica_grup);
    fclose(fp);
    return 0;
}
