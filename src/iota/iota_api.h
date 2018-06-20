/*
 * iota_api.h
 *
 *  Created on: Jun 13, 2018
 *      Author: max
 */

#ifndef SRC_IOTA_IOTA_API_H_
#define SRC_IOTA_IOTA_API_H_

#include <stdint.h>

//#define NODE_URL	"http://51.255.203.228"//:14265
//#define NODE_URL	"https://iotanode2.jlld.at"//:443
//#define NODE_URL	"http://nodes.iota.fm"//:80
//#define NODE_URL	"http://node1.iota.cm"//:14265
#define NODE_URL	"http://iotahosting.org"//:14265

#define NODE_PORT	14265

typedef struct ADDRESS_OBJECT{
	uint8_t pubKey[81];
	uint32_t value;
	uint8_t spentState;
	uint8_t index;
}addressObj_t;

void iotaApi_getAddress(uint8_t* seed_chars, addressObj_t** addr, uint8_t lower_index, uint8_t upper_index, uint8_t security);
void iotaApi_getBalance(addressObj_t* addr);
void iotaApi_wereAddressesSpentFrom(addressObj_t* addr);
int8_t iotaApi_getTransactionsToApprove(uint8_t* trunkTx, uint8_t* branchTx);
int8_t iotaApi_attachToTangle(uint8_t* txChars, uint8_t* trunkTx, uint8_t* branchTx, const uint8_t nTx, uint8_t* trytes);
int8_t iotaApi_broadcastTransactions(uint8_t* broadcastTx, uint8_t nTx);

#endif /* SRC_IOTA_IOTA_API_H_ */
