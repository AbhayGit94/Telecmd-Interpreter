//
//  main.c
//  Telecommand Interpreter
//
//  Created by Abhay Gojiya on 04.10.21.
//

#include <stdio.h>
#include "telecmd_interpreter.h"

int main(int argc, const char * argv[])
{
    /* After Receving Command Batch file from ground station,
     * telecmdInterpreter will handle it for further process. */
    telecmdInterpreter();
    return 0;
}
