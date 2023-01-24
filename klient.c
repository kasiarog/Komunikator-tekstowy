// KLIENT

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
#define ILE_MAX_UZYTKOWNIKOW 13


typedef struct zapytanie{
    int id_nadawcy, id_adresata, error, polecenie;    //  16 bajtów
    char wiadomosc[DLUGOSC_WIADOMOSCI], nick[10];   // 110 bajtów
}zapytanie;
// razem 126 bajtów


int main(){
    int ID_KOLEJKI_DO_SERWERA = msgget(KLUCZ_KOLEJKI_SERWERA, IPC_CREAT|0644);
    int MyID = msgget((getpid()*10), IPC_CREAT|0644); // id kolejki, która tworzy uzytkownik = id uzytkownika

    // Pętla zewnętrzna - oczekiwanie na zalogowanie
    int input, haslo, success;
    bool zalogowany = false;

    while(zalogowany == false) {
        zapytanie login, odebrana;
        char slowo[10];
        fflush(stdin);
        printf("\nWitaj użytkowniku! \n"
            "Zaloguj się proszę na swoje konto, a jeśli go nie masz, zarejestruj się. \n\n"
            "Obsługa interfejsu polega na wpisywaniu odpowiednich numerów dostępnych poleceń. Jeśli chcesz wykonać czynności: \n"
            "1. Zaloguj się (wpisana przez ciebie liczba to 1) \n"
            "2. Zarejestruj się, jeśli nie masz jeszcze konta (wpisana przez ciebie liczba to 2) \n"
            "3. Zamknij aplikację (wpisana przez ciebie liczba to 3) \n");
        printf("Podaj nr polecenia: ");
        scanf("%d", &input);
        
        switch (input) {
            case 1:
                //Zaloguj
                printf("\nZaloguj się!\nPodaj swój nick: ");
                scanf("%s", slowo);
                strcpy(login.nick, slowo);

                printf("Podaj swoje hasło: ");
                scanf("%d", &haslo);
                login.id_adresata = haslo;
                login.polecenie = 9;
                login.id_nadawcy = MyID;
                success = msgsnd(ID_KOLEJKI_DO_SERWERA, &login, ROZMIAR_STRUCTA, 0);
        
                if (success == 0){
                    zalogowany = true;
                    success = msgrcv(MyID, &odebrana, ROZMIAR_STRUCTA, 0, 0);
                    if (odebrana.error == 5){
                        printf("\nLogowanie nie powiodło się. Niepoprawny login.");
                        zalogowany = false;
                    }
                    else if(odebrana.error == 6){
                        printf("\nLogowanie nie powiodło się. Użytkownik jest już zalogowany.");
                        zalogowany = false;
                    }
                    else if(odebrana.error == 4){
                        printf("\nLogowanie nie powiodło się. Niepoprawne hasło.");
                        zalogowany = false;
                    }
                    else if (odebrana.error != 4 && odebrana.error != 5 && odebrana.error != 6 && odebrana.error != 0){
                        printf("\nLogowanie nie powiodło się. Spróbuj jeszcze raz.");
                        zalogowany = false;
                    }
                }
    
                break;
            case 2:
                //Zarejestruj
                printf("Zarejestruj się!\nPodaj swój nick: ");
                scanf("%s", slowo);
                strcpy(login.nick, slowo);

                printf("Podaj swoje hasło: ");
                scanf("%d", &haslo);
                login.id_adresata = haslo;
                login.polecenie = 8;
                login.id_nadawcy = MyID;

                success = msgsnd(ID_KOLEJKI_DO_SERWERA, &login, ROZMIAR_STRUCTA, 0);
                if(success == 0){
                    success = msgrcv(MyID, &odebrana, ROZMIAR_STRUCTA, 0, 0);
                    if(odebrana.error != 0){
                        printf("Rejestracja użytkownika nie powiodła się. Spróbuj jeszcze raz.");
                    }
                }
                break;
            case 3:
                //Wyjdź
                printf("Zamykanie programu i usuwanie kolejek\n");
                //todo zamknięcie kolejek!!!!!!
                success = msgctl(MyID, IPC_RMID, 0);
                // printf("Usunięta? %d", success);
                return 0;
            default:
                printf("Nieprawidłowe polecenie");
        }
    }


    if(zalogowany){
        printf("\nWitaj *zalogowany* użytkowniku! \n"
            "Oto Twoje dostępne opcje: \n"
            "- 0. Wyloguj się z serwera \n"
            "- 1. Wyślij wiadomość do użytkownika \n"
            "- 2. Wyślij wiadomość do grupy użytkowników \n"
            "- 3. Sprawdź kto jest zalogowany \n"
            "- 4. Sprawdź dostępne grupy \n"
            "- 5. Zapisz się do grupy \n"
            "- 6. Wypisz się z grupy \n"
            "- 7. Sprawdź skład danej grupy \n"
            "- 8. Odczytaj wiadomości \n"
            "- 9. Zablokuj użytkownika \n"
           );
    }
    
    while(zalogowany){
        zapytanie odebrana_wiad, zap;
        zap.id_nadawcy = MyID;

        int input, input_dwa, rozmiar = 1, pom, ilosc_zablokowanych = 0, ZABLOKOWANI[ILE_MAX_UZYTKOWNIKOW];
        char slowo2[100];

        fflush(stdin);
        
        printf("\n\nPODAJ NR POLECENIA: ");
        scanf("%d", &input);

        // fflush(stdin);
        
        switch (input) {
            case 0:
                //Wylogowanie
                zap.polecenie = 0;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                zalogowany = false;
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                msgctl(MyID, IPC_RMID, NULL);
                return 0;
                break;
            case 1:
                // Wyślij wiadomość do użytkownika
                printf("Podaj ID Osoby do której chcesz wysłać wiadomość: \n");
                scanf("%d", &input_dwa);
                zap.polecenie = 1;
                zap.id_adresata = input_dwa;
                zap.id_nadawcy = MyID;

                printf("Wpisz swoją wiadomość (niewięcej niż 100 znaków): \n");
                scanf("%s", slowo2);
                // fgets(slowo2, sizeof(slowo2), stdin);
                strcpy(zap.wiadomosc, slowo2);

                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 2:
                // Wyślij wiadomość do grupy użytkowników
                printf("Podaj ID Grupy do której chcesz wysłać wiadomość\n");
                scanf("%d", &input_dwa);
                zap.polecenie = 2;
                zap.id_adresata = input_dwa;
                zap.id_nadawcy = MyID;
                
                printf("Wpisz swoją wiadomość (niewięcej niż 100 znaków): \n");
                scanf("%s", slowo2);
                // fgets(slowo2, 100, stdin);
                strcpy(zap.wiadomosc, slowo2);

                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 3:
                // Sprawdź kto jest zalogowany
                zap.polecenie = 3;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                // printf("send case 3");
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                // printf("stop");
                break;
            case 4:
                // Sprawdź dostępne grupy
                zap.polecenie = 4;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 5:
                // Zapisz się do grupy
                printf("Podaj ID Grupy do której chcesz się zapisać:\n");
                scanf("%d", &input_dwa);
                zap.polecenie = 5;
                zap.error = input_dwa;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 6:
                // Wypisz się z grupy
                printf("Podaj ID Grupy z której chcesz się wypisać:\n");
                scanf("%d", &input_dwa);
                zap.polecenie = 6;
                zap.error = input_dwa;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 7:
                // Sprawdź skład danej grupy
                printf("Podaj ID Grupy której skład chcesz sprawdzić:\n");
                scanf("%d", &input_dwa);
                zap.polecenie = 7;
                zap.error = input_dwa;
                zap.id_adresata = ID_KOLEJKI_DO_SERWERA;
                zap.id_nadawcy = MyID;
                msgsnd(ID_KOLEJKI_DO_SERWERA, &zap, ROZMIAR_STRUCTA, 0);
                break;
            case 8:
                while(rozmiar > 0){
                    rozmiar = msgrcv(MyID, &odebrana_wiad, ROZMIAR_STRUCTA, 0, IPC_NOWAIT);
                    pom = 0;
                    for(int i=0; i<ILE_MAX_UZYTKOWNIKOW; i++){
                        if(ZABLOKOWANI[i] == odebrana_wiad.id_nadawcy ){
                            pom = 1;
                        }
                    }
                    if (pom == 0 && rozmiar != -1){
                        printf("\n ---------- Masz wiadomość od: %s ----------\n%s \n", odebrana_wiad.nick, odebrana_wiad.wiadomosc);
                    }
                }
                break;
            case 9:
                // Zablokuj użytkownika
                printf("Podaj ID osoby którą chcesz zablokować: \n");
                scanf("%d", &input_dwa);
                pom = 0;
                for(int i=0; i<ilosc_zablokowanych; i++){
                    if(ZABLOKOWANI[i] == input_dwa){
                        printf("Ten uzytkownik jest już zablokowany! \n");
                        pom = 1;
                        break;
                    }
                }
                if(pom == 0){
                    ilosc_zablokowanych++;
                    ZABLOKOWANI[ilosc_zablokowanych] = input_dwa;
                }
                break;
            default:
                printf("Nieprawidłowe polecenie");
        }
    }
    // }

    printf("\n ej bo program klienta się skończył, a nie powinien tędy\n");
    return 0;
    
}
