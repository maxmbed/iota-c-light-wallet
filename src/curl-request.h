/*
 * curl-request.h
 *
 *  Created on: Jun 13, 2018
 *      Author: max
 */

#ifndef SRC_CURL_REQUEST_H_
#define SRC_CURL_REQUEST_H_

#include <stdint.h>

int request(uint8_t* ip, int16_t port, uint8_t* cmd, uint8_t* resp);


#endif /* SRC_CURL_REQUEST_H_ */
