/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2017 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <cyaml.h>

#include "ttest.h"

#define UNUSED(_x) ((void)(_x))

static bool test_free_null_data(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	cyaml_err_t err;
	struct target_struct {
		int test_value_int;
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("test_int", CYAML_FLAG_DEFAULT,
				struct target_struct, test_value_int),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	ttest_ctx_t tc = ttest_start(report, __func__, NULL, NULL);

	err = cyaml_free(config, &top_schema, NULL);
	if (err != CYAML_OK) {
		return ttest_fail(&tc, "Free failed: %s", cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

static bool test_free_null_config(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	cyaml_err_t err;
	ttest_ctx_t tc = ttest_start(report, __func__, NULL, NULL);

	UNUSED(config);

	err = cyaml_free(NULL, NULL, NULL);
	if (err != CYAML_ERR_BAD_PARAM_NULL_CONFIG) {
		return ttest_fail(&tc, "Free failed: %s", cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

static bool test_free_null_schema(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	cyaml_err_t err;
	ttest_ctx_t tc = ttest_start(report, __func__, NULL, NULL);

	err = cyaml_free(config, NULL, NULL);
	if (err != CYAML_ERR_BAD_PARAM_NULL_SCHEMA) {
		return ttest_fail(&tc, "Free failed: %s", cyaml_strerror(err));
	}

	return ttest_pass(&tc);
}

bool free_tests(
		ttest_report_ctx_t *rc,
		cyaml_log_t log_level,
		cyaml_log_fn_t log_fn)
{
	bool pass = true;
	cyaml_config_t config = {
		.log_fn = log_fn,
		.log_level = log_level,
		.flags = CYAML_CFG_DEFAULT,
	};

	ttest_heading(rc, "Free tests");

	pass &= test_free_null_data(rc, &config);
	pass &= test_free_null_config(rc, &config);
	pass &= test_free_null_schema(rc, &config);

	return pass;
}