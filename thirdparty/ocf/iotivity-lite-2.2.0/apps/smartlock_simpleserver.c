/*
   -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   Copyright 2017-2021 Open Connectivity Foundation
   -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/

/* Application Design
 *
 * support functions:
 * app_init
 *  initializes the oic/p and oic/d values.
 * register_resources
 *  function that registers all endpoints, e.g. sets the RETRIEVE/UPDATE handlers for each end point
 *
 * main 
 *  starts the stack, with the registered resources.
 *
 * Each resource has:
 *  global property variables (per resource path) for:
 *    the property name
 *       naming convention: g_<path>_RESOURCE_PROPERTY_NAME_<propertyname>
 *    the actual value of the property, which is typed from the json data type
 *      naming convention: g_<path>_<propertyname>
 *  global resource variables (per path) for:
 *    the path in a variable:
 *      naming convention: g_<path>_RESOURCE_ENDPOINT
 *
 *  handlers for the implemented methods (get/post)
 *   get_<path>
 *     function that is being called when a RETRIEVE is called on <path>
 *     set the global variables in the output
 *   post_<path>
 *     function that is being called when a UPDATE is called on <path>
 *     checks the input data
 *     if input data is correct
 *       updates the global variables
 *
 */
/*
tool_version          : 20200103
input_file            : ../smartlock/out_codegeneration_merged.swagger.json
version of input_file :  
title of input_file   : server_smartlock
*/

#include "oc_api.h"
#include "oc_core_res.h"
#include "port/oc_clock.h"
#include <signal.h>

#ifdef OC_CLOUD
#include "oc_cloud.h"
#endif
#if defined(OC_IDD_API)
#include "oc_introspection.h"
#endif

#ifdef __linux__
/* linux specific code */
#include <pthread.h>
#ifndef NO_MAIN
static pthread_mutex_t mutex;
static pthread_cond_t cv;
static struct timespec ts;
#endif /* NO_MAIN */
#endif

#ifdef WIN32
/* windows specific code */
#include <windows.h>
static CONDITION_VARIABLE cv;   /* event loop variable */
static CRITICAL_SECTION cs;     /* event loop variable */
#endif

#ifdef USE_GUI
#include "smartlock_ui.h"
#endif

#define btoa(x) ((x)?"true":"false")

#define MAX_STRING 30           /* max size of the strings. */
#define MAX_PAYLOAD_STRING 65   /* max size strings in the payload */
#define MAX_ARRAY 10            /* max size of the array */
/* Note: Magic numbers are derived from the resource definition, either from the example or the definition.*/

volatile int quit = 0;          /* stop variable, used by handle_signal */
static const size_t DEVICE = 0; /* default device index */



/* global property variables for path: "/lock_status" */
static char *g_lock_status_RESOURCE_PROPERTY_NAME_lockState = "lockState"; /* the name for the attribute */
char g_lock_status_lockState[ MAX_PAYLOAD_STRING ] = "Unlocked"; /* current value of property "lockState" The state of the lock. *//* registration data variables for the resources */

/* global resource variables for path: /lock_status */
static char *g_lock_status_RESOURCE_ENDPOINT = "/lock_status"; /* used path for this resource */
static char *g_lock_status_RESOURCE_TYPE[MAX_STRING] = {"oic.r.lock.status"}; /* rt value (as an array) */
int g_lock_status_nr_resource_types = 1;

/* global property variables for path: "/lock_password" */
static char *g_password_RESOURCE_PROPERTY_NAME_password = "password"; /* the name for the attribute */
char g_password_password[ MAX_PAYLOAD_STRING ] = "123456"; /* current value of property "password" The state of the lock. *//* registration data variables for the resources */

/* global resource variables for path: /lock_password */
static char *g_password_RESOURCE_ENDPOINT = "/lock_password"; /* used path for this resource */
static char *g_password_RESOURCE_TYPE[MAX_STRING] = {"oic.r.lock.password"}; /* rt value (as an array) */
int g_password_nr_resource_types = 1;

/**
 * function to set up the device.
 *
 */
int
app_init(void)
{
    int ret = oc_init_platform("ocf", NULL, NULL);
    /* the settings determine the appearance of the device on the network
       can be ocf.2.2.0 (or even higher)
       supplied values are for ocf.2.2.0 */
    ret |= oc_add_device("/oic/d", "oic.d.smartlock", "server_smartlock", 
        "ocf.2.2.0", /* icv value */
        "ocf.res.1.3.0, ocf.sh.1.3.0",  /* dmv value */
        NULL, NULL);

#if defined(OC_IDD_API)
    FILE *fp;
    uint8_t *buffer;
    size_t buffer_size;
    const char introspection_error[] =
        "\tERROR Could not read 'server_introspection.cbor'\n"
        "\tIntrospection data not set.\n";
    fp = fopen("./server_introspection.cbor", "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        buffer_size = ftell(fp);
        rewind(fp);

        buffer = (uint8_t *)oc_alloc(buffer_size * sizeof(uint8_t));
        size_t fread_ret = fread(buffer, buffer_size, 1, fp);
        fclose(fp);

        if (fread_ret == 1) {
            oc_set_introspection_data(0, buffer, buffer_size);
            PRINT("\tIntrospection data set 'server_introspection.cbor': %d [bytes]\n", (int)buffer_size);
        } else {
            PRINT("%s", introspection_error);
        }
        oc_free(buffer);
    } else {
        PRINT("%s", introspection_error);
    }
#else
    PRINT("\t introspection via header file\n");
#endif
    return ret;
}

/**
 * helper function to check if the POST input document contains 
 * the common readOnly properties or the resouce readOnly properties
 * @param name the name of the property
 * @return the error_status, e.g. if error_status is true, then the input document contains something illegal
 */
static bool
check_on_readonly_common_resource_properties(oc_string_t name, bool error_state)
{
    if (strcmp ( oc_string(name), "n") == 0) {
        error_state = true;
        PRINT ("   property \"n\" is ReadOnly \n");
    }else if (strcmp ( oc_string(name), "if") == 0) {
        error_state = true;
        PRINT ("   property \"if\" is ReadOnly \n");
    } else if (strcmp ( oc_string(name), "rt") == 0) {
        error_state = true;
        PRINT ("   property \"rt\" is ReadOnly \n");
    } else if (strcmp ( oc_string(name), "id") == 0) {
        error_state = true;
        PRINT ("   property \"id\" is ReadOnly \n");
    } else if (strcmp ( oc_string(name), "id") == 0) {
        error_state = true;
        PRINT ("   property \"id\" is ReadOnly \n");
    } 
    return error_state;
}


/**
 * get method for "/lock_status" resource.
 * function is called to intialize the return values of the GET method.
 * initialisation of the returned values are done from the global property values.
 * Resource Description:
 * The Resource describing a lock.
 * The Property "lockState" is a string. The value 'Locked' indicates that the door is Locked.
 * The value 'Unlocked' indicates that the door is Unlocked.
 *
 * @param request the request representation.
 * @param interfaces the interface used for this call
 * @param user_data the user data.
 */
static void
get_lock_status(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)user_data;  /* variable not used */
    /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
       the call to the HW needs to fill in the global variable before it returns to this function here.
       alternative is to have a callback from the hardware that sets the global variables.

       The implementation always return everything that belongs to the resource.
       this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
    bool error_state = false;
    int oc_status_code = OC_STATUS_OK;


    PRINT("-- Begin get_lock_status: interface %d\n", interfaces);
    oc_rep_start_root_object();
    switch (interfaces) {
    case OC_IF_BASELINE:
        PRINT("   Adding Baseline info\n" );
        oc_process_baseline_interface(request->resource);

        /* property (string) 'lockState' */
        oc_rep_set_text_string(root, lockState, g_lock_status_lockState);
        PRINT("   %s : %s\n", g_lock_status_RESOURCE_PROPERTY_NAME_lockState, g_lock_status_lockState);
        break;
    case OC_IF_A:

        /* property (string) 'lockState' */
        oc_rep_set_text_string(root, lockState, g_lock_status_lockState);
        PRINT("   %s : %s\n", g_lock_status_RESOURCE_PROPERTY_NAME_lockState, g_lock_status_lockState);
        break;
    default:
        break;
    }
    oc_rep_end_root_object();
    if (error_state == false) {
        oc_send_response(request, oc_status_code);
    }
    else {
        oc_send_response(request, OC_STATUS_BAD_OPTION);
    }
    PRINT("-- End get_lock_status\n");
}

/**
 * get method for "/lock_password" resource.
 * function is called to intialize the return values of the GET method.
 * initialisation of the returned values are done from the global property values.
 * Resource Description:
 * The Resource describing a lock.
 * The Property "password" is a string. The value 'Locked' indicates that the door is Locked.
 * The value 'Unlocked' indicates that the door is Unlocked.
 *
 * @param request the request representation.
 * @param interfaces the interface used for this call
 * @param user_data the user data.
 */
static void
get_password(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)user_data;  /* variable not used */
    /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
       the call to the HW needs to fill in the global variable before it returns to this function here.
       alternative is to have a callback from the hardware that sets the global variables.

       The implementation always return everything that belongs to the resource.
       this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
    bool error_state = false;
    int oc_status_code = OC_STATUS_OK;


    PRINT("-- Begin get_password: interface %d\n", interfaces);
    oc_rep_start_root_object();
    switch (interfaces) {
    case OC_IF_BASELINE:
        PRINT("   Adding Baseline info\n" );
        oc_process_baseline_interface(request->resource);

        /* property (string) 'password' */
        oc_rep_set_text_string(root, password, g_password_password);
        PRINT("   %s : %s\n", g_password_RESOURCE_PROPERTY_NAME_password, g_password_password);
        break;
    case OC_IF_A:

        /* property (string) 'password' */
        oc_rep_set_text_string(root, password, g_password_password);
        PRINT("   %s : %s\n", g_password_RESOURCE_PROPERTY_NAME_password, g_password_password);
        break;
    default:
        break;
    }
    oc_rep_end_root_object();
    if (error_state == false) {
        oc_send_response(request, oc_status_code);
    }
    else {
        oc_send_response(request, OC_STATUS_BAD_OPTION);
    }
    PRINT("-- End get_password\n");
}

/**
 * post method for "/lock_status" resource.
 * The function has as input the request body, which are the input values of the POST method.
 * The input values (as a set) are checked if all supplied values are correct.
 * If the input values are correct, they will be assigned to the global  property values.
 * Resource Description:
 * Sets the current lock state.
 *
 * @param request the request representation.
 * @param interfaces the used interfaces during the request.
 * @param user_data the supplied user data.
 */
static void
post_lock_status(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)interfaces;
    (void)user_data;
    bool error_state = false;
    PRINT("-- Begin post_lock_status:\n");
    oc_rep_t *rep = request->request_payload;

    /* loop over the request document for each required input field to check if all required input fields are present */
    bool var_in_request= false; 
    rep = request->request_payload;
    while (rep != NULL) {
        if (strcmp ( oc_string(rep->name), g_lock_status_RESOURCE_PROPERTY_NAME_lockState) == 0) {
            var_in_request = true;
        }
        rep = rep->next;
    }
    if ( var_in_request == false) 
    { 
        error_state = true;
        PRINT (" required property: 'lockState' not in request\n");
    }
    /* loop over the request document to check if all inputs are ok */
    rep = request->request_payload;
    while (rep != NULL) {
        PRINT("key: (check) %s \n", oc_string(rep->name));

        error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
        if (strcmp ( oc_string(rep->name), g_lock_status_RESOURCE_PROPERTY_NAME_lockState) == 0) {
            /* property "lockState" of type string exist in payload */
            if (rep->type != OC_REP_STRING) {
                error_state = true;
                PRINT ("   property 'lockState' is not of type string %d \n", rep->type);
            }
            if (strlen(oc_string(rep->value.string)) >=  (MAX_PAYLOAD_STRING-1) )
            {
                error_state = true;
                PRINT ("   property 'lockState' is too long %d expected: MAX_PAYLOAD_STRING-1 \n", (int)strlen(oc_string(rep->value.string)));
            }
        }rep = rep->next;
    }
    /* if the input is ok, then process the input document and assign the global variables */
    if (error_state == false)
    {
        switch (interfaces) {
        default: {
                     /* loop over all the properties in the input document */
                     oc_rep_t *rep = request->request_payload;
                     while (rep != NULL) {
                         PRINT("key: (assign) %s \n", oc_string(rep->name));
                         /* no error: assign the variables */

                         if (strcmp ( oc_string(rep->name), g_lock_status_RESOURCE_PROPERTY_NAME_lockState) == 0) {
                             /* assign "lockState" */
                             PRINT ("  property 'lockState' : %s\n", oc_string(rep->value.string));
                             strncpy(g_lock_status_lockState, oc_string(rep->value.string), MAX_PAYLOAD_STRING-1);
                             oc_storage_write("lockState", (uint8_t *)g_lock_status_lockState, strlen(g_lock_status_lockState));
#ifdef USE_GUI
                             ui_set_lock_status(g_lock_status_lockState);
#endif
                         }
                         rep = rep->next;
                     }
                     /* set the response */
                     PRINT("Set response \n");
                     oc_rep_start_root_object();
                     /*oc_process_baseline_interface(request->resource); */
                     PRINT("   %s : %s\n", g_lock_status_RESOURCE_PROPERTY_NAME_lockState, g_lock_status_lockState);
                     oc_rep_set_text_string(root, lockState, g_lock_status_lockState);

                     oc_rep_end_root_object();
                     /* TODO: ACTUATOR add here the code to talk to the HW if one implements an actuator.
                        one can use the global variables as input to those calls
                        the global values have been updated already with the data from the request */
                     oc_send_response(request, OC_STATUS_CHANGED);
                 }
        }
    }
    else
    {
        PRINT("  Returning Error \n");
        /* TODO: add error response, if any */
        //oc_send_response(request, OC_STATUS_NOT_MODIFIED);
        oc_send_response(request, OC_STATUS_BAD_REQUEST);
    }
    PRINT("-- End post_lock_status\n");
}


/**
 * post method for "/lock_password" resource.
 * The function has as input the request body, which are the input values of the POST method.
 * The input values (as a set) are checked if all supplied values are correct.
 * If the input values are correct, they will be assigned to the global  property values.
 * Resource Description:
 * Sets the current lock state.
 *
 * @param request the request representation.
 * @param interfaces the used interfaces during the request.
 * @param user_data the supplied user data.
 */
static void
post_password(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)interfaces;
    (void)user_data;
    bool error_state = false;
    PRINT("-- Begin post_password:\n");
    oc_rep_t *rep = request->request_payload;

    /* loop over the request document for each required input field to check if all required input fields are present */
    bool var_in_request= false; 
    rep = request->request_payload;
    while (rep != NULL) {
        if (strcmp ( oc_string(rep->name), g_password_RESOURCE_PROPERTY_NAME_password) == 0) {
            var_in_request = true;
        }
        rep = rep->next;
    }
    if ( var_in_request == false) 
    { 
        error_state = true;
        PRINT (" required property: 'password' not in request\n");
    }
    /* loop over the request document to check if all inputs are ok */
    rep = request->request_payload;
    while (rep != NULL) {
        PRINT("key: (check) %s \n", oc_string(rep->name));

        error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
        if (strcmp ( oc_string(rep->name), g_password_RESOURCE_PROPERTY_NAME_password) == 0) {
            /* property "password" of type string exist in payload */
            if (rep->type != OC_REP_STRING) {
                error_state = true;
                PRINT ("   property 'password' is not of type string %d \n", rep->type);
            }
            if (strlen(oc_string(rep->value.string)) >=  (MAX_PAYLOAD_STRING-1) )
            {
                error_state = true;
                PRINT ("   property 'password' is too long %d expected: MAX_PAYLOAD_STRING-1 \n", (int)strlen(oc_string(rep->value.string)));
            }
        }rep = rep->next;
    }
    /* if the input is ok, then process the input document and assign the global variables */
    if (error_state == false)
    {
        switch (interfaces) {
        default: {
                     /* loop over all the properties in the input document */
                     oc_rep_t *rep = request->request_payload;
                     while (rep != NULL) {
                         PRINT("key: (assign) %s \n", oc_string(rep->name));
                         /* no error: assign the variables */

                         if (strcmp ( oc_string(rep->name), g_password_RESOURCE_PROPERTY_NAME_password) == 0) {
                             /* assign "password" */
                             PRINT ("  property 'password' : %s\n", oc_string(rep->value.string));
                             strncpy(g_password_password, oc_string(rep->value.string), MAX_PAYLOAD_STRING-1);
                             oc_storage_write("password", (uint8_t *)g_password_password, strlen(g_password_password));
#ifdef USE_GUI
                             ui_set_lock_passwd(g_password_password);
#endif
                         }
                         rep = rep->next;
                     }
                     /* set the response */
                     PRINT("Set response \n");
                     oc_rep_start_root_object();
                     /*oc_process_baseline_interface(request->resource); */
                     PRINT("   %s : %s\n", g_password_RESOURCE_PROPERTY_NAME_password, g_password_password);
                     oc_rep_set_text_string(root, password, g_password_password);

                     oc_rep_end_root_object();
                     /* TODO: ACTUATOR add here the code to talk to the HW if one implements an actuator.
                        one can use the global variables as input to those calls
                        the global values have been updated already with the data from the request */
                     oc_send_response(request, OC_STATUS_CHANGED);
                 }
        }
    }
    else
    {
        PRINT("  Returning Error \n");
        /* TODO: add error response, if any */
        //oc_send_response(request, OC_STATUS_NOT_MODIFIED);
        oc_send_response(request, OC_STATUS_BAD_REQUEST);
    }
    PRINT("-- End post_password\n");
}



/**
 * register all the resources to the stack
 * this function registers all application level resources:
 * - each resource path is bind to a specific function for the supported methods (GET, POST, PUT)
 * - each resource is 
 *   - secure
 *   - observable
 *   - discoverable 
 *   - used interfaces, including the default interface.
 *     default interface is the first of the list of interfaces as specified in the input file
 */
void
register_resources(void)
{

    PRINT("Register Resource with local path \"/lock_status\"\n");
    oc_resource_t *res_lock_status = oc_new_resource(NULL, g_lock_status_RESOURCE_ENDPOINT, g_lock_status_nr_resource_types, 0);
    PRINT("     number of Resource Types: %d\n", g_lock_status_nr_resource_types);
    for( int a = 0; a < g_lock_status_nr_resource_types; a++ ) {
        PRINT("     Resource Type: \"%s\"\n", g_lock_status_RESOURCE_TYPE[a]);
        oc_resource_bind_resource_type(res_lock_status,g_lock_status_RESOURCE_TYPE[a]);
    }

    oc_resource_bind_resource_interface(res_lock_status,  OC_IF_A); /* oic.if.a */
    oc_resource_bind_resource_interface(res_lock_status,  OC_IF_BASELINE); /* oic.if.baseline */
    oc_resource_set_default_interface(res_lock_status,  OC_IF_A);  
    PRINT("     Default OCF Interface: 'oic.if.a'\n"); 
    oc_resource_set_discoverable(res_lock_status, true);
    /* periodic observable
       to be used when one wants to send an event per time slice
       period is 1 second */
    oc_resource_set_periodic_observable(res_lock_status, 1);
    /* set observable
       events are send when oc_notify_observers(oc_resource_t *resource) is called.
       this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
    /*oc_resource_set_observable(res_lock_status, true); */

    oc_resource_set_request_handler(res_lock_status, OC_GET, get_lock_status, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_lock_status);
#endif 
    oc_resource_set_request_handler(res_lock_status, OC_POST, post_lock_status, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_lock_status);
#endif
    oc_add_resource(res_lock_status);

    PRINT("Register Resource with local path \"/lock_password\"\n");
    oc_resource_t *res_password = oc_new_resource(NULL, g_password_RESOURCE_ENDPOINT, g_password_nr_resource_types, 0);
    PRINT("     number of Resource Types: %d\n", g_password_nr_resource_types);
    for( int a = 0; a < g_password_nr_resource_types; a++ ) {
        PRINT("     Resource Type: \"%s\"\n", g_password_RESOURCE_TYPE[a]);
        oc_resource_bind_resource_type(res_password,g_password_RESOURCE_TYPE[a]);
    }

    oc_resource_bind_resource_interface(res_password,  OC_IF_A); /* oic.if.a */
    oc_resource_bind_resource_interface(res_password,  OC_IF_BASELINE); /* oic.if.baseline */
    oc_resource_set_default_interface(res_password,  OC_IF_A);  
    PRINT("     Default OCF Interface: 'oic.if.a'\n"); 
    oc_resource_set_discoverable(res_password, true);
    /* periodic observable
       to be used when one wants to send an event per time slice
       period is 1 second */
    oc_resource_set_periodic_observable(res_password, 1);
    /* set observable
       events are send when oc_notify_observers(oc_resource_t *resource) is called.
       this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
    /*oc_resource_set_observable(res_password, true); */

    oc_resource_set_request_handler(res_password, OC_GET, get_password, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_password);
#endif 
    oc_resource_set_request_handler(res_password, OC_POST, post_password, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_password);
#endif
    oc_add_resource(res_password);

    /* disable observe for oic/d */
    oc_resource_t* device_resource = oc_core_get_resource_by_index(OCF_D, DEVICE);
    oc_resource_set_observable(device_resource, false);
    /* disable observe for oic/p */
    oc_resource_t* platform_resource = oc_core_get_resource_by_index(OCF_P, DEVICE);
    oc_resource_set_observable(platform_resource, false);
}

#ifdef OC_SECURITY
#ifdef OC_SECURITY_PIN
void
random_pin_cb(const unsigned char *pin, size_t pin_len, void *data)
{
    (void)data;
    PRINT("\n====================\n");
    PRINT("Random PIN: %.*s\n", (int)pin_len, pin);
    PRINT("====================\n");
}
#endif /* OC_SECURITY_PIN */
#endif /* OC_SECURITY */

void
factory_presets_cb(size_t device, void *data)
{
    (void)device;
    (void)data;
#if defined(OC_SECURITY) && defined(OC_PKI)
    /* code to include an pki certificate and root trust anchor */
#include "oc_pki.h"
#include "pki_certs.h"
    int credid =
        oc_pki_add_mfg_cert(0, (const unsigned char *)my_cert, strlen(my_cert), (const unsigned char *)my_key, strlen(my_key));
    if (credid < 0) {
        PRINT("ERROR installing PKI certificate\n");
    } else {
        PRINT("Successfully installed PKI certificate\n");
    }

    if (oc_pki_add_mfg_intermediate_cert(0, credid, (const unsigned char *)int_ca, strlen(int_ca)) < 0) {
        PRINT("ERROR installing intermediate CA certificate\n");
    } else {
        PRINT("Successfully installed intermediate CA certificate\n");
    }

    if (oc_pki_add_mfg_trust_anchor(0, (const unsigned char *)root_ca, strlen(root_ca)) < 0) {
        PRINT("ERROR installing root certificate\n");
    } else {
        PRINT("Successfully installed root certificate\n");
    }

    oc_pki_set_security_profile(0, OC_SP_BLACK, OC_SP_BLACK, credid);
#else
    PRINT("No PKI certificates installed\n");
#endif /* OC_SECURITY && OC_PKI */
}


/**
 * intializes the global variables
 * registers and starts the handler

*/
void
initialize_variables(void)
{
    oc_storage_read("lockState", (uint8_t *)g_lock_status_lockState, MAX_PAYLOAD_STRING);
    oc_storage_read("password", (uint8_t *)g_password_password, MAX_PAYLOAD_STRING);

    /* set the flag for NO oic/con resource. */
    oc_set_con_res_announced(false);

}

#ifndef NO_MAIN

#ifdef WIN32
/**
 * signal the event loop (windows version)
 * wakes up the main function to handle the next callback
 */
static void
signal_event_loop(void)
{
    WakeConditionVariable(&cv);
}
#endif /* WIN32 */

#ifdef __linux__
/**
 * signal the event loop (Linux)
 * wakes up the main function to handle the next callback
 */
static void
signal_event_loop(void)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cv);
    pthread_mutex_unlock(&mutex);
}
#endif /* __linux__ */

/**
 * handle Ctrl-C
 * @param signal the captured signal
 */
void
handle_signal(int signal)
{
    (void)signal;
    signal_event_loop();
    quit = 1;
}

#ifdef OC_CLOUD
/**
 * cloud status handler.
 * handler to print out the status of the cloud connection
 */
static void
cloud_status_handler(oc_cloud_context_t *ctx, oc_cloud_status_t status,
    void *data)
{
    (void)data;
    PRINT("\nCloud Manager Status:\n");
    if (status & OC_CLOUD_REGISTERED) {
        PRINT("\t\t-Registered\n");
    }
    if (status & OC_CLOUD_TOKEN_EXPIRY) {
        PRINT("\t\t-Token Expiry: ");
        if (ctx) {
            PRINT("%d\n", oc_cloud_get_token_expiry(ctx));
        } else {
            PRINT("\n");
        }
    }
    if (status & OC_CLOUD_FAILURE) {
        PRINT("\t\t-Failure\n");
    }
    if (status & OC_CLOUD_LOGGED_IN) {
        PRINT("\t\t-Logged In\n");
    }
    if (status & OC_CLOUD_LOGGED_OUT) {
        PRINT("\t\t-Logged Out\n");
    }
    if (status & OC_CLOUD_DEREGISTERED) {
        PRINT("\t\t-DeRegistered\n");
    }
    if (status & OC_CLOUD_REFRESHED_TOKEN) {
        PRINT("\t\t-Refreshed Token\n");
    }
}
#endif // OC_CLOUD

/**
 * main application.
 * intializes the global variables
 * registers and starts the handler
 * handles (in a loop) the next event.
 * shuts down the stack
 */
int
main(void)
{
    int init;
    oc_clock_time_t next_event;

#ifdef WIN32
    /* windows specific */
    InitializeCriticalSection(&cs);
    InitializeConditionVariable(&cv);
    /* install Ctrl-C */
    signal(SIGINT, handle_signal);
#endif
#ifdef __linux__
    /* linux specific */
    struct sigaction sa;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    sa.sa_handler = handle_signal;
    /* install Ctrl-C */
    sigaction(SIGINT, &sa, NULL);
#endif

    PRINT("Used input file : \"../smartlock/out_codegeneration_merged.swagger.json\"\n");
    PRINT("OCF Server name : \"server_smartlock\"\n");

    /*
       The storage folder depends on the build system
       for Windows the projects simpleserver and cloud_server are overwritten, hence the folders should be the same as those targets.
       for Linux (as default) the folder is created in the makefile, with $target as name with _cred as post fix.
       */
#ifdef OC_SECURITY
    PRINT("Intialize Secure Resources\n");
#ifdef WIN32
#ifdef OC_CLOUD
    PRINT("\tstorage at './cloudserver_creds' \n");
    oc_storage_config("./cloudserver_creds");
#else
    PRINT("\tstorage at './simpleserver_creds' \n");
    oc_storage_config("./simpleserver_creds/");
#endif
#else
    PRINT("\tstorage at './device_builder_server_creds' \n");
    oc_storage_config("./device_builder_server_creds");
#endif

    /*intialize the variables */
    initialize_variables();

#endif /* OC_SECURITY */

    /* initializes the handlers structure */
    static const oc_handler_t handler = {.init = app_init,
        .signal_event_loop = signal_event_loop,
        .register_resources = register_resources
#ifdef OC_CLIENT
            ,
        .requests_entry = 0 
#endif
    };
#ifdef OC_SECURITY
#ifdef OC_SECURITY_PIN
    /* please enable OC_SECURITY_PIN
       - have display capabilities to display the PIN value
       - server require to implement RANDOM PIN (oic.sec.doxm.rdp) onboarding mechanism
       */
    oc_set_random_pin_callback(random_pin_cb, NULL);
#endif /* OC_SECURITY_PIN */
#endif /* OC_SECURITY */

    oc_set_factory_presets_cb(factory_presets_cb, NULL);

    /* start the stack */
    init = oc_main_init(&handler);

    if (init < 0) {
        PRINT("oc_main_init failed %d, exiting.\n", init);
        return init;
    }

#ifdef OC_CLOUD
    /* get the cloud context and start the cloud */
    PRINT("Start Cloud Manager\n");
    oc_cloud_context_t *ctx = oc_cloud_get_context(0);
    if (ctx) {
        oc_cloud_manager_start(ctx, cloud_status_handler, NULL);
    }
#endif 

    PRINT("OCF server \"server_smartlock\" running, waiting on incoming connections.\n");

#ifdef WIN32
    /* windows specific loop */
    while (quit != 1) {
        next_event = oc_main_poll();
        if (next_event == 0) {
            SleepConditionVariableCS(&cv, &cs, INFINITE);
        } else {
            oc_clock_time_t now = oc_clock_time();
            if (now < next_event) {
                SleepConditionVariableCS(&cv, &cs,
                    (DWORD)((next_event-now) * 1000 / OC_CLOCK_SECOND));
            }
        }
    }
#endif

#ifdef __linux__
    /* linux specific loop */
    while (quit != 1) {
        next_event = oc_main_poll();
        pthread_mutex_lock(&mutex);
        if (next_event == 0) {
            pthread_cond_wait(&cv, &mutex);
        } else {
            ts.tv_sec = (next_event / OC_CLOCK_SECOND);
            ts.tv_nsec = (next_event % OC_CLOCK_SECOND) * 1.e09 / OC_CLOCK_SECOND;
            pthread_cond_timedwait(&cv, &mutex, &ts);
        }
        pthread_mutex_unlock(&mutex);
    }
#endif

    /* shut down the stack */
#ifdef OC_CLOUD
    PRINT("Stop Cloud Manager\n");
    oc_cloud_manager_stop(ctx);
#endif
    oc_main_shutdown();
    return 0;
}
#endif /* NO_MAIN */
