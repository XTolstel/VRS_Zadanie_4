# Kontrola zadania 4 na fyzickej doske

Datum: 29.10.2026
Doska: NUCLEO-F303K8
ST-LINK: 066BFF495649657867094247
Seriovy port: COM5, 115200, 8N1, bez flow control.

## Vysledok
FINALNY HARDVEROVY TEST: VSETKY KONTROLY PRESLI.

Pri prvom zostaveni bola odhalena a opravena poskodena implementacia
funkcie proccesDmaData v main.c. Nasledne bol projekt uspesne zostaveny,
nahraty do dosky a obsah flash pamate bol overeny programatorom.

Zostavenie: 0 chyb, 1 upozornenie linkera na RWX LOAD segment
v povodnom linker skripte sablony. Nejde o chybu prekladu C.
Velkost: text 10808 B, data 92 B, bss 4812 B.

## Funkcne kontroly
- Pravidelne spravy o buffri: prijate; intervaly priblizne 2000 ms.
- Znaky bez uvodneho # sa ignoruju.
- Priklad #Platn15uborZnakov$: 13 malych, 2 velke pismena.
- Rozdelenie #Ab12 a cD$ do dvoch davok: 2 male, 2 velke.
- Pred ukoncovacim znakom sa nevysiela vysledok.
- Dva ramce v jednej davke sa spracuju samostatne.
- Ukoncovaci znak na 35. pozicii po # je akceptovany.
- 35 znakov bez ukoncenia sposobi zahodenie retazca.
- Po zahodeni sa novy ramcek normalne spracuje.
- Prazdny ramcek, cislice a interpunkcia: spravne nulove pocty.
- 64 po sebe iducich ramcov: vsetky prijate presne raz, so spravnymi
  poctami a v spravnom poradi; prenos prekrocil kapacitu DMA buffra viackrat.
- Format a percento obsadenosti DMA buffra su spravne.
- Pri 108 obsadenych bajtoch doska hlasi 84.4 %.
- Po dalsej 20-bajtovej davke bol v konecnom teste buffer opat na 0 bajtoch.
- Po tomto opakovnom spusteni bol #AfterTC$ spravne vyhodnoteny:
  4 male a 3 velke pismena.

Vsetky zapisovane davky mali najviac 20 znakov.
USB/ST-LINK moze jednu davku rozdelit na mensie casti s IDLE medzerou.
V takom pripade je pripustny aj skorsi restart DMA; prva skuska to
ukazala pri obsadenosti 124 bajtov, po ktorej sa prijali dalsie 4 bajty.
Preto sa spravne overuje zachovanie dat a dostatok volneho miesta,
nie povinne vynulovanie po kazdom zapise 20 znakov z PC.

## Registre precitane z dosky
DMA1 Channel6 CCR = 0x00002087:
EN=1, TCIE=1, HTIE=1, CIRC=0, DIR=0, MINC=1, vysoka priorita.

USART2 CR1 = 0x0000001D:
UE=1, RE=1, TE=1, IDLEIE=1.

## Zaznamy
V nadradenom priecinku Cv4:
- cv4-build.log
- cv4-flash.log
- cv4-hardware-test.log
- cv4-registers.log

Funkcne overenie sa vztahuje na uvedene scenare a predpoklady zadania.
Nevykonaval sa dlhotrvajuci zatazovy test ani neobmedzene zahlcovanie TX.
