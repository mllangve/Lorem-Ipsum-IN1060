/*Resource Trackeren lar brukeren enkelt holde oversikt over diverse ressurser, evner og "spells",
  samt hvor mange av hver ting de har tilgjengelige til en hver tid. Brukerne vil selv kunne stille 
  inn hvor mange "poeng" de skal ha for hver ressurs i form av LED-lys. Når de bruker en ressurs vil de
  enkelt kunne skru av et lys for å indikere at de har én mindre tilgjengelig. 
  
  Brukeren vil også enkelt kunne stille ressursene tilbake til den forhåndslagrede mengden de satte tidligere
  skulle det være behov for dette. Som f.eks ved en long-rest.

  Dersom brukeren skulle fjerne et poeng fra en ressurs ved et uhell vil de enkelt kunne rette opp i dette ved å skru 
  lyset på igjen. Dette vil de også få mulighet til ved "short-rests" ettersom man ikke får tilbake alle ressursene sine,
  men heller noen få, om noen, på hver ressurs.

  Brukeren vil også få feedback på diverse samhandlinger de gjør med resource-trackeren i form av lyd i tillegg til lyssignaler.

  Optimalt, og i en ferdig laget artefakt ville det vært ti rader med fem lys i hver rad, men for dette prosjektets skyld,
  i tillegg til at det er en prototype og et finansielt aspekt ved å utvide med flere rader har vi valgt å holde oss til to.
 */
 //BIBLIOTEK
#include <EEPROM.h>
//BIBLIOTEK - SLUTT---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SET OG REST - START
int set = 13;                     //Pin koblet på push-button som brukes til å lagre nytt lys-oppsett
int rest = 10;                    //Pin koblet på push-button som brukes til å sette LED-lysene tilbake til forhåndslagret set-verdi
int hoyttaler = A5;               //Pin koblet til Piezo-høyttaler, skal brukes til å spille av diverse lydsekvenser
int setVal;                       //Variabel som brukes til å lese sets verdi
int restVal;                      //Variabel som brukes til å lese rests verdi
int knappeTrykk = 0;              //Variabel som fra start settes til 0 som holder oversikt på hvor mange ganger set og rest har blitt trykket på for å bestemme hvilken respons bruker får ved neste knappetrykk
int setLager [2];                 //En tom array med to plasser som brukes til å holde oversikt over hvor mange lys som er på for de to lysradene
String knappeNavn = "";           //En tom String-variabel som lagrer navnet på knappen som ble trykket på sist for å unngå feil-trykk og blanding av knappene
unsigned long tid = 2000;         //Variabel på 2000ms som skal brukes til å sjekke hvor lag tid det har gått mellom hvert knappetrykk
unsigned long forrigeTrykk = 0;   //Variabel som starter på null, brukes også til å sjekke hvor lang tid som har gått mellom hvert knappetrykk
//SET OG REST - SLUTT-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//RAD 1 - START
//PINS KOBLET PÅ LED-PÆRER
int rad1Lys1 = A0;
int rad1Lys2 = A1;
int rad1Lys3 = A2;
int rad1Lys4 = A3;
int rad1Lys5 = A4;
//LED-PINS SLUTT
int rad1Pluss = 12;                        //Pin koblet på push-button som brukes til å skru på neste LED-lys
int rad1Minus = 9;                         //Pin koblet på push-button som brukes til å skru av neste LED-lys
int rad1PlussVal;                          //Variabel som brukes til å lese rad1Pluss’ verdi
int rad1MinusVal;                          //Variabel som brukes til å lese rad2Minus’ verdi
static int antLysPaaRad1 = 0;              //Variabel som holder oversikt hvor mange lamper som står på
int lagringsPlass1 = 0;                    //Variabel som viser hvilken plass i EEPROM minnet skal oppdateres med antLysPaaRad1
//RAD 1 - SLUTT-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//RAD 2 - START
//PINS KOBLET PÅ LED-PÆRER
int rad2Lys1 = 7;
int rad2Lys2 = 5;
int rad2Lys3 = 4;
int rad2Lys4 = 3;
int rad2Lys5 = 2;
//LED-PINS SLUTT
int rad2Pluss = 11;                         //Pin koblet på push-button som brukes til å skru på neste LED-lys
int rad2Minus = 8;                          //Pin koblet på push-button som brukes til å skru av neste LED-lys
int rad2PlussVal;                           //Variabel som brukes til å lese rad2Pluss’ verdi
int rad2MinusVal;                           //Variabel som brukes til å lese rad2Minus’ verdi
static int antLysPaaRad2 = 0;               //Variabel som holder oversikt hvor mange lamper som står på
int lagringsPlass2 = 1;                     //Variabel som viser hvilken plass i EEPROM minnet skal oppdateres med antLysPaaRad2
//RAD 2 - SLUTT-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



//ARRAYS
int lysRad1 [] = {0, rad1Lys1, rad1Lys2, rad1Lys3, rad1Lys4, rad1Lys5};                                                       //Holder alle LED-lysene på rad 1 - Har en "tom" plass først i arrayen. Dette er fordi for-loopene i 
                                                                                                                              //pluss() og minus() hopper over plass 0 i arrayen når metodene kjøres, og uten disse tomme plassene vil
                                                                                                                              //ikke bruker kunne skru av eller på første lys i raden individuelt. Dette gjelder også for lysRad2[].

int lysRad2 [] = {0, rad2Lys1, rad2Lys2, rad2Lys3, rad2Lys4, rad2Lys5};                                                       //Holder alle LED-lysene på rad 2
int lysRader [] = {rad1Lys1, rad1Lys2, rad1Lys3, rad1Lys4, rad1Lys5, rad2Lys1, rad2Lys2, rad2Lys3, rad2Lys4, rad2Lys5};       //Holder alle LED-lysene i systemet
//ARRAYS - SLUTT------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//PLUSS//
//La bruker skru på LED på en bestemt rad

void pluss(int lysRad[], int antLysPaa){                                                            //Registrer hvilken lysrad og hvilken teller man ønsker å bruke som argumenter                                                      
      
      /*Starter en for-loop som går fra 0 til 5, og 
      sjekker om i er mindre eller like antLysPaa paramteret.
      Hvis if-setningen stemmer vil systemet sette på lyset på den 
      spesifikke plassen i lysraden nevnt som parameter. På denne måten
      vil metoden alldri skru på fler lys enn ønsket, ettersom antLysPaa
      kun øker med 1 ved hvert trykk, og metoden vil dermed alltid slå på 
      ett lys til for hvert trykk*/                                                          
      for(int i = 0; i <= 5; i ++){
        if (i <= antLysPaa){
          digitalWrite(lysRad[i], HIGH);
        }
      }   
   }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//MINUS//
//La bruker skru av LED på en bestemt rad

void minus(int lysRad[], int antLysPaa){                                                            //Registrer hvilken lysrad og hvilken teller man ønsker å bruke som argumenter
    
    /*Denne funksjonen fungerer likt som pluss(), bare at den
    skrur av ett lys om gangen ved hvert trykk på den tildelte
    knappen.*/
    for(int i = 0; i <= 5; i ++){
        if (i >= antLysPaa){
          digitalWrite(lysRad[i], LOW);
        }
      }
  }

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SET//
/*Lar brukeren skru på ønsket antall LED for hver rad, og lagre disse
  verdiene slik at de enkelt kan hentes opp igjen senere. Knappen må være trykket inn
  to ganger innen en gitt tid for å i første omgang starte prosessen med å registrere
  ny set-verdi
 */
void setVerdier(){
  /*Sjekker verdien av knappeTrykk for å se hvilken 
    oppgave systemet skal utføre*/
  
  if (knappeTrykk == 2) {                                                                           //Systemets respons om det er andre gangen knappen trykkes
    antLysPaaRad1 = 0;                                                                              //Setter begge tellere lik 0
    antLysPaaRad2 = 0;                                          
    setLager[0] = antLysPaaRad1;                                                                    //Regisrerer 0-verdiene i setLager[] for å slette forrige set-verdi slik at bruker kan registrere nye
    setLager[1] = antLysPaaRad2;                                
    setKlar();                                                                                      //Spiller av en lyssekvens for å signalisere at bruker kan legge inn ny set-verdi
    alleAv();                                                                                       //Skrur av alle lysene, og bruker kan nå legge inn den nye set-verdien
  }                                                    
  
  if (knappeTrykk == 3) {                                                                           //Systemets respons om det er tredje gangen knappen trykkes
    setLager[0] = antLysPaaRad1;                                                                    //Registrerer de nye rad-verdiene i setLager slik at de enkelt kan hentes opp igjen senere ved behov.
    setLager[1] = antLysPaaRad2;                                
    setFerdig();                                                                                    //Spiller av et lyd- og lyssignal som forteller bruker at verdiene de la inn er lagret
    pluss(lysRad1, antLysPaaRad1);                                                                  //Oppdaterer lysene etter til den nye set-verdien etter systemet har informert om at setLager er oppdatert
    pluss(lysRad2, antLysPaaRad2);   
    EEPROM.put(2, setLager[0]);
    EEPROM.put(3, setLager[1]);
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//REST//
/*Lar brukeren enkelt tilbakestille systemet til set-verdien de registrerte tidligere
  i stedet for å måtte skru på lysene manuelt
 */
void longRest(){
  alleAv();                                                                                    //skrur av alle lysene, rent designvalg som gjør at lyssignal ser penere ut når det settes i gang
  delay(250);                                     
  setFerdig();                                                                                 //Forteller brukeren at verdiene nå vil tilbakestilles til set-verdiene
                                                                                               
  antLysPaaRad1 = setLager[0];                                                                 //Oppdaterer rad-variablene med tilsvarende verdier i setLager arrayen   
  antLysPaaRad2 = setLager[1];                                                                 
  pluss(lysRad1, antLysPaaRad1);                                                               //Oppdaterer lysene slik at de settes tilbake til det som ble lagret i setLager
  pluss(lysRad2, antLysPaaRad2);
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//KNAPPETELLER//
/*Holder oversikt over antall knappetrykk, tid mellom knappetrykk samt hvilke knapper som ble trykket sist.
  Funksjonen vil også hente inn lagreVerdier() og longRest() når spesifiserte kriterier er møtt for å lagre 
  nye lys-verdier, og tilbakestille LED-lysene til den lagrede verdien.
  */
void knappeTeller(String knapp){                                                                        //Tar inn knappens navn i form av en String-variabel som argument
  if (knappeTrykk <= 2){                                                                                //Sjekker at knappen er mindre enn 2 før den sjekker hvilken oppgave som skal utføres
      knappeTrykk ++;                                                                                   //Øker knappeTrykks verdi med 1
                                                                                                        
  /*Systemets reaksjon hvis det er første gangen knappen trykkes siden knappeTrykk ble satt lik 0       
  og oppdaterer verdiene innenfor if-setningen som skrevet under*/                                      
  if (knappeTrykk == 1){                                                                                
    forrigeTrykk = millis();
    knappeNavn = knapp;                                                                                  
  }                                                                                                     
                                                                                                        
                                                                                                        
  /*Hvis det er andre gangen knappen trykkes siden knappeTrykk ble satt lik 0 vil systemet se etter to mulige reaksjoner. Dette
    er for å sikre at ønsket respons kun kommer ved riktig input, og for at systemet skal kunne si ifra dersom bruker taster feil,
    enten ved at de bruker for mye tid eller at de har trykket på forskjellige knapper for å initialisere en funksjon i systemet.
    Systemet vil også enten hente inn og gå gjennom lagreVerdier() eller longRest() ut i fra hvilken knapp som trykkes på
   */                                                                                                   
                                                                                                        
  if (knappeTrykk == 2 && millis() - forrigeTrykk < tid && knappeNavn == knapp) {                       //Sjekker at det er maks to sekunder siden forrige gang knappen trykkes, i tillegg til at det er samme knappen som trykkes på
    forrigeTrykk = millis();                                                                            //Setter forrigeTrykk lik tiden når knappen ble trykket ned for andre gangen
    
    if (knappeNavn == "Set"){                                                                           //Aktiveres om det er set knappen som ble trykket på
      setVerdier();                                                                                     //Henter setVerdier funksjonen
    }
    if (knappeNavn == "Rest"){                                                                          //Aktiveres om det er rest knappen som ble trykket på
      knappeTrykk = 0;                                                                                  //Setter knappeTrykk lik 0 fordi knappesyklusen skal avsluttes dersom rest trykkes på to ganger
      longRest();                                                                                       //Henter longRest funksjonen
    }                                                                                                   
  }                                                                                                     
                                                                                                                                                                                                                                                                                 
  if (knappeTrykk == 2 && millis() - forrigeTrykk > tid || knappeTrykk == 2 && knappeNavn != knapp){    //Slår inn ved to trykk når den if-setningen over ikke møter kriteriene
    forrigeTrykk = millis();                                                                            //Setter forrigeTrykk lik tiden når knappen ble trykket inn
    feilTrykk();                                                                                        //Henter inn feilTrykk() for å fortelle bruker at input ikke ble akseptert
    alleAv();                                                                                           //Skrur alle lysene av gjennom alleAv()
                                                                                                        
    pluss(lysRad1, antLysPaaRad1);                                                                      //Setter lysene tilbake til samme verdi som før systemet spilte av errorsignalet
    pluss(lysRad2, antLysPaaRad2);                                                         
                                                                                                                                                                                                                
    knappeTrykk = 0;                                                                                    //Tilbakestiller knappeTrykk til 0
    knappeNavn = "";                                                                                    //Tilbakestiller knappeNavn til en tom string
  }                                                                                                     
                                                                                                                                                                                                                
  /*Hvis det er tredje gangen trykkes siden knappeTrykk ble satt lik 0 vil systemet se etter to mulige reaksjoner.
    Akkurat som ved trykk to er dette for å sikre at man ikke kan få uønskede reaksjoner fra systemet ved input, og
    samtidig sikre at bruker blir informert ved feil bruk av systemet.                                  
    */                                                                                                  
                                                                                                        
  if (knappeTrykk == 3 && millis() - forrigeTrykk > tid && knappeNavn == knapp){                        //Sjekker at det har gått minst to sekunder siden forrige trykk og at det er samme knappen som trykkes på
    forrigeTrykk = millis();                                                                            //Setter forrigeTrykk lik
    setVerdier();                                                                                       //Henter setVerdier() som reagerer passende utifra knappeTrykks verdi
                                                                                                        
    knappeTrykk = 0;                                                                                    //Tilbakestiller knappeTrykk til 0
    knappeNavn = "";                                                                                    //Tilbakestiller knappeNavn til en tom string
  }                                                                                                     
                                                                                                        
  /*Hvis if-setningen over ikke gikk gjennom vil denne gå i gang, dette gjelder hvis bruker trykker på feil knapp som også tar bruk av denne funksjonen.
    Skjer dette vil brukeren få beskjed om at de har trykket på feil knapp før de får muligheten igjen til å trykke riktig for å lagre. Dette i stedet for å måtte
    legge inn alle set-verdiene på nytt. */                                                             
  if (knappeTrykk == 3 && millis() - forrigeTrykk > tid && knappeNavn != knapp                          //Sjekker først om det har gått mer enn t0 sekunder i tillegg til å sjekke om det er en annen knapp som er trykket på
      ||                                                                                                //Eller
      knappeTrykk == 3 && millis() - forrigeTrykk < tid){                                               //Sjekker om det har gått mindre enn to sekunder siden forrige trykk. Systemet ser om handlingen møter ett av disse to kriteriene 
    
    feilTrykk();                                                                                        //Henter funksjonen for å informere bruker om at de har gitt feil input
                                                                                                         
    pluss(lysRad1, antLysPaaRad1);                                                                      //Setter lysene tilbake til samme verdi som før systemet spilte av errorsignalet
    pluss(lysRad2, antLysPaaRad2);                                                         
                                                                                                        
    knappeTrykk = 2;                                                                                    //Setter knappeTrykk lik 2
                                                                                                        
    if(knapp == "Set"){                                                                                 //Sjekker om knappen som ble trykket på var set knappen, i så fall:
      knappeTrykk = 0;                                                                                  //Tilbakestiller knappeTrykk til 0
      knappeNavn = "";                                                                                  //Tilbakestiller knappeNavn til en tom string
    }
   }
 }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
//SKRU AV ALLE LYS//
void alleAv() {
  for (int i = 0; i < 6; i ++){         //for loop som iterer gjennom antallet pærer i en rad for å skru alle av
     
    digitalWrite(lysRad1[i], LOW);      //Skrur av LED-pæren som er registrert på [i]-plassen i lysradene  
    digitalWrite(lysRad2[i], LOW);      //Skrur av LED-pæren som er registrert på [i]-plassen i lysradene
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SKRU PÅ ALLE LYS//
void allePaa(){
  for (int i = 0; i < 6; i ++){         //for loop som iterer gjennom antallet pærer i en rad for å skru alle på
    
    digitalWrite(lysRad1[i], HIGH);     //Skrur på LED-pæren som er registrert på [i]-plassen i lysradene
    digitalWrite(lysRad2[i], HIGH);     //Skrur på LED-pæren som er registrert på [i]-plassen i lysradene
  }
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SIGNAL VED OPPDATERTE SET-VERDIER//
/*Lyd- og lyssignal for å informere bruker om at set-verdiene er oppdatert.
  Brukes både når bruker lagrer de nye verdiene og når de bruker longRest() for
  å skru LED-ene tilbake til de lagrede verdiene.
 */
void setFerdig(){
  alleAv();                                           //Skrur av alle LED'ene for å få en finere overgang til signalet
  delay(150);                                         //Kort pause før signalet settes i gang
  
  for (int p = 0; p < 2; p ++){                       //for loop som skal sørge for at signalet spilles to ganger.
    for (int i = 0; i < 6; i ++){                     //for loop som sørger for at alle lysene blir skrudd på etter tur
                                                      
    /* For å spille av forskjellige lyder samtidig som pærene
       skrus på er det registrert 6 if-setninger som vil spille
       av lyder utifra hvilke verdier "p" og "i" har, og dermed
       time det med lysene                            
     */                                               
      if (p == 0 && i == 2){                          
        tone(hoyttaler, 600, 125);                    
      }                                               
      if (p == 0 && i == 3 || p == 0 && i == 5){      
        tone(hoyttaler, 800, 125);                    
    }                                                 
      if (p == 0 && i == 4 || p == 0 && i == 6){       
        tone(hoyttaler, 1000, 125);                   
      }                                               
      if (p == 1 && i == 2){                          
        tone(hoyttaler, 600, 125);                    
      }                                               
      if (p == 1 && i == 3 || p == 0 && i == 5){      
        tone(hoyttaler, 800, 125);                    
    }                                                 
      if (p == 1 && i == 4 || p == 0 && i == 6){       
        tone(hoyttaler, 1000, 125);                   
      }                                               
                                                      
      digitalWrite(lysRad1[i], HIGH);                 //Skrur på ett lys på hver rad som står over hverandre
      digitalWrite(lysRad2[i], HIGH);
      delay(75);                                      //lar de stå på en kort periode
      digitalWrite(lysRad1[i], LOW);                  //Skrur av pærene før neste syklus begynner
      digitalWrite(lysRad2[i], LOW);                  
    }                                                 
  }                                                   
  alleAv();                                           //Skrur til slutt av alle lysene
  delay(250);                                         //Legger inn et lite delay for å gi en pause før funksjonen eller handlingen som kommer etter ikke starter med en gang
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SIGNAL VED FEIL INPUT//
void feilTrykk() {
    alleAv();                                     //Skrur av alle LED'ene for å få en finere overgang til signalet
    delay(150);                                   //Kort pause før signalet settes i gang
    for (int p = 0; p < 2; p ++){                 //for loop som skal sørge for at signalet spilles to ganger.
      for (int i = 0; i < 10; i ++) {             //for loop som sørger for at alle lysene blir gått gjennom
                                                  
        /*For å spille av forskjellige lyder samtidig som pærene
          skrus på er det registrert 4 if-setninger som vil spille
          av lyder utifra hvilke verdier "p" og "i" har, og dermed
          time det med lysene                     
        */                                        
        if (p == 0 && i < 5){                     
          tone(hoyttaler, 36, 150);               
          }                                       
        if (p == 0 && i >= 5){                    
          tone(hoyttaler, 63, 200);               
        }                                         
        if (p == 1 && i < 5){                     
          tone(hoyttaler, 36, 150);               
          }                                       
        if (p == 1 && i >= 5){                    
          tone(hoyttaler, 63, 200);               
        }                                         
                                                  
        /*Skrur på alle lysene i en rad og lar de stå på en kort periode 
         før de skrus av, og neste rad skrus på.  
         */                                       
        digitalWrite(lysRader[i], HIGH);          
        i ++;                                     
        digitalWrite(lysRader[i], HIGH);          
        i ++;                                     
        digitalWrite(lysRader[i], HIGH);          
        i ++;                                     
        digitalWrite(lysRader[i], HIGH);          
        i ++;                                     
        digitalWrite(lysRader[i], HIGH);                                               
        delay(150);                               //Et lite delay før neste handling
        alleAv();                                 //Skrur av alle lysene før neste syklusen settes i gang
    }                                             
  }                                               
  alleAv();                                       //Skrur til slutt av alle lysene når systemet har iterert gjennom hele for-loopen
  delay(250);                                     //Legger inn et lite delay for å gi en pause før funksjonen eller handlingen som kommer etter ikke starter med en gang
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//SIGNAL TIL BRUKER AT DE KAN LEGGE INN NYE SET VERDIER//
void setKlar(){
  alleAv();                       //Skrur av alle lys
  delay(75);                      //En kort pause før funksjonen setter i gang
  
  for (int i = 0; i < 3; i ++){
      tone(hoyttaler, 800, 125);
      allePaa();
      delay(125);
      tone(hoyttaler, 1000, 125);
      alleAv();
      delay(125);
      i++;
  }
  
  delay(250);                     //En kort pause før neste handling settes i gang
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//OPPSTARTSSEKVENS//
/*Metode som skal hentes inn ved oppstart for å 
la bruker fortsette der da slapp da artefakten ble brukt sist*/
void oppstart(){
  if (EEPROM.read(0) >= 0 && EEPROM.read(0) <= 5 && EEPROM.read(1) >= 0 && EEPROM.read(1) <= 5){     //Sjekker at verdiene i lagringsplassene som brukes til å hente verdiene fra forrige bruk er mellom 0 og 5
    antLysPaaRad1 = EEPROM.read(lagringsPlass1);                                                     //Setter antLysPaaRad1 til å være det samme som er lagret på plass 0 i EEPROM
    antLysPaaRad2 = EEPROM.read(lagringsPlass2);                                                     //Setter antLysPaaRad1 til å være det samme som er lagret på plass 1 i EEPROM
    setLager[0] = EEPROM.read(2);                                                                    //Setter setLager[0] til å være lik plass 2 i EEPROm, dette for at man skal kunne hente opp set-verdier fra tidligere bruk
    setLager[1] = EEPROM.read(3);                                                                    //Setter setLager[1] til å være lik plass 3 i EEPROm, dette for at man skal kunne hente opp set-verdier fra tidligere bruk
    alleAv();                                                                                           
    delay(250);                                     
    setFerdig();
    pluss(lysRad2, antLysPaaRad2);                                                                   //Henter pluss() metodene for begge lysradene med antLysPaaRad variablene som ble oppdatert over for å skru på riktig antall lys
    pluss(lysRad1, antLysPaaRad1);
  }
  else {                                                                                             //Hvis verdiene i EEPROMs plass 0 og/eller 1 er mer enn 5 eller mindre enn 0 vil systemet la bruker legge inn nye verdier ved start                                                                                        
    knappeTrykk = 1;
    knappeNavn = "Set";
    knappeTeller("Set");
  }
}
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
void setup() {
  
  /*Registrerer hvilke variabler som
  er input og output*/
  
  //Set og Rest
  pinMode(set, INPUT);
  pinMode(rest, INPUT);
  pinMode(hoyttaler, OUTPUT);
  
  //Rad 1
  pinMode(rad1Pluss, INPUT);
  pinMode(rad1Minus, INPUT);
  pinMode(rad1Lys1, OUTPUT);
  pinMode(rad1Lys2, OUTPUT);
  pinMode(rad1Lys3, OUTPUT);
  pinMode(rad1Lys4, OUTPUT);
  pinMode(rad1Lys5, OUTPUT);
  
  //Rad 2
  pinMode(rad2Pluss, INPUT);
  pinMode(rad2Minus, INPUT);
  pinMode(rad2Lys1, OUTPUT);
  pinMode(rad2Lys2, OUTPUT);
  pinMode(rad2Lys3, OUTPUT);
  pinMode(rad2Lys4, OUTPUT);
  pinMode(rad2Lys5, OUTPUT);
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  /*Registrerer hva som skal gjøres ved oppstart av systemet.
  Siden det ikke er noen måte å ha en lagret fil slik systemet settes opp nå 
  vil det alltid startes opp slik at bruker bare kan legge inn og lagre nye set-verdier
  i stedet for å måtte initiere handlingen selv.*/
  

  Serial.begin(9600);      //For å kunne overvåke hvordan systemet oppfører seg i seriell overvåker
                       
  oppstart();              //Henter oppstart() metoden for å la bruker fortsette der de avsluttet
}
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void loop() {

  /*Registrerer hvilke tomme variabler som hører til
    de forskjellige knappene
   */
  setVal = digitalRead(set);
  restVal = digitalRead(rest);
  rad1PlussVal = digitalRead(rad1Pluss);
  rad1MinusVal = digitalRead(rad1Minus);
  rad2PlussVal = digitalRead(rad2Pluss);
  rad2MinusVal = digitalRead(rad2Minus);
//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Set og Rest start
  /*Lar bruker endre set-verdiene gjennom å først dobbeltrykke på knappen
    for så å registrere hvor mange lys som skal være på, og så trykke på denne
    knappen igjen for å lagre de nye verdiene
   */
  if (setVal == HIGH){                            //Utfører handlingen hvis set-knappen trykkes på
    delay(200);                                   //Et kort delay slik at bruker rekker å slippe knappen før systemet registrerer flere knappetrykk
    
    knappeTeller("Set");                          //Henter knappeTeller funksjonen med "Set" som parameter. Funksjonen utfører så jobben sin og utfører den passende handlingen utifra de forskjellige kriteriene som stilles
    }
  
  if (restVal == HIGH){                           //Utfører handlingen hvis rest-knappen trykkes på
    delay(200);                                   //Et kort delay slik at bruker rekker å slippe knappen før systemet registrerer flere knappetrykk
    knappeTeller("Rest");                         //Henter knappeTeller funksjonen med "Set" som parameter. Funksjonen utfører så jobben sin og utfører den passende handlingen utifra de forskjellige kriteriene som stilles
  }
//Set og Rest slutt --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
//Rad 1 start

  if (rad1PlussVal == HIGH){
    delay(200);
    if (antLysPaaRad1 >= 0 && antLysPaaRad1 < 5){                                                   //Sjekker hvor mange lys som er på, hvis ikke alle lysene er på vil systemet gå videre for å skru på neste lys                                                                             //Øker telleren med 1
      antLysPaaRad1 ++; 
      EEPROM.update(lagringsPlass1, antLysPaaRad1);                                                 //Oppdaterer lagringsplassen i EEPROM med den nye antLysPaaRad1 veriden
      pluss(lysRad1, antLysPaaRad1);                                                                //Henter pluss() funksjonen med lysene og lysenes rad som parametre for å skru på det neste lyset i rekken
    }
 }

  if (rad1MinusVal == HIGH){
    delay(200);
   /*Skrur av neste lyset i rekken, og systemet gir bruker feedback ved 
     at lyset slukkes for å signalisere at handlingen deres er utført
   */
   if (antLysPaaRad1 > 0 && antLysPaaRad1 < 6){                                                   //Sjekker hvor mange lys som er av, hvis ikke alle lysene er av vil systemet gå videre for å skru av neste lys
    minus(lysRad1, antLysPaaRad1);                                                                //Henter minus() funksjonen med lysene og lysenes rad som parametre for å skru av det neste lyset i rekken
    antLysPaaRad1 --;                                                                             //Minsker telleren med 1
    EEPROM.update(lagringsPlass1, antLysPaaRad1);                                                 //Oppdaterer lagringsplassen i EEPROM med den nye antLysPaaRad1 veriden
  }
 }
//Rad 1 slutt --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
//Rad 2 start
  /*Her utføres akkurat samme operasjoner som ved rad 1.
      Både for å skru på lys når rad2PlussVal trykkes på, og 
      å skru av lys når rad2MinusVal trykkes på. Disse endringene
      skjer for rad 2.
     */
  if (rad2PlussVal == HIGH){
    delay(200);
     if (antLysPaaRad2 >= 0 && antLysPaaRad2 < 5){
      antLysPaaRad2 ++;
      EEPROM.update(lagringsPlass2, antLysPaaRad2);
      pluss(lysRad2, antLysPaaRad2);
     }
  }
  if (rad2MinusVal == HIGH){
    delay(200);
    if (antLysPaaRad2 > 0 && antLysPaaRad2 < 6){
      minus(lysRad2, antLysPaaRad2);
      antLysPaaRad2 --;
      EEPROM.update(lagringsPlass2, antLysPaaRad2);
      }
    }
//Rad 2 slutt -----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------   
 }
