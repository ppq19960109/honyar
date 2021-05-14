/*
 * ctrlSmartLock.h
 *
 *  Created on: Nov 18, 2020
 *      Author: jerry
 */

#ifndef SRC_CTRLSMARTLOCK_H_
#define SRC_CTRLSMARTLOCK_H_

#include "hyprotocol.h"

#define ADD_ONE_TIME_PASSWORD  "AddOTP"
#define CONFIGURE_KEY_TIME  "KeyTimeLimitation"
#define GET_KEY_TIME		"GetKeyTimeLimitation"
#define REMOTE_UNLOCK		"RemoteUnlock"
#define REMOVE_KEY_TIME		"RemoveKeyTimeLimitation"


#define EVENT_REMOTE_UNLOCK_NOTIFICATION "RemoteUnlockNotification"
#define EVENT_ADD_OPT_NOTIFICATION	"AddOPTNotification"
#define EVENT_REMOVE_KEY_TIME_NOTIFICATION	"RemoveKeyTimeLimitationNotification"
#define EVENT_KEY_TIME_NOTIFICATION	"KeyTimeLimitationNotification"


int handleCtrlLockCmd(KeyValue *head);

#endif /* SRC_CTRLSMARTLOCK_H_ */
