#define JAMCAPI 1

#include "typedefs.h"


static sdword crc32tab[256] = {
    0L, 1996959894L, -301047508L, -1727442502L, 124634137L,
    1886057615L, -379345611L, -1637575261L, 249268274L, 2044508324L,
    -522852066L, -1747789432L, 162941995L, 2125561021L, -407360249L,
    -1866523247L, 498536548L, 1789927666L, -205950648L, -2067906082L,
    450548861L, 1843258603L, -187386543L, -2083289657L, 325883990L,
    1684777152L, -43845254L, -1973040660L, 335633487L, 1661365465L,
    -99664541L, -1928851979L, 997073096L, 1281953886L, -715111964L,
    -1570279054L, 1006888145L, 1258607687L, -770865667L, -1526024853L,
    901097722L, 1119000684L, -608450090L, -1396901568L, 853044451L,
    1172266101L, -589951537L, -1412350631L, 651767980L, 1373503546L,
    -925412992L, -1076862698L, 565507253L, 1454621731L, -809855591L,
    -1195530993L, 671266974L, 1594198024L, -972236366L, -1324619484L,
    795835527L, 1483230225L, -1050600021L, -1234817731L, 1994146192L,
    31158534L, -1731059524L, -271249366L, 1907459465L, 112637215L,
    -1614814043L, -390540237L, 2013776290L, 251722036L, -1777751922L,
    -519137256L, 2137656763L, 141376813L, -1855689577L, -429695999L,
    1802195444L, 476864866L, -2056965928L, -228458418L, 1812370925L,
    453092731L, -2113342271L, -183516073L, 1706088902L, 314042704L,
    -1950435094L, -54949764L, 1658658271L, 366619977L, -1932296973L,
    -69972891L, 1303535960L, 984961486L, -1547960204L, -725929758L,
    1256170817L, 1037604311L, -1529756563L, -740887301L, 1131014506L,
    879679996L, -1385723834L, -631195440L, 1141124467L, 855842277L,
    -1442165665L, -586318647L, 1342533948L, 654459306L, -1106571248L,
    -921952122L, 1466479909L, 544179635L, -1184443383L, -832445281L,
    1591671054L, 702138776L, -1328506846L, -942167884L, 1504918807L,
    783551873L, -1212326853L, -1061524307L, -306674912L, -1698712650L,
    62317068L, 1957810842L, -355121351L, -1647151185L, 81470997L,
    1943803523L, -480048366L, -1805370492L, 225274430L, 2053790376L,
    -468791541L, -1828061283L, 167816743L, 2097651377L, -267414716L,
    -2029476910L, 503444072L, 1762050814L, -144550051L, -2140837941L,
    426522225L, 1852507879L, -19653770L, -1982649376L, 282753626L,
    1742555852L, -105259153L, -1900089351L, 397917763L, 1622183637L,
    -690576408L, -1580100738L, 953729732L, 1340076626L, -776247311L,
    -1497606297L, 1068828381L, 1219638859L, -670225446L, -1358292148L,
    906185462L, 1090812512L, -547295293L, -1469587627L, 829329135L,
    1181335161L, -882789492L, -1134132454L, 628085408L, 1382605366L,
    -871598187L, -1156888829L, 570562233L, 1426400815L, -977650754L,
    -1296233688L, 733239954L, 1555261956L, -1026031705L, -1244606671L,
    752459403L, 1541320221L, -1687895376L, -328994266L, 1969922972L,
    40735498L, -1677130071L, -351390145L, 1913087877L, 83908371L,
    -1782625662L, -491226604L, 2075208622L, 213261112L, -1831694693L,
    -438977011L, 2094854071L, 198958881L, -2032938284L, -237706686L,
    1759359992L, 534414190L, -2118248755L, -155638181L, 1873836001L,
    414664567L, -2012718362L, -15766928L, 1711684554L, 285281116L,
    -1889165569L, -127750551L, 1634467795L, 376229701L, -1609899400L,
    -686959890L, 1308918612L, 956543938L, -1486412191L, -799009033L,
    1231636301L, 1047427035L, -1362007478L, -640263460L, 1088359270L,
    936918000L, -1447252397L, -558129467L, 1202900863L, 817233897L,
    -1111625188L, -893730166L, 1404277552L, 615818150L, -1160759803L,
    -841546093L, 1423857449L, 601450431L, -1285129682L, -1000256840L,
    1567103746L, 711928724L, -1274298825L, -1022587231L, 1510334235L,
    755167117L
};


/*
**  Calculate 32-bit CRC on block. Initial 'seed' to crc32() should be
**  -1L.
*/
dword JAMsysCrc32(void *pBuf, unsigned int len, dword crc)
{
    byte *Ptr = pBuf;

    for (; len; len--, Ptr++)
        crc = (crc >> 8) ^ crc32tab[(int)((crc ^ *Ptr) & 0xffUL)];
    return (crc);
}

/* end of file "jamcrc32.c" */
