#include "contiki.h"
#include "contiki-net.h"
#include "board-peripherals.h"
#include "http-socket/http-socket.h"
#include "ip64-addr.h"


/* Utilisé pour afficher les adresses IP sur la liaison série PRINT6ADDR */
#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#define SENSOR_READ_PERIOD (CLOCK_SECOND)
/* URL du service Maker de IFTT */
#define IFTTT_MAKER_URL "http://maker.ifttt.com/trigger/meuh/with/key/"
#define IFTTT_URL IFTTT_MAKER_URL MAKER_KEY

/* https://github.com/contiki-os/contiki/wiki/Processes */
/* On déclare le processus bam (boite à meuh) */
PROCESS_NAME(bam_process);
PROCESS(bam_process, "\"Boite a meuh\" Process");
/* on veut que le processus bam soit lancé automatiquement au démarrage du capteur */
AUTOSTART_PROCESSES(&bam_process);


/* Stucture Contiki permettant de manipuler les socket HTTP*/
static struct http_socket http;

void meuh(void)
{
    /* envoi de la requête au serveur IFTTT signalant que le capteur à été retourné */
    http_socket_get(&http, IFTTT_URL, 0, 0, NULL, NULL);
    printf("MEUH\n");
    http_socket_close(&http);
}

static void
_accelerometer_handler(void)
{
    static int z = 0;
    int val = 0;

    /* On récupère la valeur de l'axe Z de l'accéléromètre */
    val = mpu_9250_sensor.value(MPU_9250_SENSOR_TYPE_ACC_Z);
    /* si elle a changé de signe par rapport à la précedente lecture on meugle */
    if (val < 0 && z >= 0)
    {
        meuh();
        z = val;
    }
    else if (val >= 0 && z < 0)
    {
        meuh();
        z = val;
    }
    /* On réactive les capteurs */
    mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);
}

PROCESS_THREAD(bam_process, ev, data)
{
    /* Structure la manipulation des timers dans Contiki*/
    static struct etimer et;
    uip_ip4addr_t ip4addr;
    uip_ip6addr_t ip6addr;

    PROCESS_BEGIN();

    printf("The \"Boite à Meuh\" ipv6 ready\n");
    printf("Looking for IP addr\n");

    /* transformation de l'adresse du dns google en ipv6. */
    /* On utilise l'adresse ipv4 car le service IFTTT ne dispose pas d'ipv6 */
    /* donc on veut que DNS nous donne des adresses ipv4. */
    uip_ipaddr(&ip4addr, 8,8,8,8);
    ip64_addr_4to6(&ip4addr, &ip6addr);
    /* Mise a jour du dns de uip */
    uip_nameserver_update(&ip6addr, UIP_NAMESERVER_INFINITE_LIFETIME);
    /* on arme un timer de 1s */
    etimer_set(&et, CLOCK_SECOND);
    /* initialisation de la structure permettant de faire des requête http */
    http_socket_init(&http);
    /* activation des capteurs du Sensortag */
    mpu_9250_sensor.configure(SENSORS_ACTIVE, MPU_9250_SENSOR_TYPE_ALL);

    while(1)
    {
        /* Le timer a expiré */
        if(ev == PROCESS_EVENT_TIMER && etimer_expired(&et))
        {
            /* récuperation de l'adresse ip du capteur*/
            uip_ds6_addr_t *addr = uip_ds6_get_global(ADDR_PREFERRED);
            /* Si on est pas encore configuré on réarme le timer*/
            if( addr == NULL)
            {
                etimer_set(&et, CLOCK_SECOND);
            }
            /* Sinon on affiche l'adresse */
            else
            {
                printf("New Address : ");
                PRINT6ADDR(&addr->ipaddr);
                printf("\n");
            }
        }
        /* les valeurs des capteurs ont été mises à jour */
        if (ev == sensors_event && data == &mpu_9250_sensor)
        {
            /* on lit les valeur et on meugle si besoin */
            _accelerometer_handler();
        }

        /* on rend la main au autre processus */
        PROCESS_YIELD();
    }

    PROCESS_END();
}
