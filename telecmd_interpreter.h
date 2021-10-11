/**
 * @file telecmd_interpreter.h
 *
 * @brief Telecommand Interpreter header file.
 *
 * @author Abhay Gojiya
 * Contact: abhaygojiya@gmail.com
 *
 */

#ifndef telecmd_interpreter_h
#define telecmd_interpreter_h

#include <stdio.h>
#include "telecmd_typeDef.h"

typedef enum
{
    CMD_NEWCMD_WITH_LOW_PRIO = 0,           //0
    CMD_NEWCMD_WITH_USER_PRIO,              //1
    CMD_DELETE_CMD_FROM_QUEUE,              //2
    CMD_SORT_CMD_QUEUE,                     //3
    CMD_MODIFY_CMD_DATA_IN_QUEUE,           //4
    CMD_PRINT_CMDS,                         //5
    CMD_REVERSE_CMD_QUEUE,                  //6
    CMD_EXECUTE_CMDS,                       //7

    MAX_CMDS,
}TELECMD_LIST_e;

/* Telecommand data*/
typedef struct
{
    UINT32              entryIdx;
    TELECMD_LIST_e      teleCmd;
    UINT32              cmdPriority;
    UINT32              cmdData;
    UINT32              targetIdx;
    UINT32              newCmdData;
}TELECMD_CONFIG_t;

/* Node of Telecommand Queue */
struct teleCmdNode
{
    TELECMD_CONFIG_t    teleCmdData;    // TeleCommand Data
    struct teleCmdNode  *pNextCmdNode;  // pointer to point next node
    struct teleCmdNode  *pPrevCmdNode;  // pointer to point prev node
};

typedef struct teleCmdNode TELE_CMD_LIST_t;


VOID telecmdInterpreter(VOID);

#endif /* telecmd_interpreter_h */
