#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <time.h>

#include "msgapi.h"


void PrintStringList(STRINGLIST * l);
char *StringTime(dword t);
char *AscAddress(NETADDR * addr);
void rton(char *s);
char *attr_to_txt(dword attr1, dword attr2);



void main(void)
{
    MSG *ahandle;
    MSGH *msghandle;
    MIS mis;
    int i;
    struct _minf minf;
    dword current, high;
    dword bodylen, ctrllen;
    static char body[1024], ctrl[512];

    minf.def_zone = 0;
    minf.req_version = 0;

    MsgOpenApi(&minf, "", 1, "/tmp/");

    memset(&mis, '\0', sizeof(MIS));

    strcpy(mis.from, "Gerard van Essen");
    strcpy(mis.to, "Geaddresseerde");
    strcpy(mis.subj,
           "1111111111222222222233333333334444444444555555555566666666667777777777**");

    mis.origfido.zone = 2;
    mis.origfido.net = 281;
    mis.origfido.node = 527;
    mis.origfido.point = 0;

    strcpy(mis.origdomain, "fidonet");

    mis.destfido.zone = 2;
    mis.destfido.net = 500;
    mis.destfido.node = 26;
    mis.destfido.point = 0;

    mis.msgwritten = JAMsysTime(NULL);

    mis.attr1 = aLOCAL | aPRIVATE | aKILL | aIMM;
    mis.attr2 = 0L;

    mis.via =
        AddToStringList(mis.via, "2:281/527 @1234566 NetMgr 2.00+", NULL,
                        0);
//   mis.seenby = AddToStringList(mis.seenby, "2:281/527", NULL, 0);
//   mis.path = AddToStringList(mis.path, "2:281/500 2:281/1", NULL, 0);

//   mis.requested = AddToStringList(mis.requested, "timed_file", "tim1", 0);
//   mis.requested = AddToStringList(mis.requested, "netmgr_file", "net2", 0);
//   mis.requested = AddToStringList(mis.requested, "eerste_file", "1", 0);
//   mis.requested = AddToStringList(mis.requested, "tweede_file", "2", 0);
//   mis.requested = AddToStringList(mis.requested, "derde_file", "3it", 0);
//   mis.requested = AddToStringList(mis.requested, "vierde_file", "4it", 0);
//   mis.requested = AddToStringList(mis.requested, "vijfde_file", "5it", 0);
//   mis.requested = AddToStringList(mis.requested, "zesde_file", "6it", 0);
//   mis.requested = AddToStringList(mis.requested, "zevende_file", "7it", 0);
//   mis.requested = AddToStringList(mis.requested, "achtste_file", "8it", 0);
//   mis.requested = AddToStringList(mis.requested, "negende_file", "9it", 0);
//   mis.requested = AddToStringList(mis.requested, "tiende_file", "10t", 0);
//   mis.requested = AddToStringList(mis.requested, "elfde_file", "11t", 0);

//   mis.attached = AddToStringList(mis.attached, "c:\\autoexec.bat", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000001.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000002.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000003.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000004.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000005.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000006.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000007.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000008.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000009.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000010.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000011.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000012.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000013.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000014.MSG", NULL, 0);
//   mis.attached = AddToStringList(mis.attached, "c:\\tcpip\\etc\\rnspool\\artware\\0000015.MSG", NULL, 0);

    strcpy(body, "Hallo!\rDit is de body!\r\rKijk: {|}\r\rGerard\r");
    for (i = 0; i < 70; i++)
        strcat(body, "123456789\r");
    strcpy(ctrl, "\01MSGID: 2:281/527 00112233");

    if ((ahandle = MsgOpenArea("/tmp/",
                               MSGAREA_CRIFNEC,
                               MSGTYPE_SDM | MSGTYPE_NET)) == NULL)
    {
        printf("\nError opening area\n");
        exit(254);
    }

    MsgLock(ahandle);

    if ((msghandle = MsgOpenMsg(ahandle, MOPEN_CREATE, 0L)) == NULL)
    {
        printf("Error opening message!\n");
        exit(254);
    }

    if (MsgWriteMsg(msghandle,
                    0,
                    &mis,
                    body,
                    strlen(body),
                    strlen(body), strlen(ctrl) + 1, ctrl) == -1)
    {
        printf("Error writing message!\n");
        exit(254);
    }

    if (MsgCloseMsg(msghandle) == -1)
    {
        printf("\nError closing message!\n");
        exit(254);
    }

    MsgCloseArea(ahandle);

    MsgCloseApi();

}
