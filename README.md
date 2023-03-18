# Komunikator tekstowy - projekt z przedmiotu Programowanie Systemowe i Wsółbieżne
Komunikator tekstowy obsługiwany w terminalu Linuxa, pisany w języku C

********Funkcjonalność programu********

Projekt jest komunikatorem tekstowym działającym w terminalu systemu Linux, opierającym się na mechanizmach IPC. Główną rolę pełniły tutaj kolejki komunikatów, którymi to przesyłane są wszystkie informacje między klientem a serwerem. Komunikator obsługuje użytkownika na jego osobistym koncie, umożliwiając mu:
 - rejestrację
 - logowanie się
 - wylogowywanie się
 - wysyłanie wiadomości do innego użytkownika
 - wysyłanie wiadomości do grupy użytkowników do której jest zapisany
 - podgląd zalogowanych użytkowników
 - podgląd dostępnych grup tematycznych
 - podgląd członków danej grupy
 - zapisanie się do grupy
 - wypisanie się z grupy
 - sprawdzenie wiadomości
 - blokowanie innego użytkownika (tak, aby nie otrzymywać od niego wiadomości)



********Instrukcja kompilacji********

Odpowiednio dla serwera i klienta (w osobnych oknach terminala):
- gcc serwer.c -Wall -o serwer
- gcc klient.c -Wall -o klient

********Instrukcja uruchomienia********

Odpowiednio dla serwera i każdego klienta, jakiego chcemy zalogować (w osobnych oknach terminala):
- ./serwer
- ./klient



********Krótki opis zawartości poszczególnych plików ********

---PLik klienta:

- Dzieli się na dwa główne procesy:
  
   *pierwszy:
	- zawiera dwie główne pętle: pierwsza pętla "niezalogowany" - jest to interfejs pojawiający się kiedy użytkownik nie jest zalogowany i pozwala:
	   > zalogować się
	   > zarejestrować (stworzyć konto)
	   > zamknąć aplikację
	druga pętla "zalogowany" - jest to interfejs dający dostęp do głównych funkcji serwera, jednak dostępny dopiero po zalogowaniu się w poprzednim, wyżej wspomnianym etapie programu. Zawiera funkcje pozwalające:
	   > wysłać wiadomość do innego użytkownika, lub do całej grupy;
	   > sprawdzić kto jest zalogowany, dostępne grupy, a także ich skład;
	   > zapisać, lub wypisać się z grupy;
	   > wylogowanie się i wyjście (wyłączenie) programu (zamykając za sobą odpowiednią kolejkę).

   *drugi:
	- Proces ten jest odpowiedzialny za przechwytywanie nadciągających wiadomości i wypisywanie ich na ekranie,
jest on ciągle aktywny, a więc wiadomości będą pojawiać się kiedy tylko zostaną przechwycone.


---Plik serwera:

- Zawiera implementacje poszczególnych funkcji, wspomnianych wyżej (odpowiedzi na zapytania od klienta).
- Przechowuje "bazę danych", czyli kilka dodatkowych struktur przechowywujących informacje o klientach i grupach potrzebne do przekazywania im odpowiednich wiadomości.


---Plik konfiguracyjny:

- Zawiera domyślną bazę użytkowników oraz grup użytkowników.


********Struktura danych używana do przesyłania wiadomości:********

struct zapytanie{
    int id_nadawcy, id_adresata, polecenie, error;
    char wiadomosc[DLUGOSC_WIADOMOSCI], nick[10];
};


********Pola struktury:********

-- "polecenie", typu int - liczba całkowita oznaczająca rodzaj polecenia jakiego dotyczy zapytanie/odpowiedź.
Konkretne numery odpowiadają konkretnym typom poleceń, dla przykładu:
'3' - to przesłanie wiadomości,
'7' - to wypisanie się z grupy.

-- "id_nadawcy", typu int - pole w którym przesyłamy id kolejki nadawcy zapytania (klienta) podczas wysyłania polecenia do serwera.
Używany aby zidentyfikować nadawcę wiadomości przez klienta, kiedy trafia do niego za pośrednictwem serwera.

-- "id_adresata", typu int - pole słuzące do identyfikacji adresata wiadomości (użytkownika, grupy lub też serwera) przez serwer. W niektórych funkcjach jest używany jako bufor na inne, potrzebne dane.

-- "wiadomosc", typu char[] - ciąg znaków, zwyczajowo przechowujący docelową wysyłaną treść wiadomości.

-- "nick", typu char[] - zmienna przechowująca nick użytkownika przy logowaniu się na serwer.

-- "error", typu int - zmienna, w której przekazujemy informacje o powodzeniu lub niepowodzeniu akcji.


********Protokół wysyłania komunikatów********

a) komunikacja Klient -> Serwer

Serwer tworzy własną kolejkę komunikatów, z której będzie czytał wszystkie płynące do niego zapytania.
Każdy klient wysyła swoje zapytania do jednej i tej samej kolejki serwera.


b) komunikacja Serwer -> Klient

Każdy Klient ma własną kolejkę komunikatów, której ID udostępnia serwerowi w procesie logowania.
Serwer wysyła odpowiedzi na zapytania oraz przechwycone wiadomości do danego klienta poprzez tą osobistą kolejkę.


c) Ilość kolejek wynosi n+1, gdzie n jest liczbą aktywnych klientów (a +1 odnosi się do kolejki serwera).


