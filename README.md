Petrica Gheorghe-Vladimir - 323CA

# Web Client. Communication with REST API

## Descriere generala
Protocolul HTTP este folosit in mai multe tipuri de aplicatii precum trans-
ferul de fisiere sau expunerea unor API-uri REST

Vom comunica cu server care simuleaza o biblioteca de carti online si vom
face catre acesta diferite cereri HTTP, iar raspunsurile acestuia vor fi inter-
pretate de clientul ce urmeaza sa fie descris.

## Implementare Client
In implementarea clientului m-am folosit de laboratorul 9, din care am ex-
tras functiile compute_get_request si compute_post_request, pe care le-am adap-
tat corespunzator.
* Pentru cererile de tip GET, adaugam la cererea noastra calea si versiunea
protocolului HTTP, host-ul, precum si daca avem token jwt sau cookies plus info-
rmatia de keep-alive pentru a mentine conexiunea intre server si client deschisa
si dupa primirea raspunsului.
* Pentru cererile de tip POST, procedem precum anterior doar ca intre punerea
hostului si cea a cookie-urilor/token-ului vom adauga si Contet-Type precum si
Contet_Length, iar dupa punerea informatiei de keep-alive vom adauga datele
propriu-zise.
* Pentru cererile de tip DELETE, am folosit ca model cererea de tip GET, modi-
ficand doar numele metodei.

In fisierul request.c unde am implementat functiile de mai sus, am mai imple-
mentat si functiile get_cookies si get_token care se ocupa de extragerea cookie-
urilor precum a token-ului

In fisierul client.c am realizat implementarea clientului propriu-zisa dupa cum
urmeaza:

* vom avea o bucla infinita pentru a primi mai multe comenzi
* deschidem o noua conexiune intre host si server inainte de fiecare comanda
* vom citi de la STDIN comanda pe care vrem s-o interpretam
* REGISTER: verificam mai intai daca suntem deja conectacti la server, iar in
cazul in care suntem vom afisa un mesaj de eroare. In caz negativ, citim de la
STDIN username-ul si parola pe care vrem sa le punem noului cont. Vom verifica
mai departe daca aceste sunt valide (daca nu sunt nule/ nu contine spatii). In
cazul in care acestea sunt valide, vom crea din acestea un obiect JSON, pe care
il vom transforma in string, astfel ca vom trimite o cerere de tip POST catre
server cu stringul realizat anterior. Vom intrepreta mai departe raspunsul server-
ului si vom afisa un mesaj de eroare/succes.
* LOGIN: este implementat asemanator cu functia de register, doar ca in cazul in
care primesc un mesaj de succes de la server, atunci voi extrage cookie-urile din
raspunsul serverului.
* ENTER_LIBRARY: in cazul acestei comenzi vom trimite catre server o cerere de tip
GET, din care vom extrage in caz de succes token-ul jwt (vom avea acces la bibli-
oteca)
* GET_BOOKS: vom verifica daca suntem conectati si daca avem acces la biblioteca.
In functie de raspunsul serverului, il vom interpreta si vom afisa cu ajutorul
functiilor din parson.h, toate cartile din biblioteca. In caz de esec afisam un
mesaj de eroare.
* GET_BOOK: vom verifica dacă suntem conectați și dacă avem acces la bibliotecă. În cazul în care avem acces, vom cere de la utilizator ID-ul cărții pe care dorim să o obținem. Vom construi endpoint-ul folosind acest ID și vom trimite o cerere GET către server. Răspunsul serverului va fi interpretat și dacă obținem un status de succes, vom afisa detaliile cărții. În caz de eroare, vom afisa un mesaj corespunzător.
* ADD_BOOK: vom verifica dacă suntem conectați și dacă avem acces la bibliotecă. Vom cere de la utilizator titlul, autorul, genul, editura și numărul de pagini ale cărții. Vom verifica dacă toate aceste informații sunt valide. Dacă sunt valide, vom crea un obiect JSON cu informațiile respective, îl vom serializa și trimite într-o cerere POST către server. Vom interpreta răspunsul serverului și vom afișa un mesaj de succes sau eroare.
* DELETE_BOOK: vom verifica dacă suntem conectați și dacă avem acces la bibliotecă. Vom cere de la utilizator ID-ul cărții pe care dorim să o ștergem. Vom construi endpoint-ul folosind acest ID și vom trimite o cerere DELETE către server. Răspunsul serverului va fi interpretat și dacă obținem un status de succes, vom afișa un mesaj de succes. În caz de eroare, vom afișa un mesaj corespunzător.
* LOGOUT: vom verifica dacă suntem conectați. Dacă suntem conectați, vom trimite o cerere GET către server pentru a realiza logout-ul. Vom interpreta răspunsul serverului și în caz de succes vom reseta starea clientului la deconectat și vom elibera memoria alocată pentru cookies și token. În caz de eroare, vom afișa un mesaj corespunzător.
* EXIT: când utilizatorul introduce comanda de ieșire, vom elibera memoria alocată pentru cookies și token, dacă acestea există, și vom închide conexiunea. Bucla principală va fi întreruptă și programul se va termina.
* COMANDA INVALIDA: In cazul in care utilizatorul nu introduce o comanda valida,
acesta va fi notificat cu un mesaj de eroare.  