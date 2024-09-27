#include <stdio.h>
#include <string.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"
#include "lib/logging_mosq.h"
//#include "process.hpp"

#include "my_cpp_module.h"

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t *mosq_pid = NULL;

static int callback_message(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message *ed = (struct mosquitto_evt_message*)event_data;
	char *new_payload;
	uint32_t new_payloadlen;


	UNUSED(event);
	UNUSED(userdata);
    
	/* This simply adds "hello " to the front of every payload. You can of
	 * course do much more complicated message processing if needed. */

	/* Calculate the length of our new payload */
	new_payloadlen = ed->payloadlen + (uint32_t)strlen("hello ")+1;

	/* Allocate some memory - use
	 * mosquitto_calloc/mosquitto_malloc/mosquitto_strdup when allocating, to
	 * allow the broker to track memory usage */
	new_payload = (char*)mosquitto_calloc(1, new_payloadlen);
	if(new_payload == NULL){
		return MOSQ_ERR_NOMEM;
	}

	/* Print "hello " to the payload */
	snprintf(new_payload, new_payloadlen, "hello ");
	memcpy(new_payload+(uint32_t)strlen("hello "), ed->payload, ed->payloadlen);

	/* Assign the new payload and payloadlen to the event data structure. You
	 * must *not* free the original payload, it will be handled by the
	 * broker. */
	ed->payload = new_payload;
	ed->payloadlen = new_payloadlen;
	
	//TEST -> CHANGE ALSO PUBLISH TOPIC OTHER THAN PAYLOAD
	uint32_t new_topiclen = (uint32_t)strlen(ed->topic) + (uint32_t)strlen("test")+1;
	char *new_topic = (char*)mosquitto_calloc(1, new_topiclen);
	if(new_topic == NULL){
		return MOSQ_ERR_NOMEM;
	}
	snprintf(new_topic,new_topiclen, "test");
	memcpy(new_topic+(uint32_t)strlen("test"), ed->topic, (uint32_t)strlen(ed->topic));

	ed->topic=new_topic;

	MyClass* obj = MyClass_new();
    MyClass_doSomething(obj);
    MyClass_delete(obj);


    //C++ FUNCTION PROCESSING THE MESSAGES 
	//process_msg(NULL,"other_topic",(int)ed->payloadlen,ed->payload,ed->qos,ed->retain,ed->properties);

	return MOSQ_ERR_SUCCESS;
}


int mosquitto_plugin_version(int supported_version_count, const int *supported_versions)
{
	int i;

	for(i=0; i<supported_version_count; i++){
		if(supported_versions[i] == 5){
			return 5;
		}
	}
	return -1;
}

int mosquitto_plugin_init(mosquitto_plugin_id_t *identifier, void **user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);

	mosq_pid = identifier;

	//WILL LOAD YAML DOCUMENTS? IN GLOBAL STRUCT?
	//TODO load_yaml(); -> broken compatibility between C and C++ class, solve

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, NULL);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);

	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL);
}
