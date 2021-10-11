/**
 * @file telecmd_interpreter.c
 *
 * @brief Telecommand Interpreter Source Code. This file is responsible for telecommand batch
 * file interpret, queue and execute the commands.
 *
 * @author Abhay Gojiya
 * Contact: abhaygojiya@gmail.com
 *
 */

/* System includes */
#include <string.h>
#include <stdlib.h>

/* Custom includes */
#include "telecmd_interpreter.h"

/* Defines and Data Types */
#define TELECMD_FILE "CMD.bat"
//#define TELECMD_FILE    "../../../Telecommand Interpreter/CMD.bat"
#define MAX_LENGTH      256
#define INVALID_VAL     0

/* Static Variables */
static UINT32 nodeEntryIdx; /* Unique Idx for nodes of TeleCommand Queue */
static TELE_CMD_LIST_t *pHeadTeleCmdQ      = NULL; /* Head of the Queue */

/* Pointer for sorting the list */
static TELE_CMD_LIST_t *pFirstHandlerPtr   = NULL; /* First list pointer */
static TELE_CMD_LIST_t *pFirstEndPtr       = NULL; /* First list end pointer */
static TELE_CMD_LIST_t *pSecondHandlerPtr  = NULL; /* Second list pointer */
static TELE_CMD_LIST_t *pSecondEndPtr      = NULL; /* Second end pointer */

/* Function Prototypes */
static VOID addNewCmdDataIntoQueue(TELECMD_CONFIG_t *pRcvdTeleCmdData);
static VOID deleteCmdDataFromQueue(UINT32 refEntryIdx);
static VOID sortTeleCmdQueue(VOID);
static UINT32 getLengthOfCmdQueue(VOID);
static VOID mergeReorderNodeOfQueue(VOID);
static VOID swapHandlingPtr(VOID);
static VOID modifyCmdDataInQueue(UINT32 refEntryIdx, UINT32 refNewData);
static VOID printCmdDataQueue(VOID);
static VOID reverseCmdQueue(VOID);
static VOID executeCmdFromQueue(VOID);

/* Function Definitions */

/*------------------------------------------------------------------------------
 * FUNCTION: telecmdInterpreter()
 *------------------------------------------------------------------------------
 * ABSTRACT: This Function will read data from batch file, parse it,
 *           check the command type and based on type it will execute it
 *           or add into queue.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *             IN:    None
 *             OUT:   None
 * RETURN VALUE: -
 *------------------------------------------------------------------------------
 * GLOBALS: -
 *
 *----------------------------------------------------------------------------*/
VOID telecmdInterpreter(VOID)
{
    FILE *pCmdFile              = NULL;          /* Pointer to cmd batch file */
    CHAR cmdBuffer[MAX_LENGTH]  = {INVALID_VAL}; /* Buffer to read commands */
    
    pCmdFile = fopen(TELECMD_FILE, "r");
    if(pCmdFile == NULL)
    {
        printf("ERROR: Failed to open telecommand file\n");
        return;
    }
    
    /* Read line by line until EOF */
    while( fgets(cmdBuffer, MAX_LENGTH, pCmdFile) )
    {
        TELECMD_CONFIG_t parseCmdData = {INVALID_VAL}; /* store parsed values */
        UINT32 lenghtOfCmd = (UINT32) strlen(cmdBuffer); /* lenght of command */
        
        cmdBuffer[lenghtOfCmd-1] = '\0';
        /* Parse command id and check the type (utility or telecommand) */
        sscanf(cmdBuffer, "%u", &parseCmdData.teleCmd);
        
        switch(parseCmdData.teleCmd)
        {
            case CMD_NEWCMD_WITH_LOW_PRIO:
                /* Parse the command values from buffer */
                sscanf(cmdBuffer, "%u %u",  &parseCmdData.teleCmd,
                                            &parseCmdData.cmdData);
                /* Add command data into Queue */
                addNewCmdDataIntoQueue(&parseCmdData);
                break;
            
            case CMD_NEWCMD_WITH_USER_PRIO:
                /* Parse the command values from buffer */
                sscanf(cmdBuffer, "%u %u %u",   &parseCmdData.teleCmd,
                                                &parseCmdData.cmdPriority,
                                                &parseCmdData.cmdData);
                /* Add command data into Queue */
                addNewCmdDataIntoQueue(&parseCmdData);
                break;
                
            case CMD_DELETE_CMD_FROM_QUEUE:
                /* Parse the command values from buffer */
                sscanf(cmdBuffer, "%u %u",  &parseCmdData.teleCmd,
                                            &parseCmdData.targetIdx);
                /* Add command data into Queue */
                addNewCmdDataIntoQueue(&parseCmdData);
                break;
                
            case CMD_MODIFY_CMD_DATA_IN_QUEUE:
                /* Parse the command values from buffer */
                sscanf(cmdBuffer, "%u %u %u",   &parseCmdData.teleCmd,
                                                &parseCmdData.targetIdx,
                                                &parseCmdData.newCmdData);
                /* Add command data into Queue */
                addNewCmdDataIntoQueue(&parseCmdData);
                break;
            
            case CMD_PRINT_CMDS:
                /* Utility command: Print the command list */
                printCmdDataQueue();
                break;
                
            case CMD_SORT_CMD_QUEUE:
                /* Utility command: Sort the command list */
                sortTeleCmdQueue();
                break;

            case CMD_REVERSE_CMD_QUEUE:
                /* Utility command: Reverse the command list */
                reverseCmdQueue();
                break;

            case CMD_EXECUTE_CMDS:
                /* Utility command: Execute the command list */
                executeCmdFromQueue();
                break;
                
            default:
                printf("ERROR: Invalid Command Received [%s]\n", cmdBuffer);
                break;
        }
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION: addNewCmdDataIntoQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will allocate memory in heap section for new data
 *           node, fill the data and it will add new data node into
 *           telecommand queue.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    Address of new telecommand struct node.
 *              OUT:   None
 * RETURN VALUE: -
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *          nodeEntryIdx (Unique Idx for nodes)
 *----------------------------------------------------------------------------*/
static VOID addNewCmdDataIntoQueue(TELECMD_CONFIG_t *pRcvdTeleCmdData)
{
    /* Allocate heap memory for new command node */
    TELE_CMD_LIST_t *pNewTeleCmdNode = (TELE_CMD_LIST_t *) malloc(sizeof(TELE_CMD_LIST_t));
    
    if (pNewTeleCmdNode == NULL)
    {
        printf("ERROR: Failed to assign dynamic memory for new node\n");
        return;
    }
    
    /* Assign unique entry Idx to new node for further reference */
    pRcvdTeleCmdData->entryIdx = nodeEntryIdx++;
    /* Copy the parsed command data into heap memory of new node */
    memcpy( &(pNewTeleCmdNode->teleCmdData), pRcvdTeleCmdData, sizeof(TELECMD_CONFIG_t) );

    /* Update the pointers of new node */
    pNewTeleCmdNode->pNextCmdNode = pHeadTeleCmdQ;
    pNewTeleCmdNode->pPrevCmdNode = NULL;
     
    if (pHeadTeleCmdQ != NULL)
    {
        /* update the previous node pointer of first node if list not empty*/
        pHeadTeleCmdQ->pPrevCmdNode = pNewTeleCmdNode;
    }
    
    /* Update the head pointer as nodes are added at front of list */
    pHeadTeleCmdQ = pNewTeleCmdNode;
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION: deleteCmdDataFromQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will find node with given entry idx and delete that
 *           node from Queue.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    Entry Idx
 *              OUT:   None
 * RETURN VALUE: -
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *----------------------------------------------------------------------------*/
static VOID deleteCmdDataFromQueue(UINT32 refEntryIdx)
{
    TELE_CMD_LIST_t  *pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    
    while (pCurPosNode != NULL)
    {
        /* compare the entry Idx with target entry idx and delete the node */
        if (pCurPosNode->teleCmdData.entryIdx == refEntryIdx)
        {
            if (pCurPosNode == pHeadTeleCmdQ)
            {
                pHeadTeleCmdQ = pCurPosNode->pNextCmdNode;
            }
            else
            {
                pCurPosNode->pPrevCmdNode->pNextCmdNode = pCurPosNode->pNextCmdNode;
            }
            /* Free the memory of node */
            free(pCurPosNode);
            return;
        }
        pCurPosNode = pCurPosNode->pNextCmdNode;
    }
    
    printf("ERROR: deleteCmdDataFromQueue: Node not found in Queue\n");
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION: sortTeleCmdQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will Ensure that commands are sorted by their
 *           priorities. For sorting we are using merge sort because
 *           it is very efficient for immutable datastructures.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 * RETURN VALUE: -
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue),
 *          pFirstHandlerPtr (Handling pointer for first part of queue),
 *          pFirstEndPtr (Handling pointer for first part of queue),
 *          pSecondHandlerPtr (Handling pointer for second part of queue),
 *          pSecondEndPtr(Handling pointer for second part of queue)
 *----------------------------------------------------------------------------*/
static VOID sortTeleCmdQueue(VOID)
{
    /* If list is empty, no need to sort */
    if (pHeadTeleCmdQ == NULL)
    {
        return;
    }
    
    TELE_CMD_LIST_t *pHoldNode = NULL; /* hold location to handle pointers */
    UINT32 lHalfQueueVar = INVALID_VAL;  /* loop varible for devide the list */
    UINT32 lenOfQueue = getLengthOfCmdQueue(); /* length of the list */
    
    /* The loop var is initially 1. It is incremented as 2, 4, 8, ..
       until it reaches the length of the linked list. For each loop,
       the linked list is sorted around the loop */
    for (lHalfQueueVar = 1; lHalfQueueVar < lenOfQueue; lHalfQueueVar = lHalfQueueVar*2)
    {
        /* Everytime start first list pointer from head location */
        pFirstHandlerPtr = pHeadTeleCmdQ;
        while (pFirstHandlerPtr)
        {
            UINT32 lHalfQCounter = lHalfQueueVar; /* counter for end pointer handling */
            BOOL isFirstIter = FALSE; /* flag var to check first iteration */
            TELE_CMD_LIST_t *pNextIterNode  = NULL; /* Pointer to store next iteration start location */
            
            /* If it is first iteration, update the flag value */
            if (pFirstHandlerPtr == pHeadTeleCmdQ)
            {
                isFirstIter = TRUE;
            }

            pFirstEndPtr = pFirstHandlerPtr;
            /* Set first end pointer as per loop cont and end of list */
            while (--lHalfQCounter && pFirstEndPtr->pNextCmdNode)
            {
                pFirstEndPtr = pFirstEndPtr->pNextCmdNode;
            }
            
            /* set second list pointer as per end of first list */
            pSecondHandlerPtr = pFirstEndPtr->pNextCmdNode;
            if (pSecondHandlerPtr == NULL)
            {
                /* end of list, so break the loop */
                break;
            }
            
            /* Reupdate the counter for second end pointer */
            lHalfQCounter = lHalfQueueVar;
            
            pSecondEndPtr = pSecondHandlerPtr;
            /* Set second end pointer as per loop cont and end of list */
            while (--lHalfQCounter && pSecondEndPtr->pNextCmdNode)
            {
                pSecondEndPtr = pSecondEndPtr->pNextCmdNode;
            }

            /* store next iteartion location */
            pNextIterNode = pSecondEndPtr->pNextCmdNode;
            
            /* merge and sort the list */
            mergeReorderNodeOfQueue();
 
            if (isFirstIter == TRUE)
            {
                /* update head if iteration is first */
                pHeadTeleCmdQ = pFirstHandlerPtr;
            }
            else
            {
                pHoldNode->pNextCmdNode = pFirstHandlerPtr;
            }
            
            /* hold the second end pointer location and set first pointer */
            pHoldNode = pSecondEndPtr;
            pFirstHandlerPtr = pNextIterNode;
        }
        pHoldNode->pNextCmdNode = pFirstHandlerPtr;
    }
    /* After sorting set NULL to first element of list */
    pHeadTeleCmdQ->pPrevCmdNode = NULL;
}

/*------------------------------------------------------------------------------
 * FUNCTION: getLengthOfCmdQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will calculate the length of queue and return
 *           the length.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 * RETURN VALUE: Length of the Queue (UINT32)
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *----------------------------------------------------------------------------*/
static UINT32 getLengthOfCmdQueue(VOID)
{
    TELE_CMD_LIST_t* pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    UINT32 countLen = INVALID_VAL; /* to count the length of list */
    
    while(pCurPosNode != NULL)
    {
        countLen++;
        pCurPosNode = pCurPosNode->pNextCmdNode;
    }
    
    return countLen;
}

/*------------------------------------------------------------------------------
 * FUNCTION: mergeReorderNodeOfQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function is used for sorting of Queue. It will check the
 *           command priority and sort it.
 *           It will compare the next value of pFirstHandlerPtr and
 *           current value of pSecondHandlerPtr and insert pSecondHandlerPtr
 *           after pFirstHandlerPtr if it's smaller than next value of
 *           pFirstHandlerPtr. do this until pFirstHandlerPtr or
 *           pSecondHandlerPtr end.
 *           If pFirstHandlerPtr ends, then we assign next of pFirstHandlerPtr
 *           to pSecondHandlerPtr because pSecondHandlerPtr may have some
 *           elements left out which are greater than the last value of
 *           pFirstHandlerPtr. If pSecondHandlerPtr ends then it will assign
 *           pSecondEndPtr to pFirstEndPtr.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue),
 *          pFirstHandlerPtr (Handling pointer for first part of queue),
 *          pFirstEndPtr (Handling pointer for first part of queue),
 *          pSecondHandlerPtr (Handling pointer for second part of queue),
 *          pSecondEndPtr(Handling pointer for second part of queue)
 *----------------------------------------------------------------------------*/
static VOID mergeReorderNodeOfQueue(VOID)
{
    TELE_CMD_LIST_t *pMergeA    = NULL; /* Merge pointer for List A */
    TELE_CMD_LIST_t *pMergeB    = NULL; /* Merge pointer for List B */
    TELE_CMD_LIST_t *pMergeEndA = NULL; /* Merge end pointer for List A */
    TELE_CMD_LIST_t *pMergeEndB = NULL; /* Merge end pointer for List A */

    /* if priority of first list pointer is less then second list pointer,
    Swap the pointers of first and second list */
    if (pFirstHandlerPtr->teleCmdData.cmdPriority < pSecondHandlerPtr->teleCmdData.cmdPriority)
    {
        swapHandlingPtr();
    }
 
    /* store first second handling pointers into merge pointers */
    pMergeA     = pFirstHandlerPtr;
    pMergeB     = pSecondHandlerPtr;
    pMergeEndA  = pFirstEndPtr;
    pMergeEndB  = pSecondEndPtr->pNextCmdNode;
 
    /* Check Merge pointer location for first list and second list */
    while (pMergeA != pMergeEndA && pMergeB != pMergeEndB)
    {
        /* swap the node if priority of first list node is less than second */
        if (pMergeA->pNextCmdNode->teleCmdData.cmdPriority < pMergeB->teleCmdData.cmdPriority)
        {
            TELE_CMD_LIST_t *pSwapNode = pMergeB->pNextCmdNode;
            pMergeB->pNextCmdNode = pMergeA->pNextCmdNode;
            pMergeA->pNextCmdNode = pMergeB;
            pMergeB->pPrevCmdNode = pMergeA;
            pMergeB->pNextCmdNode->pPrevCmdNode = pMergeB;
            pMergeB = pSwapNode;
        }
        pMergeA = pMergeA->pNextCmdNode;
    }
    

    if (pMergeA == pMergeEndA)
    {
        /* assign next of pFirstHandlerPtr to pSecondHandlerPtr because
         * pSecondHandlerPtr may have some elements left out which are greater
         * than the last value of pFirstHandlerPtr */
        pMergeA->pNextCmdNode = pMergeB;
        pMergeB->pPrevCmdNode = pMergeA;
    }
    else
    {
        pSecondEndPtr = pFirstEndPtr;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION: swapHandlingPtr()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function is used to swap pointers of first queue and
 *           pointers of second queue.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS:
 *          pFirstHandlerPtr (Handling pointer for first part of queue),
 *          pFirstEndPtr (Handling pointer for first part of queue),
 *          pSecondHandlerPtr (Handling pointer for second part of queue),
 *          pSecondEndPtr(Handling pointer for second part of queue)
 *----------------------------------------------------------------------------*/
static VOID swapHandlingPtr(VOID)
{
    TELE_CMD_LIST_t *pSwapNodePtr = NULL; /* temp pointer for swapping */
    
    /* Swap handler pointers*/
    pSwapNodePtr = pFirstHandlerPtr;
    pFirstHandlerPtr = pSecondHandlerPtr;
    pSecondHandlerPtr = pSwapNodePtr;
    /* Swap end pointers*/
    pSwapNodePtr = pFirstEndPtr;
    pFirstEndPtr = pSecondEndPtr;
    pSecondEndPtr = pSwapNodePtr;
}

/*------------------------------------------------------------------------------
 * FUNCTION: modifyCmdDataInQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will find anf modify the command data using entry idx
 *           of the node.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    Entry Idx and New Data
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *----------------------------------------------------------------------------*/
static VOID modifyCmdDataInQueue(UINT32 refEntryIdx, UINT32 refNewData)
{
    TELE_CMD_LIST_t *pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    while(pCurPosNode != NULL)
    {
        /* Search for entry Idx and update the new data in to command node */
        if(pCurPosNode->teleCmdData.entryIdx == refEntryIdx)
        {
            pCurPosNode->teleCmdData.cmdData = refNewData;
            return;
        }
        pCurPosNode = pCurPosNode->pNextCmdNode;
    }
    return;
}

/*------------------------------------------------------------------------------
 * FUNCTION:   printCmdDataQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function is used to print the commands from queue based on
 *           command id.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *
 *----------------------------------------------------------------------------*/
static VOID printCmdDataQueue(VOID)
{
    TELE_CMD_LIST_t  *pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    
    while (pCurPosNode != NULL)
    {
        switch(pCurPosNode->teleCmdData.teleCmd)
        {
            case CMD_NEWCMD_WITH_LOW_PRIO:
                /* Print entry Idx, priority and data of the node */
                printf("(%u, %u, %u)\n", pCurPosNode->teleCmdData.entryIdx,
                                         pCurPosNode->teleCmdData.cmdPriority,
                                         pCurPosNode->teleCmdData.cmdData);
                break;
                
            case CMD_NEWCMD_WITH_USER_PRIO:
                /* Print entry Idx, priority and data of the node */
                printf("(%u, %u, %u)\n", pCurPosNode->teleCmdData.entryIdx,
                                         pCurPosNode->teleCmdData.cmdPriority,
                                         pCurPosNode->teleCmdData.cmdData);
                break;
                
            case CMD_DELETE_CMD_FROM_QUEUE:
                /* Print entry Idx and Target Idx which we want to detele */
                printf("(%u, %u)\n", pCurPosNode->teleCmdData.entryIdx,
                                     pCurPosNode->teleCmdData.targetIdx);
                break;
                
            case CMD_MODIFY_CMD_DATA_IN_QUEUE:
                /* Print entry Idx, Target Idx and new data */
                printf("(%u, %u, %u)\n", pCurPosNode->teleCmdData.entryIdx,
                                         pCurPosNode->teleCmdData.targetIdx,
                                         pCurPosNode->teleCmdData.newCmdData);
                break;
                
            case CMD_SORT_CMD_QUEUE:
            case CMD_PRINT_CMDS:
            case CMD_REVERSE_CMD_QUEUE:
            default:
                printf("ERROR: Invalid Command found in Queue\n");
                break;
        }
        pCurPosNode = pCurPosNode->pNextCmdNode;
    }
}

/*------------------------------------------------------------------------------
 * FUNCTION: reverseCmdQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will reverse the command list so that the commands
 *           with the least priority become the most prior commands and
 *           update head pointer.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *
 *----------------------------------------------------------------------------*/
static VOID reverseCmdQueue(VOID)
{
    TELE_CMD_LIST_t  *pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    TELE_CMD_LIST_t  *pRefNode = NULL; /* Ptr for previous node handling */

    while(pCurPosNode != NULL)
    {
        /* Reverse the list */
        pRefNode = pCurPosNode->pPrevCmdNode;
        pCurPosNode->pPrevCmdNode = pCurPosNode->pNextCmdNode;
        pCurPosNode->pNextCmdNode = pRefNode;
        pCurPosNode = pCurPosNode->pPrevCmdNode;
    }
     
    /* check empty list and list with only one node before updating Head */
    if(pRefNode != NULL )
       pHeadTeleCmdQ = pRefNode->pPrevCmdNode;
}

/*------------------------------------------------------------------------------
 * FUNCTION: executeCmdFromQueue()
 *------------------------------------------------------------------------------
 * ABSTRACT: This function will execute the command from the list and
 *           after execution it will remove the command from the list.
 *------------------------------------------------------------------------------
 * PARAMETERS:
 *              IN:    None
 *              OUT:   None
 *------------------------------------------------------------------------------
 * GLOBALS: pHeadTeleCmdQ (Head pointer of Queue)
 *
 *----------------------------------------------------------------------------*/
static VOID executeCmdFromQueue(VOID)
{
    TELE_CMD_LIST_t *pCurPosNode = pHeadTeleCmdQ; /* Ptr for Queue Handling */
    TELE_CMD_LIST_t *pHoldDelPos = NULL; /* Ptr for remove the node from Queue */
    
    while (pCurPosNode != NULL)
    {
        switch (pCurPosNode->teleCmdData.teleCmd)
        {
            case CMD_NEWCMD_WITH_LOW_PRIO:
            case CMD_NEWCMD_WITH_USER_PRIO:
                /* For Now, No action required for this command */
                break;
                
            case CMD_DELETE_CMD_FROM_QUEUE:
                /* if targetIdx is not own entryIdx, find and delete the node */
                if(pCurPosNode->teleCmdData.targetIdx != pCurPosNode->teleCmdData.entryIdx)
                {
                    deleteCmdDataFromQueue(pCurPosNode->teleCmdData.targetIdx);
                }
                break;
                
            case CMD_MODIFY_CMD_DATA_IN_QUEUE:
                /* modify the command data as per request */
                modifyCmdDataInQueue(pCurPosNode->teleCmdData.targetIdx, pCurPosNode->teleCmdData.newCmdData);
                break;
                
            case CMD_SORT_CMD_QUEUE:
            case CMD_PRINT_CMDS:
            case CMD_REVERSE_CMD_QUEUE:
            default:
                printf("ERROR: Invalid Command found in Queue\n");
                break;
        }
       
        /*  Hold current position before next position so we can delete it */
        pHoldDelPos = pCurPosNode;
        pCurPosNode = pCurPosNode->pNextCmdNode;
    
        /* Delete the command from list as it is executed */
        if(pHoldDelPos != NULL)
        {
            if (pHoldDelPos == pHeadTeleCmdQ)
            {
                pHeadTeleCmdQ = pHoldDelPos->pNextCmdNode;
            }
            else
            {
                pHoldDelPos->pPrevCmdNode->pNextCmdNode = pHoldDelPos->pNextCmdNode;
            }
            /* Free the memory of the node */
            free(pHoldDelPos);
        }
    }
}
