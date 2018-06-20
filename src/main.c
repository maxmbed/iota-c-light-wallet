#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// iota-related stuff
#include "iota/kerl.h"
#include "iota/conversion.h"
#include "iota/addresses.h"
#include "iota/transfers.h"
#include "iota/iota_api.h"


enum{
	dest,
	remain
};

int print_help() {
	printf("Usage c_light_wallet <SEED_81_CHARS> SECURITY INDEX COUNT <DEST_ADDRESS_81 _CHARS> FUND_TO_SPEND\n");
	return 0;
}

void dbg_printLocalTime(uint8_t* msg);
void address(char* seed_chars, int index, int security, char result[81]);

int main(int argc, char *argv[]){

	//Argument input checking
	if (argc != 7) {
		return print_help();
	}

	//Get input argument
	int security = (int)strtol(argv[2], NULL, 10);
	int index = (int)strtol(argv[3], NULL, 10);
	int count = (int)strtol(argv[4], NULL, 10);
	char* seed_chars = argv[1];


	addressObj_t addressInfo; //List of "spendable" address

	char *seed = seed_chars; // 81 ternary characters

	TX_INPUT inputTx[10];
	TX_OUTPUT outputTx[2];

	memcpy(outputTx[dest].address, argv[5], 81);//Assigning destination address
	outputTx[dest].value = strtol(argv[6], NULL, 10);//Assigning iota value

	//List all addresses from index to count
	for(int i=index;i<count;i++)
	{
		address(seed_chars, i, security, addressInfo.pubKey);
		iotaApi_getBalance(&addressInfo);
		iotaApi_wereAddressesSpentFrom(&addressInfo);
		addressInfo.index = i;
		printf("[get address][%d] %.81s : %di spent : %d\n",addressInfo.index, addressInfo.pubKey, addressInfo.value, addressInfo.spentState);
	}

	uint8_t idx = 0;
    uint8_t cumulatedBalance = 0;
    uint8_t numOutputTx = 1;
    for(int i=0;i<count;i++)
    {
        address(seed_chars, i, security, addressInfo.pubKey);
        iotaApi_getBalance(&addressInfo);

        if(addressInfo.value > 0 && addressInfo.value < outputTx[dest].value)
        {
            inputTx[idx].key_index = i; //Store input transaction index
            inputTx[idx].balance = addressInfo.value;

            if(idx == 0)
                continue;

            cumulatedBalance =+ inputTx[idx].balance;
            if( cumulatedBalance >= outputTx[dest].value )
            {
                //We have the necessary fund to make the transaction

                uint64_t remain_iota = inputTx[idx].balance - outputTx[dest].value;

                //Check if we need additional remain addresses
                if(remain_iota > 0)
                {
                    //Find an unspent address where we can send the remain iota
                    for(int i=0;i<count;i++)
                    {
                        address(seed_chars, i, security, addressInfo.pubKey);
                        iotaApi_wereAddressesSpentFrom(&addressInfo);
                        if(addressInfo.spentState == 0)
                        {
                            //We found an unspent address to use
                            memcpy(outputTx[remain].address, addressInfo.pubKey, 81);
                            outputTx[remain].value = remain_iota;
                            numOutputTx = 2;
                        }
                    }
                }
            }
            idx++;
        }
        else if(addressInfo.value >= outputTx[dest].value)
        {
            inputTx[idx].key_index = i;
            inputTx[idx].balance = addressInfo.value;

            uint64_t remain_iota = inputTx[idx].balance - outputTx[dest].value;
            //Check if we need additional remain addresses
            if(remain_iota > 0)
            {
                //Find an unspent address where we can send the remain iota
                for(int i=0;i<count;i++)
                {
                    address(seed_chars, i, security, addressInfo.pubKey);
                    iotaApi_wereAddressesSpentFrom(&addressInfo);
                    if(addressInfo.spentState == 0)
                    {
                        //We found an unspent address
                        memcpy(outputTx[remain].address, addressInfo.pubKey, 81);
                        outputTx[remain].value = remain_iota;
                        numOutputTx = 2;
                        break;
                    }
                }
                break;
            }


        }

	}

	printf("\n\n");
	printf("[info][destination address] : output TX : address[%.81s] - value %ldi\n",outputTx[dest].address, outputTx[dest].value);
	if(outputTx[remain].value != 0)
	{
		printf("[info][remain address] : output TX : address[%.81s] - value %ldi\n",outputTx[remain].address, outputTx[remain].value);
	}

	for(int i=0;i<=idx;i++)
	{
		printf("[info][spent address] : input TX : address[%d] - value %ldi\n",inputTx[i].key_index, -inputTx[i].balance);
	}

	memcpy(outputTx[dest].message, "HELLO9WORLD",strlen("HELLO9WORLD"));
	memcpy(outputTx[dest].tag, "EMBEDDED",strlen("EMBEDDED"));

	printf("\n");

	//Define the transaction chars array. The char trytes will be saved in this array. (base-27 encoded)
	char transaction_chars[10][2673];
	//Get all raw transactions trytes. Will be saved in transaction_chars
	prepare_transfers(seed, 2, outputTx, numOutputTx, inputTx, idx+1, transaction_chars);

	printf("[info] : Bundle transaction :\n");
	uint8_t numOfTx = numOutputTx + (security*(idx+1));
	for(int i=0;i<numOfTx;i++)
	{
		printf("\t[part %d] : %.2673s\n", i, transaction_chars[0]+(i*2673));
	}

	uint8_t trunkTx[81] = " ";
	uint8_t branchTx[81] = " ";
	exit(0);

	if(iotaApi_getTransactionsToApprove(trunkTx, branchTx))
	{
		printf("[error] : get transctions to approve\n");
	}
	else
	{
		printf("[info] : trunk transaction : [%.81s] - branch transaction : [%.81s]\n",trunkTx, branchTx);

		dbg_printLocalTime("POW started");

		uint8_t* trytes = malloc(1024+(numOfTx*2673));
		iotaApi_attachToTangle((uint8_t*)transaction_chars, trunkTx, branchTx, numOfTx, trytes);

		dbg_printLocalTime("POW done");

		printf("[info] : raw trytes response %s\n",trytes);

		printf("[info] : broadcast transaction to Tangle\n");
		iotaApi_broadcastTransactions(trytes, numOfTx);
	}

	return 0;
}



void dbg_printLocalTime(uint8_t* msg)
{
	time_t timer;
	struct tm* t;
	time(&timer);
	t = localtime(&timer);
	printf("[debug] : %s %s",msg, asctime(t));
}


void address(char* seed_chars, int index, int security, char result[81]) {
	
	unsigned char address[81];
	unsigned char seed_bytes[48];
	chars_to_bytes(seed_chars, seed_bytes, 81);
	get_public_addr(seed_bytes, index, security, address);
	
	bytes_to_chars(address, result, 48);
}

void get_address(uint8_t* seed_chars, addressObj_t** addr, uint8_t lower_index, uint8_t upper_index, uint8_t security)
{
	char char_address[81];

	for (int i = lower_index; i < upper_index; i++)
	{
		address(seed_chars, i, security, char_address);
		memcpy(addr[i]->pubKey,char_address, 81);
		printf("[info] : address %d - %s\n",i, addr[i]->pubKey);
	}
}
