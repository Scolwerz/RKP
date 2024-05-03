A program fordításához c fordítóprogram szükséges. (gcc)
Használata: gcc -o chart projekt.c functions.c -fopenmp
Rendszerkövetelmények: (a teljes értékű futtatáshoz)
- Linux operációs rendszer
- Legalább 1MB tárhely
- Legalább 128KB memória
- Minimum 2 szállal rendelkező processzor

Segédlet:
A programot (vagy egy ugyan ilyen funkcióval és visszatérési értékekkel rendelkező programot) a fordítás után a rendeltetésszerű futtatáshoz 2 külön terminálból kell elindítani.
Az egyik terminálban küldő, míg a másikban fogadó módban indítjuk, ugyan azt a kommunikációs módot választva.
Ehhez segítség fordítás után a "./chart --help" parancs futtatásával érhető el.
A program debug/test üzemmódba állítható a "*/" karakterek kódbol való eltávolításával.
Ilyenkor csökkentett számú adattal fog dolgozni a program a könnyebb átláthatóság érdekében.
A program lehetséges kilépési értékei:
0	-  A program rendeltetészerűen lefutott
1	-  A futtatható állomány neve nem megfelelő
2	-  Hibás paraméterezés (Több információért ./chart --help)
3	-  Memóriakezeléssel kapcsolatos probléma
4	-  Fájlkezeléssel kapcsolatos probléma
5	-  Socketekkel kapcsolatos probléma
10	-  A Socket által küldött és visszakapott értékek nem egyeznek
11	-  A küldött adatok bájtban megadott mérete nem egyezik
Függvények és Metódusok:
- void print_version_info()
Kiírja a program információit (több szál használatával)
- void print_help_info();
Kiírja a program használatához szükséges információkat
- int Measurement(int** Values);
Gép által generált méréseket állít elő, amit a paraméterben megkapott memóriaterületen tárol elő
Visszatérési értéke: Tárolt adatok mennyisége (Hiba esetén -1)
- unsigned char* u_int_bytes_little(unsigned int value);
Visszaadja a paraméterként kapott unsigned int bájtjait hexadecimális formában, little-endian bájtsorrendben
- unsigned char* u_int_bytes_big(unsigned int value);
Visszaadja a paraméterként kapott unsigned int bájtjait hexadecimális formában, big-endian bájtsorrendben
- unsigned int pack_rgba(int r, int g, int b, int a);
Paraméterként kap 4 int értéket (ez esetben egy rgba kódot), amit összefog egy unsigned int változóban, majd azt visszaadja
- void BMPcreator(int *Values, int NumValues);
Látrehoz egy egybites színmészségű bmp fájlt "chart.bmp" néven, melyen az előállított mérések grafikonja látható
- int FindPID();
A /proc könytárban lévő, számmal kezdődő alkönyvtárak status fájlait vizsgálja, egy másik, chart nevű, fogadó üzemmódban futó folyamatot keresve
Visszatérési értéke: A megtalált folyamat process ID-ja (Ha nincs találat -1)
- void SendViaFile(int *Values, int NumValues);- 
Paraméterként kap egy tömböt, és annak a méretét. Létrehozza a "Measurement.txt fájlt az adott felhasználó alapértelmezett könyvtárában, és abba írja a kapott tömb elemeit
- void ReceiveViaFile(int sig);
Az adott felhasználó alapértelmezett könyvtárában lévő "Measurement.txt fájlt tartalmát feldolgozza dinamikus memóriakezeléssel, majd az értékek tömbjével és mennyiségével meghívja a BMPcreator függvényt
- void SendViaSocket(int *Values, int NumValues);
Létrehoz egy UDP klienst, amely a localhost 3333-as portját figyelő szerverrel kommunikál.
Paraméterei egy tömb, és annak elemszáma, amit kapcsolatteremtés után elküld a szervernek
- void ReceiveViaSocket();
Létrehoz egy UDP szervert, amely kapcsolatot teremt a klienssel, majd a tőle kapott tömböt elmenti és feldolgozza a BMPcreator függvény meghívásával
- void SignalHandler(int sig);
Kezeli a beérkező signal-okat
