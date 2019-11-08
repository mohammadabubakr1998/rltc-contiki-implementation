#include "contiki.h"
#include "net/rime/rime.h"
#include "dev/button-sensor.h"
#include <stdio.h>
#include <stdlib.h>
#include "dev/button-sensor.h"
#include "random.c"

/*---------------------------------------------------------------------------*/

PROCESS(example_unicast_process, "Example unicast");
PROCESS(example_broadcast_process, "Broadcast example");
AUTOSTART_PROCESSES(&example_broadcast_process, &example_unicast_process);
/*---------------------------------------------------------------------------*/

//AUTOSTART_PROCESSES(&example_broadcast_process);*/
//--------------------------------------------------///////////////////
// struct Message{
//     int flag;
//     char message[10];
// };

struct Message
{
    int requestType;
    // char message[10];
    //     char request_string[10];
    //     const int targetDevice ; //The target DEVICE
    //     const int requestingNode ; //The requesting deivce
    //     int negativerating[4][4];  // Only for the calculation of Omega negative rating is to be saved;
    float dj[4];
    //     double D[4][4];
    double Fbkdj;
};

struct SendMessage
{
    int requestType;
    double Fbkdj;
};


static const int targetDevice=0;   //The target DEVICE
static const int requesting=0;     //The requesting deivce
static int negativerating[4][4]; // Only for the calculation of Omega negative rating is to be saved;
static float dj[4];
static double D[4][4];

/*---------------------------------------------------------------------------*/
void device()
{
  //   static struct global_Var glb;
  static int i = 2;
  static int deltaT = 4;
  static double x, y;

  static int positiveRating;
  static int negativerating;
  static int j = 0;
  static int m;
  if (deltaT > 0)
  {
    while (j < 4)
    {
      positiveRating = 0;
      negativerating = 0;
      static unsigned short int rnd;
      rnd = random_rand() % 20;
      // printf("rnd:%d\n",rnd);
      m = 0;
      while (m < rnd)
      {

        float tau = (float)(random_rand()) / (float)(RANDOM_RAND_MAX);
        int tau_print = tau*10;
        printf("tau=%d\n",tau_print);
        if (tau >= 0.5)
        {
          positiveRating++;
        }
        else if (tau < 0.5)
        {
          negativerating++;
        }
        m++;
      }
      x = positiveRating + 1;
      y = positiveRating + negativerating;
      if (y == 0)
      {
        dj[j] = 0;
      }
      else if (positiveRating != 0 && negativerating == 0)
      {
        dj[j] = 1;
      }
      else
      {
        y += 2;
        dj[j] = x / y;
      }
      // dj[j]=2;
      // printf("Device: dj=%d",)
      j++;
    }
  }
}

/*--------------------------------------------------------------------------*/

static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{
  // unicast_open(&uc, 146, &unicast_callbacks);

  printf("Receiving the unicast\n");
  static struct Message msg;
  packetbuf_copyto(&msg);
  printf("unicast message received from %d.%d\n",from->u8[0], from->u8[1]);

  printf("Request Type=%d\n", msg.requestType);
}

/*---------------------------------------------------------------------------*/
static void
sent_uc(struct unicast_conn *c, int status, int num_tx)
{
  const linkaddr_t *dest = packetbuf_addr(PACKETBUF_ADDR_RECEIVER);
  if (linkaddr_cmp(dest, &linkaddr_null))
  {
    return;
  }
  printf("unicast message sent to %d.%d: status %d num_tx %d\n",
         dest->u8[0], dest->u8[1], status, num_tx);
}
/*---------------------------------------------------------------------------*/
static const struct unicast_callbacks unicast_callbacks = {sent_uc,recv_uc};
static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/

//---------------------------------------------------------------------------/////

static void
broadcast_recv(struct broadcast_conn *c, const linkaddr_t *from)
{

  static struct Message msg;
  packetbuf_copyto(&msg);
  printf("broadcast message received from %d.%d:\n",
         from->u8[0], from->u8[1]);

  device();
  static int l;
  for(l=0 ; l<4 ; l++)
  {
      msg.dj[l]=dj[l];
      int dj_print = dj[l]*10;
      printf("dj:%d\n",dj_print);
  }
  // msg.dj[] = {0.4,0.7,0.8,1};
  // msg.dj[0] = 0.4;
  // msg.dj[1] = 0.6;
  // msg.dj[2] = 0.9;
  // msg.dj[3] = 1;
  msg.requestType = 3;

  printf("Request no.: %d\n", msg.requestType);
  // sprintf(msg.message,"Trust sent");
  packetbuf_copyfrom(&msg, sizeof(msg));

  printf("Sending Trust array to  %d.%d\n", from->u8[0], from->u8[1]);
  unicast_send(&uc, from);
}
static const struct broadcast_callbacks broadcast_call = {broadcast_recv};
static struct broadcast_conn broadcast;

//---------------------------------------------------------------------------------//////

PROCESS_THREAD(example_unicast_process, ev, data)
{
  PROCESS_EXITHANDLER(unicast_close(&uc);)
  PROCESS_BEGIN();
  SENSORS_ACTIVATE(button_sensor);

  // PROCESS_WAIT_EVENT();


  while (1)
  {

    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event && data == &button_sensor);
    unicast_open(&uc, 146, &unicast_callbacks);

    linkaddr_t addr;

    static struct Message msg; //ist line
    // sprintf(msg.message,"Send Trust");
    msg.requestType = 1;
    // printf("Request type from")
    packetbuf_copyfrom(&msg, sizeof(msg));
    addr.u8[0] = 1;
    addr.u8[1] = 0;
    // printf("Address set\n");
    // PROCESS_WAIT_EVENT();

    if (!linkaddr_cmp(&addr, &linkaddr_node_addr))
    {
      printf("Trust request sending to 1.0\n");
      unicast_send(&uc, &addr);
    }
    // static const struct unicast_callbacks unicast_callbacks = {recv_uc};
    SENSORS_DEACTIVATE(button_sensor);

    // break;
    // SENSORS_ACTIVATE(button_sensor);
    //BUTTON_DISABLE_IRQ();
  }


  PROCESS_END();
}

// PROCESS_EXIT();
// ---------------------------------------------------------------------------

PROCESS_THREAD(example_broadcast_process, ev, data)
{
  static struct etimer et;

  PROCESS_EXITHANDLER(broadcast_close(&broadcast);)

  PROCESS_BEGIN();

  broadcast_open(&broadcast, 129, &broadcast_call);
  broadcast_close(&broadcast);

  PROCESS_END();
}
