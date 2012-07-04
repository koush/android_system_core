// Overload this file in your own device-specific config if you need
// non-standard property_perms and/or control_perms structs
//
// To avoid conflicts...
// if you redefine property_perms, #define PROPERTY_PERMS there
// if you redefine control_perms, #define CONTROL_PARMS there
//
// A typical file will look like:
//
/*

#define CONTROL_PERMS

struct {
    const char *service;
    unsigned int uid;
    unsigned int gid;
} control_perms[] = {
    // The default perms
    { "dumpstate",AID_SHELL, AID_LOG },
    { "ril-daemon",AID_RADIO, AID_RADIO },
    // My magic device perms
    { "rawip_vsnet1",AID_RADIO, AID_RADIO },
    { "rawip_vsnet2",AID_RADIO, AID_RADIO },
    { "rawip_vsnet3",AID_RADIO, AID_RADIO },
    { "rawip_vsnet4",AID_RADIO, AID_RADIO },
     {NULL, 0, 0 }
};
*/
