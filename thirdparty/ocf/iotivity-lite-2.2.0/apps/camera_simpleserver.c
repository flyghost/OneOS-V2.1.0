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
input_file            : ../camera/out_codegeneration_merged.swagger.json
version of input_file :  
title of input_file   : server_camera
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
#include "camera_ui.h"
#endif

#define btoa(x) ((x)?"true":"false")

#define MAX_STRING 30           /* max size of the strings. */
#define MAX_PAYLOAD_STRING 65   /* max size strings in the payload */
#define MAX_ARRAY 10            /* max size of the array */
/* Note: Magic numbers are derived from the resource definition, either from the example or the definition.*/

volatile int quit = 0;          /* stop variable, used by handle_signal */
static const size_t DEVICE = 0; /* default device index */



/* global property variables for path: "/binaryswitch" */
static char *g_binaryswitch_RESOURCE_PROPERTY_NAME_value = "value"; /* the name for the attribute */
bool g_binaryswitch_value = false; /* current value of property "value" The status of the switch. */
/* global property variables for path: "/media" */
static char *g_media_RESOURCE_PROPERTY_NAME_media = "media"; /* the name for the attribute */

/* array media   *//* array of objects */
struct _media_media_t
{
    char sdp [MAX_ARRAY][MAX_PAYLOAD_STRING];  /* SDP media or attribute line */
    int sdp_array_size; /* current size of the array */
    char url [MAX_PAYLOAD_STRING];  /* The url for the media instance. */

};
struct _media_media_t g_media_media[MAX_ARRAY];
int g_media_media_array_size = 0;

/* registration data variables for the resources */

/* global resource variables for path: /binaryswitch */
static char *g_binaryswitch_RESOURCE_ENDPOINT = "/binaryswitch"; /* used path for this resource */
static char *g_binaryswitch_RESOURCE_TYPE[MAX_STRING] = {"oic.r.switch.binary"}; /* rt value (as an array) */
int g_binaryswitch_nr_resource_types = 1;
/* global resource variables for path: /media */
static char *g_media_RESOURCE_ENDPOINT = "/media"; /* used path for this resource */
static char *g_media_RESOURCE_TYPE[MAX_STRING] = {"oic.r.media"}; /* rt value (as an array) */
int g_media_nr_resource_types = 1;

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
    ret |= oc_add_device("/oic/d", "oic.d.camera", "server_camera", 
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
 * get method for "/binaryswitch" resource.
 * function is called to intialize the return values of the GET method.
 * initialisation of the returned values are done from the global property values.
 * Resource Description:
 * This Resource describes a binary switch (on/off).
 * The Property "value" is a boolean.
 * A value of 'true' means that the switch is on.
 * A value of 'false' means that the switch is off.
 *
 * @param request the request representation.
 * @param interfaces the interface used for this call
 * @param user_data the user data.
 */
static void
get_binaryswitch(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)user_data;  /* variable not used */
    /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
       the call to the HW needs to fill in the global variable before it returns to this function here.
       alternative is to have a callback from the hardware that sets the global variables.

       The implementation always return everything that belongs to the resource.
       this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
    bool error_state = false;
    int oc_status_code = OC_STATUS_OK;


    PRINT("-- Begin get_binaryswitch: interface %d\n", interfaces);
    oc_rep_start_root_object();
    switch (interfaces) {
    case OC_IF_BASELINE:
        PRINT("   Adding Baseline info\n" );
        oc_process_baseline_interface(request->resource);

        /* property (boolean) 'value' */
        oc_rep_set_boolean(root, value, g_binaryswitch_value);
        PRINT("   %s : %s\n", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
        break;
    case OC_IF_A:

        /* property (boolean) 'value' */
        oc_rep_set_boolean(root, value, g_binaryswitch_value);
        PRINT("   %s : %s\n", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
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
    PRINT("-- End get_binaryswitch\n");
}

/**
 * get method for "/media" resource.
 * function is called to intialize the return values of the GET method.
 * initialisation of the returned values are done from the global property values.
 * Resource Description:
 * This Resource specifies the media types that an OCF Server supports.
 * The resource is an array of media elements.  Each element contains:
 *     A URL at which the specified media type can be accessed.
 *     A string array containing the definition of the media using SDP.
 *     Each entry in the sdp array is an SDP line.
 *     Each line shall follow the SDP description syntax as defined in the SDP specification.
 * The SDP specification can be found at http://tools.ietf.org/html/rfc4566.
 *
 * @param request the request representation.
 * @param interfaces the interface used for this call
 * @param user_data the user data.
 */
static void
get_media(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)user_data;  /* variable not used */
    /* TODO: SENSOR add here the code to talk to the HW if one implements a sensor.
       the call to the HW needs to fill in the global variable before it returns to this function here.
       alternative is to have a callback from the hardware that sets the global variables.

       The implementation always return everything that belongs to the resource.
       this implementation is not optimal, but is functionally correct and will pass CTT1.2.2 */
    bool error_state = false;
    int oc_status_code = OC_STATUS_OK;


    PRINT("-- Begin get_media: interface %d\n", interfaces);
    oc_rep_start_root_object();
    switch (interfaces) {
    case OC_IF_BASELINE:
        PRINT("   Adding Baseline info\n" );
        oc_process_baseline_interface(request->resource);

        /* property (array of objects) 'media' */
        PRINT("   Array of objects : '%s'\n", g_media_RESOURCE_PROPERTY_NAME_media);
        oc_rep_set_array(root, media);
        for (int i = 0; i<g_media_media_array_size; i++)
        {
            oc_rep_object_array_begin_item(media);
            /* sdp ['string[]', 'SDP media or attribute line'] */
            oc_rep_open_array(media, sdp);
            PRINT("    array sdp \n");
            oc_rep_add_text_string(sdp, g_media_media[i].sdp[0]);
            PRINT("     array item string sdp[0] : %d %s\n", i, g_media_media[i].sdp[0]);
            oc_rep_close_array(media, sdp);

            /* url ['string', 'The url for the media instance.'] */
            oc_rep_set_text_string(media, url, g_media_media[i].url);
            PRINT("    string url : %d %s\n", i, g_media_media[i].url);
            oc_rep_object_array_end_item(media);
        }
        oc_rep_close_array(root, media);

        break;
    case OC_IF_A:

        /* property (array of objects) 'media' */
        PRINT("   Array of objects : '%s'\n", g_media_RESOURCE_PROPERTY_NAME_media);
        oc_rep_set_array(root, media);
        for (int i = 0; i<g_media_media_array_size; i++)
        {
            oc_rep_object_array_begin_item(media);
            /* sdp ['string[]', 'SDP media or attribute line'] */
            oc_rep_open_array(media, sdp);
            PRINT("    array sdp \n");
            oc_rep_add_text_string(sdp, g_media_media[i].sdp[0]);
            PRINT("     array item string sdp[0] : %d %s\n", i, g_media_media[i].sdp[0]);
            oc_rep_close_array(media, sdp);

            /* url ['string', 'The url for the media instance.'] */
            oc_rep_set_text_string(media, url, g_media_media[i].url);
            PRINT("    string url : %d %s\n", i, g_media_media[i].url);
            oc_rep_object_array_end_item(media);
        }
        oc_rep_close_array(root, media);

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
    PRINT("-- End get_media\n");
}

/**
 * post method for "/binaryswitch" resource.
 * The function has as input the request body, which are the input values of the POST method.
 * The input values (as a set) are checked if all supplied values are correct.
 * If the input values are correct, they will be assigned to the global  property values.
 * Resource Description:

 *
 * @param request the request representation.
 * @param interfaces the used interfaces during the request.
 * @param user_data the supplied user data.
 */
static void
post_binaryswitch(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)interfaces;
    (void)user_data;
    bool error_state = false;
    PRINT("-- Begin post_binaryswitch:\n");
    oc_rep_t *rep = request->request_payload;

    /* loop over the request document for each required input field to check if all required input fields are present */
    bool var_in_request= false; 
    rep = request->request_payload;
    while (rep != NULL) {
        if (strcmp ( oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value) == 0) {
            var_in_request = true;
        }
        rep = rep->next;
    }
    if ( var_in_request == false) 
    { 
        error_state = true;
        PRINT (" required property: 'value' not in request\n");
    }
    /* loop over the request document to check if all inputs are ok */
    rep = request->request_payload;
    while (rep != NULL) {
        PRINT("key: (check) %s \n", oc_string(rep->name));

        error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
        if (strcmp ( oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value) == 0) {
            /* property "value" of type boolean exist in payload */
            if (rep->type != OC_REP_BOOL) {
                error_state = true;
                PRINT ("   property 'value' is not of type bool %d \n", rep->type);
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

                         if (strcmp ( oc_string(rep->name), g_binaryswitch_RESOURCE_PROPERTY_NAME_value)== 0) {
                             /* assign "value" */
                             PRINT ("  property 'value' : %s\n", (char *)btoa(rep->value.boolean));
                             g_binaryswitch_value = rep->value.boolean;
                             char tmp_binaryswitch[MAX_PAYLOAD_STRING];
                             memset(tmp_binaryswitch, 0, MAX_PAYLOAD_STRING);
                             snprintf(tmp_binaryswitch, MAX_PAYLOAD_STRING, "%d", g_binaryswitch_value);
                             oc_storage_write("binaryswitch", (uint8_t *)tmp_binaryswitch, strlen(tmp_binaryswitch));
#ifdef USE_GUI
                             ui_set_binary_switch((int)g_binaryswitch_value);
#endif 
                         }
                         rep = rep->next;
                     }
                     /* set the response */
                     PRINT("Set response \n");
                     oc_rep_start_root_object();
                     /*oc_process_baseline_interface(request->resource); */
                     PRINT("   %s : %s", g_binaryswitch_RESOURCE_PROPERTY_NAME_value, (char *)btoa(g_binaryswitch_value));
                     oc_rep_set_boolean(root, value, g_binaryswitch_value);

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
    PRINT("-- End post_binaryswitch\n");
}

/**
 * post method for "/media" resource.
 * The function has as input the request body, which are the input values of the POST method.
 * The input values (as a set) are checked if all supplied values are correct.
 * If the input values are correct, they will be assigned to the global  property values.
 * Resource Description:
 * This is to change the URL that can be played back by the device. Note that some devices do not have the capability to set the URL to be played back
 *
 * @param request the request representation.
 * @param interfaces the used interfaces during the request.
 * @param user_data the supplied user data.
 */
static void
post_media(oc_request_t *request, oc_interface_mask_t interfaces, void *user_data)
{
    (void)interfaces;
    (void)user_data;
    bool error_state = false;
    PRINT("-- Begin post_media:\n");
    oc_rep_t *rep = request->request_payload;

    /* loop over the request document for each required input field to check if all required input fields are present */
    bool var_in_request= false; 
    rep = request->request_payload;
    while (rep != NULL) {
        if (strcmp ( oc_string(rep->name), g_media_RESOURCE_PROPERTY_NAME_media) == 0) {
            var_in_request = true;
        }
        rep = rep->next;
    }
    if ( var_in_request == false) 
    { 
        error_state = true;
        PRINT (" required property: 'media' not in request\n");
    }
    /* loop over the request document to check if all inputs are ok */
    rep = request->request_payload;
    while (rep != NULL) {
        PRINT("key: (check) %s \n", oc_string(rep->name));

        error_state = check_on_readonly_common_resource_properties(rep->name, error_state);
        if (strcmp ( oc_string(rep->name), g_media_RESOURCE_PROPERTY_NAME_media) == 0) {
            /* property "media" of type array exist in payload */

            size_t array_size=0;

            if ( array_size > MAX_ARRAY)
            {
                error_state = true;
                PRINT ("   property array 'media' is too long: %d expected: MAX_ARRAY \n", (int)array_size);
            }
        }rep = rep->next;
    }
    /* if the input is ok, then process the input document and assign the global variables */
    if (error_state == false)
    {
        switch (interfaces) {
            default:{
                /* loop over all the properties in the input document */
                oc_rep_t *rep = request->request_payload;
                while (rep != NULL) {
                    PRINT("key: (assign) %s \n", oc_string(rep->name));
                    /* no error: assign the variables */
                    if (strcmp ( oc_string(rep->name), g_media_RESOURCE_PROPERTY_NAME_media)== 0) {
                        /* assign "value" */;
                        oc_rep_t *media_rep = rep->value.object_array;
                        while(media_rep != NULL)
                        {
                            oc_rep_t *tmp = media_rep->value.object;
                            while(tmp != NULL)
                            {
                                if(!strcmp(oc_string(tmp->name), "url"))
                                {
                                    char *url = oc_string(tmp->value.string);
                                    strncpy(g_media_media[0].url, url, MAX_PAYLOAD_STRING - 1); 
                                    PRINT ("  property value : %s\n", g_media_media[0].url);
                                    oc_storage_write("url", (uint8_t*)g_media_media[0].url, sizeof(g_media_media[0].url));
#ifdef USE_GUI
                                    ui_set_media(g_media_media[0].url);
#endif
                                }
                                else if(!strcmp(oc_string(tmp->name), "sdp"))
                                {
                                    char *sdp = oc_string(tmp->value.string);
                                    strncpy(g_media_media[0].sdp[0], sdp, MAX_PAYLOAD_STRING - 1);
                                    PRINT ("  property value : %s\n", g_media_media[0].sdp[0]);
                                    oc_storage_write("sdp", (uint8_t*)g_media_media[0].sdp[0], sizeof(g_media_media[0].sdp[0]));
#ifdef USE_GUI
                                    ui_set_sdp(g_media_media[0].sdp[0]);
#endif
                                }
                                tmp = tmp->next;
                            }
                            media_rep = media_rep->next;
                        }
                    }
                    rep = rep->next;
                }
                /* set the response */
                PRINT("Set response \n");
                oc_rep_start_root_object();
                /*oc_process_baseline_interface(request->resource); */

                oc_rep_set_array(root, media);
                for (int i = 0; i < g_media_media_array_size; i ++)
                {
                    oc_rep_object_array_begin_item(media);
                    oc_rep_open_array(media, sdp);
                    oc_rep_add_text_string(sdp, g_media_media[i].sdp[0]);
                    oc_rep_close_array(media, sdp);
                    oc_rep_set_text_string(media, url, g_media_media[i].url);
                    oc_rep_object_array_end_item(media);
                }
                oc_rep_close_array(root, media);

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
    PRINT("-- End post_media\n");
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

    PRINT("Register Resource with local path \"/binaryswitch\"\n");
    oc_resource_t *res_binaryswitch = oc_new_resource(NULL, g_binaryswitch_RESOURCE_ENDPOINT, g_binaryswitch_nr_resource_types, 0);
    PRINT("     number of Resource Types: %d\n", g_binaryswitch_nr_resource_types);
    for( int a = 0; a < g_binaryswitch_nr_resource_types; a++ ) {
        PRINT("     Resource Type: \"%s\"\n", g_binaryswitch_RESOURCE_TYPE[a]);
        oc_resource_bind_resource_type(res_binaryswitch,g_binaryswitch_RESOURCE_TYPE[a]);
    }

    oc_resource_bind_resource_interface(res_binaryswitch,  OC_IF_A); /* oic.if.a */
    oc_resource_bind_resource_interface(res_binaryswitch,  OC_IF_BASELINE); /* oic.if.baseline */
    oc_resource_set_default_interface(res_binaryswitch,  OC_IF_A);  
    PRINT("     Default OCF Interface: 'oic.if.a'\n"); 
    oc_resource_set_discoverable(res_binaryswitch, true);
    /* periodic observable
       to be used when one wants to send an event per time slice
       period is 1 second */
    oc_resource_set_periodic_observable(res_binaryswitch, 1);
    /* set observable
       events are send when oc_notify_observers(oc_resource_t *resource) is called.
       this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
    /*oc_resource_set_observable(res_binaryswitch, true); */

    oc_resource_set_request_handler(res_binaryswitch, OC_GET, get_binaryswitch, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_binaryswitch);
#endif 
    oc_resource_set_request_handler(res_binaryswitch, OC_POST, post_binaryswitch, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_binaryswitch);
#endif
    oc_add_resource(res_binaryswitch);
    PRINT("Register Resource with local path \"/media\"\n");
    oc_resource_t *res_media = oc_new_resource(NULL, g_media_RESOURCE_ENDPOINT, g_media_nr_resource_types, 0);
    PRINT("     number of Resource Types: %d\n", g_media_nr_resource_types);
    for( int a = 0; a < g_media_nr_resource_types; a++ ) {
        PRINT("     Resource Type: \"%s\"\n", g_media_RESOURCE_TYPE[a]);
        oc_resource_bind_resource_type(res_media,g_media_RESOURCE_TYPE[a]);
    }

    oc_resource_bind_resource_interface(res_media,  OC_IF_A); /* oic.if.a */
    oc_resource_bind_resource_interface(res_media,  OC_IF_BASELINE); /* oic.if.baseline */
    oc_resource_set_default_interface(res_media,  OC_IF_A);  
    PRINT("     Default OCF Interface: 'oic.if.a'\n"); 
    oc_resource_set_discoverable(res_media, true);
    /* periodic observable
       to be used when one wants to send an event per time slice
       period is 1 second */
    oc_resource_set_periodic_observable(res_media, 1);
    /* set observable
       events are send when oc_notify_observers(oc_resource_t *resource) is called.
       this function must be called when the value changes, preferable on an interrupt when something is read from the hardware. */
    /*oc_resource_set_observable(res_media, true); */

    oc_resource_set_request_handler(res_media, OC_GET, get_media, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_media);
#endif 
    oc_resource_set_request_handler(res_media, OC_POST, post_media, NULL);


#ifdef OC_CLOUD
    oc_cloud_add_resource(res_media);
#endif
    oc_add_resource(res_media);
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
    char binaryswitch[MAX_PAYLOAD_STRING] = {0};
    char url[MAX_PAYLOAD_STRING] = {0};
    char sdp[MAX_PAYLOAD_STRING] = {0};
    oc_storage_read("binaryswitch", (uint8_t*)binaryswitch, sizeof(binaryswitch));
    oc_storage_read("url", (uint8_t*)url, sizeof(url));
    oc_storage_read("sdp", (uint8_t*)sdp, sizeof(sdp));

    /* initialize global variables for resource "/binaryswitch" */  
    g_binaryswitch_value = (strcmp(binaryswitch, "0"))?true:false; /* current value of property "value" The status of the switch. */
    /* initialize global variables for resource "/media" */
    /* initialize array "media" :  */
    memset((void*)&g_media_media, 0, sizeof(struct _media_media_t));
    strncpy(g_media_media[0].sdp[0], sdp ,MAX_PAYLOAD_STRING-1);
    strncpy(g_media_media[0].url, url ,MAX_PAYLOAD_STRING-1); 

    g_media_media_array_size = 1;


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

    PRINT("Used input file : \"../camera/out_codegeneration_merged.swagger.json\"\n");
    PRINT("OCF Server name : \"server_camera\"\n");

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

    PRINT("OCF server \"server_camera\" running, waiting on incoming connections.\n");

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
