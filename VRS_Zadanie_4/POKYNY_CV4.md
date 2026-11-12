# VRS - Zadanie 4 - Oleh Tolstel

Zaklad projektu: VRS-Predmet/vrs_cvicenie_4, vetva zadanie_cv4.
Povodne projekty Cv1, Cv2 a Cv3.6 neboli zmenene.

## Otvorenie
STM32CubeIDE: File -> Import -> General -> Existing Projects into Workspace.
Vybrat tento priecinok. Nazov projektu: VRS_Zadanie_4.
Nevytvarat novy projekt z .ioc; odovzdava sa doplnena sablona.
Projekt pouziva prilozene LL a CMSIS kniznice.

## Komunikacia s PC
Doska NUCLEO-F303K8, pripojenie USB cez ST-LINK.
Terminal: virtualny COM port dosky, 115200 baud, 8 datovych bitov,
bez parity, 1 stop bit, bez flow control.
USART2: PA2 TX, PA15 RX. Dalsi hardver nie je potrebny.
Jedna odoslana davka ma najviac 20 znakov, ako predpoklada zadanie.
Dlhsi ramcek sa moze poslat po castiach. Medzi davkami musi vzniknut
IDLE (najmenej jeden cas znaku), aby MCU vedel rozlisit jednotlive davky.

## Splnene casti
- Src/usart.c: konfiguracia USART2 a DMA cez LL.
- RX: DMA1 kanal 6, 128 bajtov, NORMAL; nie circular.
- TX: DMA1 kanal 7, NORMAL.
- Prijem cez HT, TC a IDLE, bez polling prijmu v hlavnej slucke.
- USART2_CheckDmaReception spracuje iba nove bajty.
- Ak zostane menej ako 20 volnych bajtov, nevyhodnotene bajty sa
  skopiruju a DMA sa znovu spusti od zaciatku so 128 volnymi bajtmi.
  Predtym spracovane bajty sa neposielaju callbacku druhykrat.
- proccesDmaData ignoruje znaky pred #, zachova stav medzi davkami
  a po $ ulozi text a pocet malych/velkych ASCII pismen.
- Globalna premenna letter_count typu letter_count_ obsahuje pocty
  posledneho platneho retazca.
- Ak v nasledujucich 35 znakoch po # nepride $, retazec sa zahodi.
  Znak $ na 35. pozicii je platny (najviac 34 znakov obsahu).
  Dalsi # pocas rozpracovaneho retazca je bezny znak obsahu;
  novy zaciatok sa hlada az po ukonceni alebo zahodeni retazca.
- Platny text sa odosle bez ohranicujucich znakov # a $.
- Statistiky DMA sa odosielaju kazde 2 sekundy (0.5 Hz).
- Obsadenost znamena pocet adries zapisanych v aktualnom DMA prenose,
  teda 128 - CNDTR. Po opatovnom spusteni klesne na nulu.
- Percento sa formatuje celociselne na 1 desatinne miesto, preto
  netreba pridavat podporu printf pre float.
- SysTick pocita milisekundy; v preruseniach nie su blokovacie prenosy.
- Hotove retazce cakaju vo fronte, kym sa uvolni TX. TX buffer zostava
  nezmeneny pocas DMA prenosu. Fronta ma 63 pouzitelnych miest.
  Pri automatickom posielani treba vyckat na odpovede, pretoze vystupna
  sprava je dlhsia ako vstupny retazec. Pri zahlteni sa zvysi
  diagnosticka premenna dropped_strings.

## Priklad pouzitia
Vstup:
#Platn15uborZnakov$

Odpoved:
Valid string: Platn15uborZnakov, lower-case: 13, upper-case: 2

Vstup:
#Ab12cD$

Odpoved:
Valid string: Ab12cD, lower-case: 2, upper-case: 2

Periodicka sprava ma tvar:
Buffer capacity: 128 bytes, occupied memory: 24 bytes, load [in %]: 18.8%

## Stav
Projekt bol zostaveny, nahraty cez ST-LINK a overeny na doske NUCLEO-F303K8 cez COM5.
Vsetky funkcne kontroly vo finalnom hardverovom teste presli.
Podrobnosti su v subore VYSLEDOK_KONTROLY.md; zaznamy v nadradenom priecinku Cv4.
