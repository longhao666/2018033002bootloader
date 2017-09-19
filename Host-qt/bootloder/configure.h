#ifndef CONFIGURE_H
#define CONFIGURE_H

#define CFG_LEN 0x80

/* only RW entries in object dictionary will
 * be stored in flash, others are specified
 * in the code
 **/

/* communication*/
#define NODE_ID                0x00    /* Mapped at index 0x2000, subindex 0x00 uint16_t */
#define MODULE_TYPE            0x01    /* Mapped at index 0x2001, subindex 0x00 uint16_t */
#define BAUDRATE               0x02    /* Mapped at index 0x2002, subindex 0x00 uint16_t */

#define MOT_POLE_PAIRS         0x04    /* Mapped at index 0x2003, subindex 0x01 uint16_t */
#define MOT_RESISTANCE_PHASE   0x05    /* Mapped at index 0x2003, subindex 0x02 uint16_t */
#define MOT_INDUCTANCE_PHASE   0x06    /* Mapped at index 0x2003, subindex 0x03 uint16_t */
#define MOT_POLE_PITCH         0x07    /* Mapped at index 0x2003, subindex 0x04 uint16_t */
#define MOT_BACKEMF_CONST      0x08    /* Mapped at index 0x2003, subindex 0x05 uint16_t */
#define MOT_TORQUE_CONST       0x09    /* Mapped at index 0x2003, subindex 0x06 uint16_t */

#define REAL_TEMP              0x10    /* Mapped at index 0x2004, subindex 0x01  int16_t */
#define MIN_TEMP               0x11    /* Mapped at index 0x2004, subindex 0x01  int16_t */
#define MAX_TEMP               0x12    /* Mapped at index 0x2004, subindex 0x02  int16_t */
#define REAL_VOLTAGE           0x13    /* Mapped at index 0x2005, subindex 0x01  int16_t */
#define MIN_VOLTAGE            0x14    /* Mapped at index 0x2005, subindex 0x01  int16_t */
#define MAX_VOLTAGE            0x15    /* Mapped at index 0x2005, subindex 0x01  int16_t */
#define GEAR_RATIO             0x16    /* Mapped at index 0x6091, subindex 0x01 uint32_t */

#define HOME_OFFSET            0x20    /* Mapped at index 0x607C, subindex 0x00 int32_t */
#define MIN_POS_LIMIT          0x22    /* Mapped at index 0x607D, subindex 0x01 int32_t */
#define MAX_POS_LIMIT          0x24    /* Mapped at index 0x607D, subindex 0x02 int32_t */
#define MAX_PROFILE_VEL        0x26    /* Mapped at index 0x607F, subindex 0x00 uint32_t */
#define MAX_ACCELERATION       0x28    /* int32_t */
#define MAX_DECELERATION       0x2A    /* int32_t */
#define MAX_FOLLOWING_ERR      0x2C    /* Mapped at index 0x6065, subindex 0x00 uint32_t*/


#define POS_PGAIN              0x30   /* int16_t */
#define POS_IGAIN              0x31   /* int16_t */
#define POS_DGAIN              0x32   /* int16_t */
#define POS_PGAIN_DIV          0x33   /* uint16_t */
#define POS_IGAIN_DIV          0x34   /* uint16_t */
#define POS_DGAIN_DIV          0x35   /* uint16_t */
#define POS_MIN_INTEGRAL       0x36   /* int32_t */
#define POS_MAX_INTEGRAL       0x38   /* int32_t */
#define POS_MIN_OUTPUT         0x3A   /* int32_t */
#define POS_MAX_OUTPUT         0x3C   /* int32_t */
#define POS_DEADAREA           0x3E   /* int32_t */

#define VEL_PGAIN              0x40   /* int16_t */
#define VEL_IGAIN              0x41   /* int16_t */
#define VEL_DGAIN              0x42   /* int16_t */
#define VEL_PGAIN_DIV          0x43   /* uint16_t */
#define VEL_IGAIN_DIV          0x44   /* uint16_t */
#define VEL_DGAIN_DIV          0x45   /* uint16_t */
#define VEL_MIN_INTEGRAL       0x46   /* int32_t */
#define VEL_MAX_INTEGRAL       0x48   /* int32_t */
#define VEL_MIN_OUTPUT         0x4A   /* int32_t */
#define VEL_MAX_OUTPUT         0x4C   /* int32_t */
#define VEL_DEADAREA           0x4E   /* int32_t */

#define TORQ_PGAIN             0x50   /* int16_t */
#define TORQ_IGAIN             0x51   /* int16_t */
#define TORQ_DGAIN             0x52   /* int16_t */
#define TORQ_PGAIN_DIV         0x53   /* uint16_t */
#define TORQ_IGAIN_DIV         0x54   /* uint16_t */
#define TORQ_DGAIN_DIV         0x55   /* uint16_t */
#define TORQ_MIN_INTEGRAL      0x56   /* int32_t */
#define TORQ_MAX_INTEGRAL      0x58   /* int32_t */
#define TORQ_MIN_OUTPUT        0x5A   /* int32_t */
#define TORQ_MAX_OUTPUT        0x5C   /* int32_t */
#define TORQ_DEADAREA_L        0x5E   /* int32_t */

#define FLUX_PGAIN             0x60   /* int16_t */
#define FLUX_IGAIN             0x61   /* int16_t */
#define FLUX_DGAIN             0x62   /* int16_t */
#define FLUX_PGAIN_DIV         0x63   /* uint16_t */
#define FLUX_IGAIN_DIV         0x64   /* uint16_t */
#define FLUX_DGAIN_DIV         0x65   /* uint16_t */
#define FLUX_MIN_INTEGRAL      0x66   /* int32_t */
#define FLUX_MAX_INTEGRAL      0x68   /* int32_t */
#define FLUX_MIN_OUTPUT        0x6A   /* int32_t */
#define FLUX_MAX_OUTPUT        0x6C   /* int32_t */
#define FLUX_DEADAREA_L        0x6E   /* int32_t */

#endif // CONFIGURE_H
