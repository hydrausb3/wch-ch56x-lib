#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define ENDP_1_15_MAX_PACKET_SIZE 1024

/* Global define */
// DEF_ENDP_OUT_BURST_LEVEL / DEF_ENDP_IN_BURST_LEVEL maximum burst size 16
// defined by the USB3 specification Warning USB3 endpoint bulk with 8 or 16
// burst can be problematic on some PC
#define DEF_ENDP_OUT_BURST_LEVEL 4
#define DEF_ENDP_IN_BURST_LEVEL (DEF_ENDP_OUT_BURST_LEVEL)
#define DEF_ENDP_MAX_SIZE (DEF_ENDP1_OUT_BURST_LEVEL * 1024)

#endif
