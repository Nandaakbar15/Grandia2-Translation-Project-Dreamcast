/* L62C Grandia 2 file extractor/compressor 0.5beta by Mat
Programma che gestisce gli i file in formato L62C presenti negli archivi di
Grandia 2.
Per una spiegazione sul formato della compressione fare riferimento al file
docL62C.htm scritto da Gemini.

Questa invece è il formato dell'header (grazie mille a Chop per l'aiuto):
- 4 byte identificativi che sono "L62C"
- 4 byte che identificano la grandezza dei dati compressi (quelli successivi
  ai primi 8 byte)
- 4 byte che identificano la grandezza dei dati compressi compreso l'header
  (quindi 16 byte in piu')
- 4 byte che identificano la grandezza dei dati compressi in Big Endian

NOTA: in alcuni file nell'header e' specificata una dimensione piu' piccola di
quella del file, in questi casi il file e' formato da più L62C uno dietro
l'altro.
Per adesso occorre dividere manualmente o con g2split il file in piu' parti ed
estrarle una ad una.

Note by Chop:
 In decompressione bisogna considerare soltanto i byte indicati dalla terza
 voce dell'header (ricordarsi di contare anche i 16 byte dell'header) perche'
 gli altri sono solo di padding e vanno esclusi dal file finale.

 In compressione al contrario bisogna inserire dei byte di padding 0x00 qualora
 non si riuscisse a completare un blocco di 9 byte (flag + 8byte).
 Fatto questo bisogna testare la lunghezza totale del file compresso cosi' ottenuto,
 se non e' multiplo di 4 si padda con 0xFF fino a che non lo diventa!

Il codice e stato scritto facendo in modo che le funzioni siano riutilizzabili
e gestiscano gli errori delle system call.
Penso di aver scritto un codice decente anche se c'e' un grosso spreco di
memoria nella compressione. Se qualche riga e' poco chiara contattami.
Questo programma e' stato scritto con Dev-C++ e compilato con gcc, e una
richiesta fondamentale per il funzionamento e' che i dati siano Little Endian
e che gli int siano a 4 byte e i char a 1 byte (e' per questo che nelle 
funzioni non ho usato sizeof(int), perche' se la dimensione fosse diversa il
programma non funzionerebbe comunque).

NOTE SULLA COMPRESSIONE
Nella decompressione sono implementati 2 trick:
- overlap
- negative jump (giusto per sicurezza nel caso venga utilizzato)
Nella compressione invece e' implementato solo l'overlap e non il negative jump
perche' da quello che ho potuto vedere in originale non c'e'

RINGRAZIAMENTI
- Gemini per aver spiegato il funzionamento della compressione sul forum di emuita
- Chop per aver risolto scoperto i problemidi dell'header e del padding
- _Ombra_ per aver insistito sul perfezionameto della compressione e aver contattato Chop

Mat - 18/11/2003
E-mail: mattia.d.r@libero.it
Sito:   http://www.matzone.altervista.org
Membro dei SadNES cITy: http://www.sadnescity.it
Public Release: 11/08/2004
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GRA2_BUFFSIZE 32768

// Caratteristiche della compressione
// Prima 4 byte dei file compressi (scritta "L62C")
#define GRA2_L62C 0x4332364C
// Grandezza di massima di una stringa compressa
#define GRA2_LINEA 9
// Grandezza massima del salto 1 + 63 (6 bit)
#define GRA2_MJUMP 64
// Massimo recupero 2 + 3 (2 bit)
#define GRA2_MREC 5
// Massimo recupero di una linea 8 * MREC
#define GRA2_LMREC 40


int decomprimi_file(FILE *orig, FILE *dest);
/* Decomprime un file in formato L62C
    infile:  file pointer del file compresso (verra' messo il seek a 0)
    outfile: file pointer su cui verranno accodati i dati decompressi
    
    Valore di ritorno:
    0 in caso di successo
    1 se il file non e' compresso con L62C
    2 in caso di errore di lettura o di seek
    3 in caso di errore di scrittura
    
    NOTA: La funzione non chiude i file pointer passati e muove il seek
    NOTA2: La funzione stampa dei messaggi su stdout
*/

int comprimi_file(FILE *orig, FILE *dest);
/* Comprime un file in formato L62C
    infile:  file pointer del file sorgente (verra' messo il seek a 0)
    outfile: file pointer su cui verranno accodati i dati compressi
    
    Valore di ritorno:
    0 in caso di successo
    -1 nel caso di errori all'allocazione della memoria
    2 in caso di errore di lettura o di seek
    3 in caso di errore di scrittura
    
    NOTA: La funzione non chiude i file pointer passati e muove il seek
*/

int cercaprec(unsigned char *poslet, unsigned char *posiniz, unsigned char *posmax);
/* Cerca un match uguale nei byte precedenti, overlap OK
    poslet:  posizione in cui c'e' la stringa che dobbiamo cercare
    posiniz: posizione fino a cui possiamo ritornare alla ricerca di un match
    posmax:  posizione entro la quale dobbiamo fermarci nella lettura della stringa di input
    
    Valore di ritorno:
    -1 se non trova un match di almeno 2 byte
    0 - 255 nel caso trovi un match
      il valore corrisponde a ((jump - 1) << 2) | (recupero - 2)
    
*/

int inverti(const char *num);
/*  Trasforma un array di char il Big Endian in un intero Little Endian oppure
    un array di char il Little Endian in un intero Big Endian
     num: array di 4 char da convertire
    
    Valore di ritorno:
     l'intrero convertito
*/

int main(int argc, char *argv[]) {
   FILE *letto, *scritto;
   int err;
   
   if(argc == 4) {  // 4 argomenti richiesti
      if(!(strcmpi(argv[1], "-e") && (err = strcmpi(argv[1], "-c")))) {  // opzioni supportate
         if((letto = fopen(argv[2], "rb")) == NULL) {  // apre il file in lettura
            printf("Impossibile aprire %s\n", argv[2]);
            exit(2);  // uscita con errore
         }
         if((scritto = fopen(argv[3], "wb")) == NULL) {  // apre il file in scrittura
            printf("Impossibile aprire %s\n", argv[3]);
            fclose(letto);
            exit(2);  // uscita con errore
         }
         if(err) err = decomprimi_file(letto, scritto);  // decomprime se l'opzione non e' -c
         else err = comprimi_file(letto, scritto);  // altrimenti comprime
         fclose(letto);  // chiude i file
         fclose(scritto);
         
         if(err) {  // se c'e' un errore
            unlink(argv[3]);  // elimina il file destinazione
            if(err == -1) printf("Errore nell'allocazione della memoria");
            if(err == 1) printf("Il file non \212 nel formato L62C di Grandia 2");
            else if(err == 2) printf("Impossibile leggere il file %s", argv[2]);
            else printf("Impossibile scrivere il file %s", argv[3]);
            printf(" - Operazione non riuscita\n");
            exit(2);  // uscita con errore
         }
         printf("L62C Grandia 2 file extractor/compressor 0.5beta by Mat - Operazione riuscita\n");
         exit(0);  // uscita corretta
      }
   }      
   printf("L62C Grandia 2 file extractor/compressor 0.5beta by Mat - USO:\nextL62C -e origine destinazione   (estrae un file)\nextL62C -c origine destinazione   (comprime un file)\n\nGrazie a Gemini, Chop e _Ombra_ per l'aiuto :)\n\nMat - 18/11/2003\nE-mail:\tmattia.d.r@libero.it\nSito:\thttp://www.matzone.altervista.org\nMembro dei SadNES cITy: http://www.sadnescity.it\nPublic Release: 11/08/2004\n");
   exit(1);  // // uscita con errore per mancanza di parameti
}

int decomprimi_file(FILE *orig, FILE *dest) {

   unsigned char linea[GRA2_LINEA], buffer[GRA2_BUFFSIZE] = {'\0'};
   int i, j, flag, pos = GRA2_MJUMP, salto, nletti, nbyte, header[4];
   // pos = GRA2_MJUMP perche' i primi 64 byte servono per il salto 
   int extsize;  // grandezza del file estratto
   int scritti = 0;  // numero di byte ottenuti per il file decompresso
   
   if(fseek(orig, 0, SEEK_SET)) return 2;  // si posiziona all'inizio
   if(fread(header, 4, 4, orig) != 4) return 2;  // legge l'header
   if(header[0] != GRA2_L62C) return 1;  // controllo L62C
   extsize = inverti((char *)(header + 3));  // converte in little-endian la grandezza del file decompresso
   if((extsize + 16) != header[2]) printf("Dati nell'header incoerenti, proviamo a continuare...\n");
   
   while(scritti < extsize) {
      if((nletti = fread(linea, 1, GRA2_LINEA, orig)) <= 0) break;  // termina il ciclo se siamo a fine file
      flag = linea[0];  // il primo byte è il flag
      for(i = 1; i < nletti; ++i) {  // per gli altri 8 byte
        if(scritti >= extsize) break;  // esce se abbiamo raggiunto il numero di dati da scrivere
        if((flag >> (i - 1)) & 1) {  // dati compressi
          salto = (linea[i] >> 2) + 1;  // calcola il salto
          nbyte = (linea[i] & 3) + 2;  // byte da recuperare
          scritti += nbyte;  // aumenta l'indice dei dati decompressi
          for(j = 0; j < nbyte; ++j) buffer[pos] = buffer[pos++ - salto];  // scrive i byte recuperati (overlap ok - no memcpy)
        }
        else {
           buffer[pos++] = linea[i];  // dati normali
           ++scritti;  // aumenta l'indice dei dati decompressi
        }
      }
     
      if(pos >= (GRA2_BUFFSIZE - GRA2_LMREC)) {  // buffer a rischio nel prossimo ciclo - byte recuperati (max) = 5, numero recuperi (max) = 8, quindi 8*5 = 40
         pos -= GRA2_MJUMP;  // numero di byte da scrivere
         if(fwrite(buffer + GRA2_MJUMP, pos, 1, dest) != 1) return 3;  // scrive il buffer su file
         for(j = 0; j < GRA2_MJUMP; ++j) buffer[j] = buffer[j + pos];  // sposta gli ultimi 64 byte all'inizio del buffer (per eventuali recuperi)
         pos = GRA2_MJUMP;  // setta la posizione iniziale in cui scriveremo nel buffer
      }
   }
   if(ferror(orig)) return 2;  // se c'e' stato errore ritorna
   if(fwrite(buffer + GRA2_MJUMP , pos - GRA2_MJUMP, 1, dest) != 1) return 3;  // scrive il contenuto del buffer

   if((pos = ftell(orig)) == -1) return 2;  // posizione nel file compresso
   if(fseek(orig, 0, SEEK_END)) return 2;  // fine del file
   if((j = ftell(orig)) == -1) return 2;  // grandezza del file compresso
   
   header[1] += 8; // calcola la dimesione del file compresso
   pos = (header[1] - pos) + (nletti - i);  // calcola i dati non usati
   
   printf("Dati cestinati: %d, %s\n", pos, (pos <= 10) ? "OK" : "ATTENZIONE");
   if(j > header[1]) printf("Dimensione del file compresso maggiore di quella indicata nell'header, prova a\n usare g2split\n");
   else if(j != header[1]) printf("Dimensione del file compresso minore di quella indicata nell'header,\n forse il file \212 corrotto\n");
   // questo contollo deve essere portato fuori dalla funzione
   return 0;  // torniamo al main
}

int comprimi_file(FILE *orig, FILE *dest) {
   #define uscita(num) { free(buflet); return(num); }
   unsigned char bufcomp[GRA2_LINEA];  // inizio del file compresso
   unsigned char *buflet;  // conterra' i dati del file da comprimere
   int header[4] = {GRA2_L62C};  // header del file, all'inizio contiene L62C
   int filesize;  // grandezza dei dati da comprimere 
   int ciclo = 0;  // variabile boleana
   int poslet = 0, poscomp, posiniz, flag, i;
   
   if(fseek(orig, 0, SEEK_END)) return 2;  // fine del file
   if((filesize = ftell(orig)) == -1) return 2;  // dimensione del file
   if(fseek(orig, 0, SEEK_SET)) return 2;  // ritorna all'inizio
   header[2] = filesize + 16;  // terzo campo dell'header
   header[3] = inverti((char *)(&filesize));  // quarto campo dell'header
   if(fwrite(header, 4, 4, dest) != 4) return 3;  // scrive l'header
   if(!(buflet = malloc(filesize))) return -1;  // alloca la memoria necessaria per il file
   if(fread(buflet, filesize, 1, orig) != 1) uscita(2);  // legge il contenuto del file
   
   while(!ciclo) {  // inizio della compressione
      bufcomp[0] = '\0';  // setta la il byte dei salti a 0 provvisoriamente
      poscomp = 1;  // i dati vanno dopo il flag
      for(i = 1; i & 255; i <<= 1) {  // ripete il ciclo 8 volte
         posiniz = poslet - GRA2_MJUMP;  // possiamo tornare indietro al massimo di 64 byte
         if(posiniz < 0) posiniz = 0;  // all'inizio del file non possiamo tornare indietro di 64
         // ricerca un match uguale di almeno 2 byte
         if((flag = cercaprec(buflet + poslet, buflet + posiniz, buflet + filesize)) == -1) {  // nessun match
            bufcomp[poscomp++] = buflet[poslet++];  // scrive il byte direttamente
         }
         else { // abbiamo trovato un match
            bufcomp[poscomp++] = flag;  // scrive il flag nel buffer di scrittura
            bufcomp[0] |= i;  // imposta il bit relativo a questo byte a 1
            poslet += (flag & 3) + 2; // aumenta la posizione di lettura
         }
         if(ciclo = poslet >= filesize) {
            if(poslet != filesize) printf("Errore! Credo proprio di aver sbagliato qualcosa nell'algoritmo senza\naccorgermene (non ho mai ottenuto questo errore)!\n");
            break;
         }
         // c'e' solo per scrupolo
      }
      if(fwrite(bufcomp, poscomp, 1, dest) != 1) uscita(3);  // scrive il buffer nel file
   }
   free(buflet);  // libera la memoria allocata
   poscomp = GRA2_LINEA - poscomp;  // calcola il numero di byte di riempimento
   if(poscomp) {
      header[0] = 0;  // con questo i primi 8 byte di header sono 00
      if(fwrite(header, poscomp, 1, dest) != 1) uscita(3);  // scrive gli 00 nel file
   }   
   if((filesize = ftell(dest)) == -1) return 3;  // ottiene la dimensione del file compresso
   if(filesize & 3) {
      header[0] = 0xFFFFFFFF;
      i = 4 - (filesize & 3); // numero di FF da scrivere
      if(fwrite(header, 4 - (filesize & 3), 1, dest) != 1) return 3;  // scrive gli FF nel file
      filesize += i;  // aumenta la dimensione del file
   }   
   filesize -= 8;  // togliamo gli 8byte di header
   if(fseek(dest, 4, SEEK_SET)) return 3;  // andiamo nell'header del file
   if(fwrite(&filesize, 4, 1, dest) != 1) return 3;  // scriviamo la dimensione dei dati
   return 0;  // finalmente abbiamo finito!!!
}

int cercaprec(unsigned char *poslet, unsigned char *posiniz, unsigned char *posmax) {
   int i, count = 0, max = 1, maxpos;
   unsigned char *poscur;
   
   for(poscur = poslet - 1; poscur >= posiniz; --poscur) {  // inizia a cercare un match
      for(i = 0; i < GRA2_MREC; ++i) { // cicla per un massimo di 5 byte
         if((poslet + i) >= posmax) break;  // evita di uscire dal buffer
         if(poscur[i] == poslet[i]) ++count;  // un byte uguale
         else break;  // usciamo dal ciclo
      }
      if(count > max) { // abbiamo un recupero maggiore del precedente
         max = count;  // nuovo massimo
         maxpos = poslet - poscur;  // posizione in cui abbiamo il massimo recupero
      }
      if(count == GRA2_MREC) break;  // e' inutile continuare a cercare, abbiamo gia' il massimo recupero
      count = 0;  // rimette il contatore a 0 per il prossimi ciclo
   }
   if(max == 1) return -1;  // non abbiamo trovato una stringa di almeno due byte
   return (((maxpos - 1) << 2) | (max - 2));  // ritorna il flag corrispondete alla posizione a ai byte recuperati
}

int inverti(const char *num) {
   char res[4]; // risultato
   
   res[0] = num[3];
   res[1] = num[2];
   res[2] = num[1];
   res[3] = num[0];
   return *((int *)res);
}
