#include <stdio.h>
#include <string.h>

#include "mosquitto_broker.h"
#include "mosquitto_plugin.h"
#include "mosquitto.h"
#include "mqtt_protocol.h"
#include "lib/logging_mosq.h"
//#include "process.hpp"

#include "wrapper.h"

#define UNUSED(A) (void)(A)

static mosquitto_plugin_id_t *mosq_pid = NULL;
Wrapper* wrapper_obj = NULL;

int num_documents = 0;

static int callback_message(int event, void *event_data, void *userdata)
{
	struct mosquitto_evt_message *ed = (struct mosquitto_evt_message*)event_data;

	UNUSED(event);
	UNUSED(userdata);

    wrapper_publish(wrapper_obj,NULL, ed->topic,(int)ed->payloadlen,ed->payload,ed->qos,ed->retain,ed->properties);
	//mosquitto_broker_publish_copy(NULL,ed->topic,3,"ayo",ed->qos,ed->retain,ed->properties)

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

	wrapper_obj = wrapper_new("config.yml");

	return mosquitto_callback_register(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL, NULL);
}

int mosquitto_plugin_cleanup(void *user_data, struct mosquitto_opt *opts, int opt_count)
{
	UNUSED(user_data);
	UNUSED(opts);
	UNUSED(opt_count);

	wrapper_delete(wrapper_obj); //free memory
	
	return mosquitto_callback_unregister(mosq_pid, MOSQ_EVT_MESSAGE, callback_message, NULL);
}
