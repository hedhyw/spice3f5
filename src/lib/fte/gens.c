#include "spice.h"
#include "util.h"
#include "misc.h"
#include "ifsim.h"
#include "gendefs.h"
#include "cktdefs.h"
#include "cpstd.h"
#include "ftedefs.h"
#include "fteext.h"
#include "devdefs.h"
#include "dgen.h"

static void dgen_next( );

wl_forall(wl, fn, data)
	wordlist	*wl;
	int		(*fn)( );
	char		*data;
{
	while (wl) {
		(*fn)(wl, data);
		wl = wl->wl_next;
	}
}

dgen *
dgen_init(ckt, wl, nomix, flag, model)
	GENcircuit	*ckt;
	wordlist	*wl;
	int		nomix;
	int		flag;
	int		model;
{
	dgen	*dg;
	wordlist *w, **prevp;

	dg = NEW(dgen);
	dg->ckt = ckt;
	dg->instance = NULL;
	dg->model = NULL;
	dg->dev_type_no = -1;
	dg->dev_list = wl;
	dg->flags = 0;

	prevp = &wl;

#ifdef notdef
	for (w = wl; w; w = w->wl_next) {
		if (!strcmp(w->wl_word, "#")) {
			model = 1;
			*prevp = w->wl_next;
			flag |= DGEN_DEFDEVS;
		} else if (index(w->wl_word, '#'))
			model = 1;
		else
			instance = 1;
		prevp = &w->wl_next;
	}

	if (instance && model) {
		fprintf(stderr,
			"Error: can't mix instances and models");
		tfree(dg);
		return NULL;
	}
#endif

	if (model)
		dg->flags = (DGEN_ALL & ~ DGEN_INSTANCE) | DGEN_INIT;
	else
		dg->flags = DGEN_ALL | DGEN_INIT;

	if (wl)
		dg->flags |= flag;
	else
		dg->flags |= DGEN_DEFDEVS | flag;

	dgen_next(&dg);

	return dg;
}

int
dgen_for_n(dg, n, fn, data, subindex)
	dgen	*dg;
	int	n;
	int	(*fn)( );
	char	*data;
	int	subindex;
{
	dgen	dgx, *dgxp;
	int	dnum, i, j, k;

	dgxp = &dgx;
	bcopy(dg, dgxp, sizeof(dgx));

	dnum = dgxp->dev_type_no;

	k = 0;
	for (i = 0; dgxp && dgxp->dev_type_no == dnum && i < n; i++) {
		/*printf("Loop at %d\n", i);*/
		j = (*fn)(dgxp, data, subindex);
		if (j > k)
			k = j;
		dgen_next(&dgxp);
	}

	return k - subindex;
}

dgen_nth_next(dg, n)
	dgen	**dg;
	int	n;
{
	int	i, dnum;

	dnum = (*dg)->dev_type_no;

	for (i = 0; *dg && (*dg)->dev_type_no == dnum && i < n; i++) {
		dgen_next(dg);
	}
}

static void
dgen_next(dgx)
	dgen	**dgx;
{
	int	done;
	dgen	*dg;
	char	*p;
	int	len, len2;
	int	need;
	int	i;
	wordlist *w;
	char	type, *subckt, *device, *model;
	char	*Top_Level = "\001";
	int	subckt_len;
	int	head_match;
	char	*word, *dev_name, *mod_name;

	dg = *dgx;
	if (!dg)
		return;

	/* Prime the "model only" or "device type only" iteration,
	 * required because the filtering (below) may request additional
	 * detail.
	 */
	if (!(dg->flags & DGEN_INSTANCE)) {
		if (!(dg->flags & DGEN_MODEL))
			dg->model = NULL;
		dg->instance = NULL;
	}

	need = dg->flags;
	done = 0;

	while (!done) {

		if (dg->instance) {
			/* next instance */
			dg->instance = dg->instance->GENnextInstance;
		} else if (dg->model) {
			dg->model = dg->model->GENnextModel;
			if (dg->model)
				dg->instance = dg->model->GENinstances;
		} else if (dg->dev_type_no < DEVmaxnum) {
			dg->dev_type_no += 1;
			if (dg->dev_type_no < DEVmaxnum) {
				dg->model = ((CKTcircuit *)
					(dg->ckt))->CKThead[dg->dev_type_no];
				if (dg->model)
					dg->instance = dg->model->GENinstances;
			} else {
				done = 2;
				break;
			}
		} else {
			done = 2;
			break;
		}

		if (need & DGEN_INSTANCE && !dg->instance)
			continue;
		if (need & DGEN_MODEL && !dg->model)
			continue;

#ifdef notdef
		if (dg->instance)
			printf("Maybe : %s\n", dg->instance->GENname);
		if (dg->model)
			printf("Maybe mod : %s\n", dg->model->GENmodName);
#endif

		/* Filter */

		if (!dg->dev_list) {
			if ((dg->flags & DGEN_ALLDEVS)
				|| ((dg->flags & DGEN_DEFDEVS)
				&& (ft_sim->devices[dg->dev_type_no]->flags
				& DEV_DEFAULT)))
			{
				done = 1;
			} else {
				done = 0;
			}
			continue;
		}

		done = 0;

		for (w = dg->dev_list; w && !done; w = w->wl_next) {

			/* assume a match (have to reset done every time
			 * through
			 */
			done = 1;
			word = w->wl_word;

			if (!word || !*word) {
				break;
			}

			/* Break up word into type, subcircuit, model, device,
			 * must be nodestructive to "word"
			 */

			/* type */
			if (*word == ':' || *word == '#')
				type = '\0';
			else
				type = *word++;

			/* subcircuit */

			subckt = word;
			/* look for last ":" or "#" in word */
			for (p = word + strlen(word) /* do '\0' first time */;
				p != word && *p != ':' && *p != '#'; p--)
			{
				;
			}

			if (*p != ':' && *p != '#') {
				/* No subcircuit name specified */
				subckt = NULL;
				subckt_len = 0;
			} else {

				if (p[-1] == ':') {
					head_match = 1;
					subckt_len = p - word - 1;
				} else {
					head_match = 0;
					subckt_len = p - word;
				}

				if (subckt_len == 0) {
					/* Top level only */
					if (head_match)
						subckt = NULL;
					else
						subckt = Top_Level;
				}
				word = p + 1;
			}

			/* model or device */

			if (*p == '#') {
				model = word;
				device = NULL;
			} else {
				model = NULL;
				device = word;
			}

			/* Now compare */
#ifdef notdef
			printf("Type: %c, subckt: %s, name: %s\n",
				type ? type : '0', subckt, device);
#endif

			if (dg->instance)
				dev_name = dg->instance->GENname;
			else
				dev_name = NULL;

			if (dg->model)
				mod_name = dg->model->GENmodName;
			else
				mod_name = NULL;

			if (type) {
				if (!dev_name) {
					done = 0;
					/*printf("No device.\n");*/
					need |= DGEN_MODEL;
					continue;
				} else if (type != *dev_name) {
					done = 0;
					/*printf("Wrong type.\n");*/
					/* Bleh ... plan breaks down here */
					/* need = DGEN_TYPE; */
					continue;
				}
			}

			if (subckt == Top_Level) {
				if (dev_name && dev_name[1] == ':') {
					need |= DGEN_INSTANCE;
					done = 0;
					/*printf("Wrong level.\n");*/
					continue;
				}
			} else if (subckt && (!dev_name
				|| !ciprefix(subckt, dev_name + 1)))
			{
				need |= DGEN_INSTANCE;
				done = 0;
				/*printf("Wrong subckt.\n"); */
				continue;
			}

			if (device && *device) {
				need |= DGEN_INSTANCE | DGEN_MODEL;
				if (!dev_name) {
					done = 0;
					/*printf("Didn't get dev name.\n");*/
					continue;
				} else if (strcmp(device,
					dev_name + 1 + subckt_len))
				{
					done = 0;
					/*printf("Wrong name.\n");*/
					continue;
				}
			} else if (model && *model) {
				if (strcmp(model, mod_name)) {
					done = 0;
					need |= DGEN_MODEL;
					/*printf("Wrong model name.\n");*/
					continue;
				}
			}

			break;
		}

#ifdef notdef
		if (done == 1)
			printf("Accepted\n");
		else
			printf("Skipped\n");
#endif

	}

	if (done == 2)
		*dgx = NULL;
}
