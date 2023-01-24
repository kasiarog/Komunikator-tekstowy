# Komunikator-tekstowy-projekt-PSiW-
Komunikator tekstowy obsługiwany w terminalu Linuxa, pisany w C

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
- gcc inf151846inf151879_s.c -Wall -o inf151846inf151879_s
- gcc inf151846inf151879_k.c -Wall -o inf151846inf151879_k



********Instrukcja uruchomienia********

Odpowiednio dla serwera i każdego klienta, jakiego chcemy zalogować (w osobnych oknach terminala):
- ./inf151846inf151879_s
- ./inf151846inf151879_k



********Krótki opis zawartości poszczególnych plików *.c********

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
