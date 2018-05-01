
#include <memory.h>
#include "spice.h"
#include "ftegraph.h"
#include "ftedev.h"
#include "fteinput.h"
#include "ftedefs.h"  
#include <console.h>

char *argv[2];

int
initmac(av)
  char ***av;
{	
	MoreMasters;
	MoreMasters;
	MoreMasters;
	MoreMasters;
	MoreMasters;
	MoreMasters;

	console_options.pause_atexit=false;
	console_options.title="\pBerkeley Spice";
	argv[0] = 0;
	argv[1] = 0;
	*av = argv;
	return(1);
}



char 
*_memavl() 
{ 
	long int grow, free;
	free = MaxMem(&grow);
	if (grow == 0) return((char *)free);
	return((char *)grow); 
}


int  
QDraw_Init()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_NewViewport()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Close()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Clear()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_DrawLine()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Arc()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Text()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_DefineColor()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_DefineLinestyle()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_SetLinestyle()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_SetColor()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Update()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Track()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_MakeMenu()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_MakeDialog()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}


int
QDraw_Input(request, response)
    REQUEST *request;
    RESPONSE *response;
{

    switch (request->option) {
      case char_option:
        response->reply.ch = inchar(request->fp);
        response->option = request->option;
        break;
      default:
        /* just ignore, since we don't want a million error messages */
	if (response)
	    response->option = error_option;
        break;
    }
    return;

}

 
int
QDraw_DatatoScreen()
{

    sprintf(ErrorMessage,
        "This operation is not defined for display type %s.",
        dispdev->name);
    internalerror(ErrorMessage);
    return(1);

}