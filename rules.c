/*	rules.c

	Evaluation of configurations....
*/

#include "rules.h"
#include "cgic.h"

static void eval_design(void);

int cgiMain()
{
  	cgiHeaderContentType("text/html");
	get_cgi_args();
	if (cdr_op == MORE_OPTS || cdr_op == SET_PARAMS) {
		/* we've got a database from the input to save */
		get_cgi_db_args();
		save_db();
	} else {
		if (cgiLoadDB(paramset) != cgiFormSuccess) {
			err_exit("Cannot load database file");
		}
		get_cgi_db_args();
	}

	switch (cdr_op) {
	case MORE_OPTS:
		/* the parameter entry form always adds some extra
		 * blank entries up the max number, so saving the
		 * parameter set, as we've done above, and reloading
		 * the edit form, as will happen when we fall through
		 * to the EDIT_PARAMS case below, will add more entries
		 */
	case EDIT_PARAMS:
		edit_params();
		break;
	case EVAL_DESIGN:
		eval_design();
		break;
	case VIEW_PARAMS:
		print_params();
		break;
	case NEW_USER_FORM:
		new_user_form();
		break;	
	case NEW_USER:
		/* create new user id then go to logged in eval page */
		g_uid = new_user();
		/* touch the db file so that we can open it when we
		 * print the list of db's in print_eval_form
		 */
		touch_db(g_uid);
	case SET_PARAMS:
		/* params were saved above, now allow them to use it */
	case LOGIN:
		/* validated login when checking args, print eval page */
		if (g_uid < 0) {
			invalid_login();
			break;
		}
	case PRINT_FORM:
	default:
		print_eval_form();
		break;
	}

	return(0);
}

static void eval_design(void) {
	if (oeval || (strcmp(email, dbemail) == 0)) {
		eval();
		out_html();
	} else {
		eval_denied();
	}
}
