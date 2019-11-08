/*
 * Copyright (c) 2007, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Best-effort single-hop unicast example
 * \author
 *         Adam Dunkels <adam@sics.se>
 */

#include "contiki.h"
#include "net/rime/rime.h"
#include <stdio.h>
#include <clock.h>

// #include "math.h"

// struct Message{
//     int flag;
//     char message[10];
//     char request_string[10];
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

static double D[4][4];
static double dj[4];
static int allTrustReceivedCounter = 0;
static double Fbkdj;
static unsigned char requestor;
static int target = 3;
/*---------------------------------------------------------------------------*/

static double log(double n, double r)
{
    return (n > r - 1.0) ? 1 + log(n / r, r) : 0.0;
}

static double calculatePij(int i, int j)
{
    // static struct global_Var glb;
    double Pij; //Return Variable

    double Device = D[i][j]; //To save the value from the array D
    double x = 0;            //To use for the addition
    int K = 0;
    //Below calculating ∑ Ddidj(Δt)
    while (K < 4)
    {
        x = x + D[K][j];
        K++;
    }

    Pij = Device / x;

    return Pij;
}
static double calculatePijlnPij(int j)
{

    double x = 0; //Return variable and to use in the loop for addition
    double Pij;
    double PijlnPij;
    int i =0;
    while (i < 4)
    {
        Pij = calculatePij(i, j);

        if (Pij == 0)
            PijlnPij = 0;
        else
            PijlnPij = Pij * log(Pij, 2.718);

        x = x + PijlnPij;
        i++;
    }

    return x;
}
static double calculateSumE(double E[])
{

    double x = 0; // Return variable and to be used for calculating sum
    int i = 0;
    while (i < 4)
    {
        x = x + E[i];
        i++;
    }

    return x;
}

static double broker(int target)
{
    // static struct global_Var glb;
    double Fbkdj = 0; //Final output / Return variable

    double random = 0; //Only to complete function requirement of RETURN double

    double E[4]; //To store the value of Ei
    double W[4]; // To store the value of Wi;

    double sE; //Sum of Ei;

    double EiTemp; //used in the calculation of Ei

    // Calculating natural log of N;
    double ln = log(4, 2.718);
    printf("Log Value: %d\n",ln*10);
    ln = -(ln);
    ln = 1 / ln;

    int i = 0;
    int j = 0;
    //To make a matrix from the device to device trust values
    // while( i<4 )
    // {
    //     device(); //Assign values to the dj[], The trust values from a device

    //     while( j<4)
    //     {
    //         D[i][j] = dj[j];
    //         j++;
    //     }
    //     i++;
    // }

    // Calculating Ei
    i = 0;
    while (i < 4)
    {
        j = 0;
        while (j < 4)
        {
            EiTemp = ln * calculatePijlnPij(j);

            // printf("%lf\t", EiTemp);; // Temporary only for troubleshooting------------------------

            if (i == j)
            {
                E[i] = EiTemp;
            }
            j++;
        }
        i++;
        printf("\n");
    }

    //Below code is only to display value of E[i] (Only for troubleshooting)----------------------
    printf("\n\nValues of Ei\n");
    i = 0;
    while (i < 4)
    {
        // printf("%lf\t", E[i] );
        i++;
    }
    printf("\n");
    // Till this

    // Calculating Wi
    sE = calculateSumE(E);
    i = 0;
    while (i < 4)
    {
        W[i] = (1 - E[i]) / (4 - sE);
        i++;
    }
    i = 0;
    while (i < 4)
    {
        Fbkdj = Fbkdj + (D[i][target] * W[i]);
        i++;
    }

    return Fbkdj;
    printf("Fbkdj infunction:%d\n",Fbkdj*100);
}

/*---------------------------------------------------------------------------*/
PROCESS(example_unicast_process, "Broker");
AUTOSTART_PROCESSES(&example_unicast_process);

static const struct broadcast_callbacks broadcast_call = {};
static struct broadcast_conn broadcast;

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
static void
recv_uc(struct unicast_conn *c, const linkaddr_t *from)
{


    static struct unicast_conn uc;
    static struct Message msg;
    static struct SendMessage msg1;




    // static struct linkaddr_t

    // static struct etimer broadcast_et;
    // static clock_time_t t = 5*CLOCK_SECOND;
    // etimer_set(&broadcast_et,5*CLOCK_SECOND);

    packetbuf_copyto(&msg);

    static int deviceNo;

    if (msg.requestType == 1)
    {
        printf("Trust Request Received.%u.%u\n", from->u8[0], from->u8[1]);

        broadcast_open(&broadcast, 129, &broadcast_call);
        printf("Sending Broadcast to all\n");

        msg.requestType = 2;
        requestor = from->u8[0];
        printf("Requestor no: %d\n",requestor);

        packetbuf_copyfrom(&msg, sizeof(msg));
        broadcast_send(&broadcast);
    }
    else if (msg.requestType == 3)
    {
        printf("Trust value recieved from. %u.%u\n", from->u8[0], from->u8[1]);
        static int k, q;
        deviceNo = (from->u8[0] - 2);
        //static char trust[4];
        for (k = 0; k < 4; k++)
        {
            //    gcvt(msg.dj[k],4,trust);
            //    printf("dj: %d\n",msg.dj[k]*10);
            //    dj[k]=msg.dj[k];
            D[deviceNo][k] = msg.dj[k];
            int abc = D[deviceNo][k] * 10;
            printf("TrustDevice:%u--D[%d][%d]=%d\n",from->u8[0],deviceNo, k, abc);
        }

        allTrustReceivedCounter++;
        printf("Trust Counter %d\n",allTrustReceivedCounter);
        // printf("Coiunter:%d\n",allTrustReceivedCounter);
        if (allTrustReceivedCounter == 3)
        {
            static const struct unicast_callbacks unicast_callbacks = {sent_uc};
            unicast_open(&uc, 146, &unicast_callbacks);
            // static struct linkaddr_t *addr;
            linkaddr_t addr;
            Fbkdj = broker(target);


            // printf("Fbkdj:%d",Fbkdj*10);


            // printf("Requ")



            msg.Fbkdj = Fbkdj;
            // msg.requestType = msg.requestType%17060;
            msg.requestType = 4;
            msg.dj[0]=0.1;
            msg.dj[1]=0.1;
            msg.dj[2]=0.1;
            msg.dj[3]=0.1;
            int msg_Fbkdj_print = msg.Fbkdj*10;



            packetbuf_copyfrom(&msg, sizeof(msg));
            addr.u8[0] = requestor;
            addr.u8[1] = 0;
            printf("Fbkdk:%d RequestType:%d Addr[0]:%d Addr[1]:%dh\n",msg_Fbkdj_print,msg.requestType,addr.u8[0],addr.u8[1]);
            if (!linkaddr_cmp(&addr, &linkaddr_node_addr))
            {

                printf("Sending Trust to Requestor\n");
                unicast_send(&uc, &addr);
            }

        }
    }
}



static const struct unicast_callbacks unicast_callbacks = {recv_uc, sent_uc};

static struct unicast_conn uc;
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(example_unicast_process, ev, data)
{
    PROCESS_EXITHANDLER(unicast_close(&uc);)

    PROCESS_BEGIN();

    unicast_open(&uc, 146, &unicast_callbacks);

    printf("Broker Process\n");

    //   while(1) {
    //     static struct etimer et;
    //     linkaddr_t addr;

    //     etimer_set(&et, CLOCK_SECOND);

    //     PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));

    //     packetbuf_copyfrom("Hello", 5);
    //     addr.u8[0] = 1;
    //     addr.u8[1] = 0;
    //     if(!linkaddr_cmp(&addr, &linkaddr_node_addr)) {
    //       unicast_send(&uc, &addr);
    //     }

    //   }

    PROCESS_END();
}
/*---------------------------------------------------------------------------*/
