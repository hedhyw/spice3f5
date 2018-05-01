/**********
Copyright 1991 Regents of the University of California.  All rights reserved.
**********/

#include "spice.h"
#include "util.h"
#include "ifsim.h"
#include "sperror.h"
#include "spmatrix.h"
#include "gendefs.h"
#include "devdefs.h"
#include "cktdefs.h"
#include "smpdefs.h"
#include "sensdefs.h"
#include "sensgen.h"

/* #define ASDEBUG */
#ifdef ASDEBUG
#define DEBUG(X)	if ((X) < Sens_Debug)
int Sens_Debug = 0;
char SF1[] = "res";
char SF2[] = "dc";
char SF3[] = "bf";
#endif
char *Sfilter = NULL;
double Sens_Delta = 0.000001;
double Sens_Abs_Delta = 0.000001;

static int sens_setp( ), sens_load( ), sens_temp( );
int sens_getp( );

extern SPICEdev *DEVices[];

/*
 *	Procedure:
 *
 *		Determine operating point (call CKTop)
 *
 *		For each frequency point:
 *			(for AC) call NIacIter to get base node voltages
 *			For each element/parameter in the test list:
 *				construct the perturbation matrix
 *				Solve for the sensitivities:
 *					delta_E = Y^-1 (delta_Y E - delta_I)
 *				save results
 */

extern double		inc_freq( );
static int	error;
sens_sens(ckt, restart)
	CKTcircuit	*ckt;
	int		restart;
{
	SENS_AN	*sen_info = ((SENS_AN *) ckt->CKTcurJob);
	static int	size;
	static double	*delta_I, *delta_iI,
			*delta_I_delta_Y, *delta_iI_delta_Y;
	sgen		*sg;
	static double	freq;
	static int	nfreqs;
	static int	i;
	static SMPmatrix	*delta_Y = NULL, *Y;
	static double	step_size;
	double		*E, *iE;
	IFvalue		value, nvalue;
	double		*output_values;
	IFcomplex	*output_cvalues;
	double		delta_var;
	int		(*fn)( );
	static int	is_dc;
	int		k, j, n;
	int		num_vars, branch_eq;
	char		*sen_data;
	char		namebuf[513];
	IFuid		*output_names, freq_name;
	int		bypass;
	int		type;

#ifndef notdef
	double		*save_states[8];
#ifdef notdef
	for (sg = sgen_init(ckt, 0); sg; sgen_next(&sg)) {
		if (sg->is_instparam)
			printf("%s:%s:%s -> param %s\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword);
		else
			printf("%s:%s:%s -> mparam %s\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword);
	}
#endif
#ifdef ASDEBUG
	DEBUG(1)
		printf(">>> restart : %d\n", restart);
#endif

	/* get to work */

	restart = 1;
	if (restart) {

		freq = 0.0;
		is_dc = (sen_info->step_type == SENS_DC);
		nfreqs = count_steps(sen_info->step_type, sen_info->start_freq,
			sen_info->stop_freq, sen_info->n_freq_steps,
			&step_size);

		if (!is_dc)
			freq = sen_info->start_freq;

		error = CKTop(ckt,
			(ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITJCT,
			(ckt->CKTmode & MODEUIC) | MODEDCOP | MODEINITFLOAT,
			ckt->CKTdcMaxIter);

#ifdef notdef
		ckt->CKTmode = (ckt->CKTmode & MODEUIC)
			| MODEDCOP | MODEINITSMSIG;
#endif
		if (error)
			return error;

		size = spGetSize(ckt->CKTmatrix, 1);

		/* Create the perturbation matrix */
		/* XXX check error return, '1' is complex -- necessary?
		 * only in ac */
		delta_Y = spCreate(size, !is_dc, &error);

		size += 1;

		/* Create an extra rhs */
		delta_I = NEWN(double, size);
		delta_iI = NEWN(double, size);

		delta_I_delta_Y = NEWN(double, size);
		delta_iI_delta_Y = NEWN(double, size);


		num_vars = 0;
		for (sg = sgen_init(ckt, is_dc); sg; sgen_next(&sg)) {
			num_vars += 1;
		}

		if (!num_vars)
			return OK;	/* XXXX Should be E_ something */

		k = 0;
		output_names = NEWN(IFuid, num_vars);
		for (sg = sgen_init(ckt, is_dc); sg; sgen_next(&sg)) {
			if (!sg->is_instparam) {
				sprintf(namebuf, "%s:%s",
					sg->instance->GENname,
					sg->ptable[sg->param].keyword);
			} else if ((sg->ptable[sg->param].dataType
				& IF_PRINCIPAL) && sg->is_principle == 1)
			{
				sprintf(namebuf, "%s", sg->instance->GENname);
			} else {
				sprintf(namebuf, "%s_%s",
					sg->instance->GENname,
					sg->ptable[sg->param].keyword);
			}

			(*SPfrontEnd->IFnewUid)((GENERIC *) ckt,
				output_names + k, NULL,
				namebuf, UID_OTHER, NULL);
			k += 1;
		}

		if (is_dc) {
			type = IF_REAL;
			freq_name = NULL;
		} else {
			type = IF_COMPLEX;
			(*SPfrontEnd->IFnewUid)((GENERIC *) ckt,
				&freq_name, NULL,
				"frequency", UID_OTHER, NULL);
		}

		error = (*SPfrontEnd->OUTpBeginPlot)((GENERIC *) ckt,
			(GENERIC *) ckt->CKTcurJob,
			ckt->CKTcurJob->JOBname, freq_name, IF_REAL, num_vars,
			output_names, type, (GENERIC **) &sen_data);
		if (error)
			return error;

		FREE(output_names);
		if (is_dc) {
			output_values = NEWN(double, num_vars);
			output_cvalues = NULL;
		} else {
			output_values = NULL;
			output_cvalues = NEWN(IFcomplex, num_vars);
			if (sen_info->step_type != SENS_LINEAR)
			    (*(SPfrontEnd->OUTattributes))((GENERIC *)sen_data,
				    NULL, OUT_SCALE_LOG, NULL);

		}

	} else {
		/*XXX Restore saved state */
	}

#ifdef ASDEBUG
	DEBUG(1)
		printf("start: %f, num: %d, dc: %d\n", freq, nfreqs, is_dc);
#endif

	if (!sen_info->output_volt)
		branch_eq = CKTfndBranch(ckt, sen_info->output_src);
	bypass = ckt->CKTbypass;
	ckt->CKTbypass = 0;

	/* The unknown vector of node voltages overwrites rhs */
	E = ckt->CKTrhs;
	iE = ckt->CKTirhs;
	ckt->CKTrhsOld = E;
	ckt->CKTirhsOld = iE;
	Y = ckt->CKTmatrix;
#ifdef ASDEBUG
	DEBUG(1) {
		printf("Operating point:\n");
		for (i = 0; i < size; i++)
			printf("          E [%d] = %20.15g\n", i, E[i]);
	}
#endif

#ifdef notdef
	for (j = 0; j <= ckt->CKTmaxOrder + 1; j++) {
		save_states[j] = ckt->CKTstates[j];
		ckt->CKTstates[j] = NULL;
	}
#endif

	for (i = 0; i < nfreqs; i++) {
		/* XXX handle restart */

		n = 0;

		if ((*SPfrontEnd->IFpauseTest)( )) {
			/* XXX Save State */
			return E_PAUSE;
		}

		for (j = 0; j < size; j++) {
			delta_I[j] = 0.0;
			delta_iI[j] = 0.0;
		}

		if (freq != 0.0) {
			ckt->CKTrhs = E;
			ckt->CKTirhs = iE;
			ckt->CKTmatrix = Y;

			/* This generates Y in LU form */
			ckt->CKTomega = 2.0 * M_PI * freq;

			/* Yes, all this has to be re-done */
			/* XXX Free old states */
			error = CKTunsetup(ckt);
			if (error)
				return error;

			/* XXX ckt->CKTmatrix = Y; */

			error = CKTsetup(ckt);
			if (error)
				return error;

			E = ckt->CKTrhs;
			iE = ckt->CKTirhs;
			Y = ckt->CKTmatrix;
#ifdef notdef
			for (j = 0; j <= ckt->CKTmaxOrder + 1; j++) {
				/* XXX Free new states */
				ckt->CKTstates[j] = save_states[j];
			}
#endif
			error = CKTtemp(ckt);
			if (error)
				return error;
			error = CKTload(ckt); /* INITSMSIGS */
			if (error)
				return error;
			error = NIacIter(ckt);
			if (error)
				return error;

#ifdef notdef
			/* XXX Why? */
			for (j = 0; j <= ckt->CKTmaxOrder + 1; j++) {
				ckt->CKTstates[j] = NULL;
			}
#endif

		}

		/* Use a different vector & matrix */
		ckt->CKTrhs = delta_I;
		ckt->CKTirhs = delta_iI;
		ckt->CKTmatrix = delta_Y;

		/* calc. effect of each param */
		for (sg = sgen_init(ckt, is_dc /* sen_info->plist */);
			sg; sgen_next(&sg))
		{

#ifdef ASDEBUG
			DEBUG(2) {
				printf("E/iE: %x/%x; delta_I/iI: %x/%x\n",
					E, iE, delta_I, delta_iI);
				printf("cktrhs/irhs: %x/%x\n",
					ckt->CKTrhs, ckt->CKTirhs);

				if (sg->is_instparam)
					printf("%s:%s:%s -> param %s\n",
					DEVices[sg->dev]->DEVpublic.name,
						sg->model->GENmodName,
						sg->instance->GENname,
						sg->ptable[sg->param].keyword);
				else
					printf("%s:%s:%s -> mparam %s\n",
					DEVices[sg->dev]->DEVpublic.name,
						sg->model->GENmodName,
						sg->instance->GENname,
						sg->ptable[sg->param].keyword);
			}
#endif

			spClear(delta_Y);

			for (j = 0; j < size; j++) {
				delta_I[j] = 0.0;
				delta_iI[j] = 0.0;
			}

			/* ? should this just call CKTsetup
			 * ? but then CKThead would have to get fiddled with */

			ckt->CKTnumStates = sg->istate;

			fn = DEVices[sg->dev]->DEVsetup;
			if (fn)
				(*fn)(delta_Y, sg->model, ckt,
					/* XXXX insert old state base here ?? */
					&ckt->CKTnumStates);

			/* ? CKTsetup would call NIreinit instead */
			ckt->CKTniState = NISHOULDREORDER | NIACSHOULDREORDER;

			/* XXX instead of calling temp here, just swap
			 * back to the original states */
			(void) sens_temp(sg, ckt);

			/* XXX Leave original E until here!! so that temp reads
			 * the right node voltages */

			if (sens_load(sg, ckt, is_dc)) {
				if (error && error != E_BADPARM)
					return error;	/* XXX */
				continue;
			}

			/* Alter the parameter */

#ifdef ASDEBUG
			DEBUG(1) printf("Original value: %g\n", sg->value);
#endif

#ifdef ASDEBUG
			DEBUG(2) {
				printf("Effect of device:\n");
				spPrint(delta_Y, 0, 1, 1);
				printf("LHS:\n");
				for (j = 0; j < size; j++)
					printf("%d: %g, %g\n", j,
						delta_I[j], delta_iI[j]);
			}
#endif

			if (sg->value != 0.0)
				delta_var = sg->value * Sens_Delta;
			else
				delta_var = Sens_Abs_Delta;

			nvalue.rValue = sg->value + delta_var;

#ifdef ASDEBUG
			DEBUG(1)
				printf("New value: %g\n", nvalue.rValue);
#endif

			sens_setp(sg, ckt, &nvalue);
			if (error && error != E_BADPARM)
				return error;

			spConstMult(delta_Y, -1.0);
			for (j = 0; j < size; j++) {
				delta_I[j] *= -1.0;
				delta_iI[j] *= -1.0;
			}

#ifdef ASDEBUG
			DEBUG(2) {
				printf("Effect of negating matrix:\n");
				spPrint(delta_Y, 0, 1, 1);
				for (j = 0; j < size; j++)
					printf("%d: %g, %g\n", j,
						delta_I[j], delta_iI[j]);
			}
#endif

			/* XXX swap back to temp states ??   Naw ... */
			(void) sens_temp(sg, ckt);

#ifdef ASDEBUG
			DEBUG(1) {
				if (sens_getp(sg, ckt, &value)) {
					continue;
				}

				printf("New value in device: %g\n",
					value.rValue);
			}
#endif

			sens_load(sg, ckt, is_dc);

#ifdef ASDEBUG
			DEBUG(2) {
				printf("Effect of changing the parameter:\n");
				spPrint(delta_Y, 0, 1, 1);
				for (j = 0; j < size; j++)
					printf("%d: %g, %g\n", j,
						delta_I[j], delta_iI[j]);
			}
#endif
			/* Set the perturbed variable back to it's
			 * original value
			 */

			value.rValue = sg->value;
			sens_setp(sg, ckt, &value);
			(void) sens_temp(sg, ckt); /* XXX is this necessary? */

			/* Back to business . . . */

#ifdef ASDEBUG
			DEBUG(2)
				for (j = 0; j < size; j++)
					printf("          E [%d] = %20.15g\n",
						j, E[j]);
#endif

			/* delta_Y E */
			spMultiply(delta_Y, delta_I_delta_Y, E,
				delta_iI_delta_Y, iE);

#ifdef ASDEBUG
			DEBUG(2)
				for (j = 0; j < size; j++)
					printf("delta_Y * E [%d] = %20.15g\n",
						j, delta_I_delta_Y[j]);
#endif

			/* delta_I - delta_Y E */
			for (j = 0; j < size; j++) {
				delta_I[j] -= delta_I_delta_Y[j];
				delta_iI[j] -= delta_iI_delta_Y[j];
			}

#ifdef ASDEBUG
			DEBUG(2) {
				printf(">>> Y:\n");
				spPrint(Y, 0, 1, 1);
				for (j = 0; j < size; j++)
					printf("%d: %g, %g\n", j,
						delta_I[j], delta_iI[j]);
			}
#endif
			/* Solve; Y already factored */
			spSolve(Y, delta_I, delta_I, delta_iI, delta_iI);

#ifdef ASDEBUG
			DEBUG(2) {
				for (j = 1; j < size; j++) {

					if (sg->is_instparam)
						printf("%d/%s.%s = %g, %g\n",
							j,
							sg->instance->GENname,
						sg->ptable[sg->param].keyword,
						delta_I[j], delta_iI[j]);
					else
						printf("%d/%s:%s = %g, %g\n",
							j,
							sg->instance->GENname,
						sg->ptable[sg->param].keyword,
						delta_I[j], delta_iI[j]);

				}
			}
#endif

			/* delta_I is now equal to delta_E */

			if (is_dc) {
				if (sen_info->output_volt)
					output_values[n] = delta_I
						[sen_info->output_pos->number]
						- delta_I
						[sen_info->output_neg->number];
				else {
					output_values[n] = delta_I[branch_eq];
				}
				output_values[n] /= delta_var;
			} else {
				if (sen_info->output_volt) {
					output_cvalues[n].real = delta_I
						[sen_info->output_pos->number]
						- delta_I
						[sen_info->output_neg->number];
					output_cvalues[n].imag = delta_iI
						[sen_info->output_pos->number]
						- delta_iI
						[sen_info->output_neg->number];
				} else {
					output_cvalues[n].real =
						delta_I[branch_eq];
					output_cvalues[n].imag =
						delta_iI[branch_eq];
				}
				output_cvalues[n].real /= delta_var;
				output_cvalues[n].imag /= delta_var;
			}

			n += 1;

		}

		if (is_dc)
			nvalue.v.vec.rVec = output_values;
		else
			nvalue.v.vec.cVec = output_cvalues;

		value.rValue = freq;
		OUTpData(sen_data, &value, &nvalue);
		freq = inc_freq(freq, sen_info->step_type, step_size);

	}

	(*SPfrontEnd->OUTendPlot)((GENERIC *) sen_data);

	if (is_dc) {
		FREE(output_values);	/* XXX free various vectors */
	} else {
		FREE(output_cvalues);	/* XXX free various vectors */
	}

	spDestroy(delta_Y);
	FREE(delta_I);
	FREE(delta_iI);

	ckt->CKTrhs = E;
	ckt->CKTirhs = iE;
	ckt->CKTmatrix = Y;
	ckt->CKTbypass = bypass;

#ifdef notdef
	for (j = 0; j <= ckt->CKTmaxOrder + 1; j++) {
		if (ckt->CKTstates[j])
			FREE(ckt->CKTstates[j]);
		ckt->CKTstates[j] = save_states[j];
	}
#endif
#endif

	return OK;
}

double
inc_freq(freq, type, step_size)
	double	freq;
	int	type;
	double	step_size;
{
	if (type != LINEAR)
		freq *= step_size;
	else
		freq += step_size;

	return freq;
}

double
next_freq(type, freq, stepsize)
	int	type;
	double	freq, stepsize;
{
	double	s;

	switch (type) {
	case SENS_DC:
		s = 0;
		break;

	case SENS_LINEAR:
		s = freq + stepsize;
		break;

	case SENS_DECADE:
	case SENS_OCTAVE:
		s = freq * stepsize;
		break;
	}
	return s;
}

int
count_steps(type, low, high, steps, stepsize)
	int	type;
	double	low, high;
	int	steps;
	double	*stepsize;
{
	double	s;
	int	n;

	if (steps < 1)
		steps = 1;

	switch (type) {
	default:
	case SENS_DC:
		n = 0;
		s = 0;
		break;

	case SENS_LINEAR:
		n = steps;
		s = (high - low) / steps;
		break;

	case SENS_DECADE:
		if (low <= 0.0)
			low = 1e-3;
		if (high <= low)
			high = 10.0 * low;
		n = steps * log10(high/low) + 1.01;
		s = pow(10.0, 1.0 / steps);
		break;

	case SENS_OCTAVE:
		if (low <= 0.0)
			low = 1e-3;
		if (high <= low)
			high = 2.0 * low;
		n = steps * log(high/low) / M_LOG2E + 1.01;
		s = pow(2.0, 1.0 / steps);
		break;
	}

	if (n <= 0)
		n = 1;

	*stepsize = s;
	return n;
}

static int
sens_load(sg, ckt, is_dc)
	CKTcircuit	*ckt;
	sgen		*sg;
	int		is_dc;
{
	int	(*fn)( );

	error = 0;

	if (!is_dc)
		fn = DEVices[sg->dev]->DEVacLoad;
	else
		fn = DEVices[sg->dev]->DEVload;

	if (fn)
		error = (*fn)(sg->model, ckt);
	else
		return 1;

	return error;
}


static int
sens_temp(sg, ckt)
	CKTcircuit	*ckt;
	sgen		*sg;
{
	int	(*fn)( );

	error = 0;

	fn = DEVices[sg->dev]->DEVtemperature;

	if (fn)
		error = (*fn)(sg->model, ckt);
	else
		return 1;

	return error;
}

/* Get parameter value */
int
sens_getp(sg, ckt, val)
	CKTcircuit	*ckt;
	sgen		*sg;
	IFvalue		*val;
{
	int	(*fn)( );
	int	pid;

	error = 0;

	if (sg->is_instparam) {
		fn = DEVices[sg->dev]->DEVask;
		pid = DEVices[sg->dev]->DEVpublic.instanceParms[sg->param].id;
		if (fn)
			error = (*fn)(ckt, sg->instance, pid, val, NULL);
		else
			return 1;
	} else {
		fn = DEVices[sg->dev]->DEVmodAsk;
		pid = DEVices[sg->dev]->DEVpublic.modelParms[sg->param].id;
		if (fn)
			error = (*fn)(ckt, sg->model, pid, val, NULL);
		else
			return 1;
	}

	if (error) {
		if (sg->is_instparam)
			printf("GET ERROR: %s:%s:%s -> param %s (%d)\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword, pid);
		else
			printf("GET ERROR: %s:%s:%s -> mparam %s (%d)\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword, pid);
	}

	return error;
}

/* Get parameter value */
static int
sens_setp(sg, ckt, val)
	sgen		*sg;
	CKTcircuit	*ckt;
	IFvalue		*val;
{
	int	(*fn)( );
	int	pid;

	error = 0;

	if (sg->is_instparam) {
		fn = DEVices[sg->dev]->DEVparam;
		pid = DEVices[sg->dev]->DEVpublic.instanceParms[sg->param].id;
		if (fn)
			error = (*fn)(pid, val, sg->instance, NULL);
		else
			return 1;
	} else {
		fn = DEVices[sg->dev]->DEVmodParam;
		pid = DEVices[sg->dev]->DEVpublic.modelParms[sg->param].id;
		if (fn)
			error = (*fn)(pid, val, sg->model);
		else
			return 1;
	}

	if (error) {
		if (sg->is_instparam)
			printf("SET ERROR: %s:%s:%s -> param %s (%d)\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword, pid);
		else
			printf("SET ERROR: %s:%s:%s -> mparam %s (%d)\n",
				DEVices[sg->dev]->DEVpublic.name,
				sg->model->GENmodName,
				sg->instance->GENname,
				sg->ptable[sg->param].keyword, pid);
	}

	return error;
}
