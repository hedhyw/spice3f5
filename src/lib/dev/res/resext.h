/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#ifdef __STDC__
extern int RESask(CKTcircuit*,GENinstance*,int,IFvalue*,IFvalue*);
extern int RESdelete(GENmodel*,IFuid,GENinstance**);
extern void RESdestroy(GENmodel**);
extern int RESload(GENmodel*,CKTcircuit*);
extern int RESmodAsk(CKTcircuit*,GENmodel*,int,IFvalue*);
extern int RESmDelete(GENmodel**,IFuid,GENmodel*);
extern int RESmParam(int,IFvalue*,GENmodel*);
extern int RESparam(int,IFvalue*,GENinstance*,IFvalue*);
extern int RESpzLoad(GENmodel*,CKTcircuit*,SPcomplex*);
extern int RESsAcLoad(GENmodel*,CKTcircuit*);
extern int RESsLoad(GENmodel*,CKTcircuit*);
extern int RESsSetup(SENstruct*,GENmodel*);
extern void RESsPrint(GENmodel*,CKTcircuit*);
extern int RESsetup(SMPmatrix*,GENmodel*,CKTcircuit*,int*);
extern int REStemp(GENmodel*,CKTcircuit*);
extern int RESnoise(int,int,GENmodel*,CKTcircuit*,Ndata*,double*);
#else /* stdc */
extern int RESask();
extern int RESdelete();
extern void RESdestroy();
extern int RESload();
extern int RESmodAsk();
extern int RESmDelete();
extern int RESmParam();
extern int RESparam();
extern int RESpzLoad();
extern int RESsAcLoad();
extern int RESsLoad();
extern int RESsSetup();
extern void RESsPrint();
extern int RESsetup();
extern int REStemp();
extern int RESnoise();
#endif /* stdc */

