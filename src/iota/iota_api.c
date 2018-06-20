/*
 * iota_api.c
 *
 *  Created on: Jun 13, 2018
 *      Author: max
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "iota_api.h"
#include "../curl-request.h"

void iotaApi_getBalance(addressObj_t* addr)
{
	uint8_t cmd[255];
	uint8_t resp[1024];
	//uint8_t addr[82];
	char* ptr;

	sprintf(cmd, "{\"command\": \"getBalances\", \"addresses\": [\"%.81s\"], \"threshold\": 100}", addr->pubKey);
	request(NODE_URL, NODE_PORT, cmd, resp);

	ptr = strstr(resp, "balance");
	addr->value = strtol(ptr+12,NULL,10);

}

void iotaApi_wereAddressesSpentFrom(addressObj_t* addr)
{
	uint8_t cmd[255];
	uint8_t resp[1024];
	//uint8_t addr[82];
	char* ptr;

	sprintf(cmd, "{\"command\": \"wereAddressesSpentFrom\", \"addresses\": [\"%.81s\"]}", addr->pubKey);
	request(NODE_URL, NODE_PORT, cmd, resp);

	ptr = strstr(resp, "\"states\":[true]");
	if(ptr != NULL)
		addr->spentState = 1;
	else
		addr->spentState = 0;

}

int8_t iotaApi_getTransactionsToApprove(uint8_t* trunkTx, uint8_t* branchTx)
{
	uint8_t cmd[255];
	uint8_t resp[255];
	char* ptr;

	request(NODE_URL, NODE_PORT, "{\"command\": \"getTransactionsToApprove\", \"depth\": 27}", resp);

	ptr = strstr(resp,"trunkTransaction\":");
	if(ptr == NULL)
	{
		return -1;
	}

	memcpy(trunkTx, ptr+19,81);

	ptr = strstr(resp, "branchTransaction\":");
	if(ptr == NULL)
	{
		return -1;
	}

	memcpy(branchTx, ptr+20, 81);

	return 0;

}

int8_t iotaApi_attachToTangle(uint8_t* txChars, uint8_t* trunkTx, uint8_t* branchTx, const uint8_t nTx, uint8_t* trytes)
{
	uint8_t* cmd;

	cmd = malloc(283+(nTx*2673));

	if(cmd == NULL || trytes == NULL)
	{
		perror("attachToTangle");
		return -1;
	}

	sprintf(cmd, "{\"command\": \"attachToTangle\","
			"\"trunkTransaction\": \"%.81s\","
			"\"branchTransaction\": \"%.81s\","
			"\"minWeightMagnitude\": 14, "
			"\"trytes\": [",trunkTx, branchTx);

	for(int i=0;i<nTx;i++)
	{
		strcat(cmd, "\"");
		strncat(cmd, txChars+(i*2673),2673);
		if(i == (nTx-1))
		{
			strcat(cmd, "\"]}");
		}
		else
		{
			strcat(cmd, "\",");
		}

	}

	printf("[info] : %s\n",cmd);
	request(NODE_URL, NODE_PORT, cmd, trytes);

	free(cmd);

	return 0;
}

void iotaApi_interruptAttachingToTangle(void)
{
	uint8_t cmd[255];
	uint8_t resp[255] = {0};
	char* ptr;

	request("iotanode.host", 14265, "{\"command\": \"interruptAttachingToTangle\"}", resp);
	printf("[info] : interrupt Attaching to Tangle response : %s",resp);
}

int8_t iotaApi_broadcastTransactions(uint8_t* broadcastTx, uint8_t nTx)
{
	uint8_t* cmd;
	uint8_t resp[1024];
	char* ptr;

	cmd = malloc(100+(nTx*2673));
	if(cmd == NULL)
	{
		perror("broadcastTransaction");
		exit(EXIT_FAILURE);
	}
	printf("[debug] : parsing command\n");
	strcpy(cmd, "{\"command\": \"broadcastTransactions\", \"trytes\": ");

	printf("[debug] : parsing trytes\n");

	ptr = strchr(broadcastTx, ']');
	if(ptr == NULL)
	{
		return 1;
	}

	ptr[1] = '\0';

	ptr = strchr(broadcastTx, '[');
	if(ptr == NULL)
	{
		return 1;
	}

	printf("[info] : concatenate\n");
	strcat(cmd, ptr);
	strcat(cmd,"}");

	printf("[info] : broadcast trytes : %s\n",cmd);

	request(NODE_URL, NODE_PORT, cmd, resp);

	free(cmd);
	return 0;
}
