/**
 * @file telecmd_typeDef.h
 *
 * @brief Data types for project. use this datatypes for project for better coding practice.
 *        Change according to your platform (KEIL, C55x, MpLab, ARM or others).
 *
 * @author Abhay Gojiya
 * Contact: abhaygojiya@gmail.com
 *
 */

#ifndef telecmd_typeDef_h
#define telecmd_typeDef_h

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

typedef unsigned char       BOOL;
typedef char                CHAR;

typedef signed char         INT8;
typedef unsigned char       UINT8;

typedef signed short        INT16;
typedef unsigned short      UINT16;

typedef signed int          INT32;
typedef unsigned int        UINT32;

typedef signed long long    INT64;
typedef unsigned long long  UINT64;

typedef INT8 *              INT8PTR;
typedef char *              CHARPTR;
typedef UINT8 *             UINT8PTR;
typedef UINT8 const *       UINT8CONSTPTR;

typedef INT16 *             INT16PTR;
typedef UINT16 *            UINT16PTR;
typedef UINT16 const *      UINT16CONSTPTR;

typedef INT32 *             INT32PTR;
typedef UINT32 *            UINT32PTR;
typedef UINT32 const *      UINTCONST32PTR;

typedef INT64 *             INT64PTR;
typedef UINT64 *            UINT64PTR;
typedef UINT64 const*       UINT64CONSTPTR;

typedef void                VOID;
typedef void *              VOIDPTR;

#endif /* telecmd_typeDef_h */
