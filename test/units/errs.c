/*
 * SPDX-License-Identifier: ISC
 *
 * Copyright (C) 2018 Michael Drake <tlsa@netsurf-browser.org>
 */

#include <stdbool.h>
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <cyaml.h>

#include "ttest.h"

#define UNUSED(_x) ((void)(_x))
#define YAML_LEN(_y) (sizeof(_y) - 1)

typedef struct test_data {
	cyaml_data_t **data;
	unsigned *seq_count;
	const struct cyaml_config *config;
	const struct cyaml_schema_type *schema;
} test_data_t;

/* Common cleanup function to free data loaded by tests. */
static void cyaml_cleanup(void *data)
{
	struct test_data *td = data;
	unsigned seq_count = 0;

	if (td->seq_count != NULL) {
		seq_count = *(td->seq_count);
	}

	cyaml_free(td->config, td->schema, *(td->data), seq_count);
}

/* Test loading with NULL data parameter. */
static bool test_err_load_null_data(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] = "";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) NULL, NULL);
	if (err != CYAML_ERR_BAD_PARAM_NULL_DATA) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with NULL config parameter. */
static bool test_err_load_null_config(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] = "";
	void *data_tgt = NULL;
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = NULL,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), NULL, NULL,
			(cyaml_data_t **) NULL, NULL);
	if (err != CYAML_ERR_BAD_PARAM_NULL_CONFIG) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with NULL memory allocation function. */
static bool test_err_load_null_mem_fn(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] = "";
	cyaml_config_t cfg = *config;
	void *data_tgt = NULL;
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = &cfg,
		.schema = NULL,
	};
	cyaml_err_t err;

	cfg.mem_fn = NULL;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), &cfg, NULL,
			(cyaml_data_t **) NULL, NULL);
	if (err != CYAML_ERR_BAD_CONFIG_NULL_MEMFN) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with NULL schema. */
static bool test_err_load_null_schema(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] = "";
	void *data_tgt = NULL;
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = NULL,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, NULL,
			(cyaml_data_t **) NULL, NULL);
	if (err != CYAML_ERR_BAD_PARAM_NULL_SCHEMA) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with bad top level type (non-pointer). */
static bool test_err_schema_top_level_non_pointer(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"7\n";
	int *value = NULL;
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int)
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &value,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &value, NULL);
	if (err != CYAML_ERR_TOP_LEVEL_NON_PTR) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (value != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with bad top level sequence and no seq_count. */
static bool test_err_schema_top_level_sequence_no_count(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n";
	struct target_struct {
		int *value;
		unsigned value_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int)
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_SEQUENCE(CYAML_FLAG_POINTER, int,
				&entry_schema, 0, CYAML_UNLIMITED),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_BAD_PARAM_SEQ_COUNT) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with bad top level sequence and no seq_count. */
static bool test_err_schema_top_level_not_sequence_count(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"7\n";
	int *value = NULL;
	unsigned count = 0;
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int)
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &value,
		.seq_count = &count,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &value, &count);
	if (err != CYAML_ERR_BAD_PARAM_SEQ_COUNT) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (value != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with bad type. */
static bool test_err_schema_bad_type(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = 99999,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = sizeof(data_tgt->value),
			},
			.data_offset = offsetof(struct target_struct, value),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_BAD_TYPE_IN_SCHEMA) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with string min greater than max. */
static bool test_err_schema_string_min_max(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"value: foo\n";
	struct target_struct {
		const char *value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_STRING_PTR("value", CYAML_FLAG_POINTER,
				struct target_struct, value,
				10, 9),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_BAD_MIN_MAX_SCHEMA) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (0). */
static bool test_err_schema_bad_data_size_1(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: 1\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_INT,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = 0,
			},
			.data_offset = offsetof(struct target_struct, value),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (9). */
static bool test_err_schema_bad_data_size_2(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: 1\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_INT,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = 9,
			},
			.data_offset = offsetof(struct target_struct, value),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (0) for flags. */
static bool test_err_schema_bad_data_size_3(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const char * const strings[] = {
		"foo",
		"bar",
		"baz",
		"bat",
	};
	static const unsigned char yaml[] =
		"key:\n"
		"  - bat\n"
		"  - bar\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_FLAGS,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = 0,
				.enumeration = {
					.strings = strings,
					.count = CYAML_ARRAY_LEN(strings),
				},
			},
			.data_offset = offsetof(struct target_struct, value),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (0) for sequence count. */
static bool test_err_schema_bad_data_size_4(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 2\n"
		"  - 4\n";
	struct target_struct {
		int *value;
		unsigned value_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_SEQUENCE,
				.flags = CYAML_FLAG_POINTER,
				.data_size = sizeof(*(data_tgt->value)),
				.sequence = {
					.min = 0,
					.max = CYAML_UNLIMITED,
					.schema = &entry_schema,
				},
			},
			.data_offset = offsetof(struct target_struct, value),
			.count_size = 0,
			.count_offset = offsetof(
					struct target_struct,
					value_count),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (9) for sequence count. */
static bool test_err_schema_bad_data_size_5(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 2\n"
		"  - 4\n";
	struct target_struct {
		int *value;
		unsigned value_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_SEQUENCE,
				.flags = CYAML_FLAG_POINTER,
				.data_size = sizeof(*(data_tgt->value)),
				.sequence = {
					.min = 0,
					.max = CYAML_UNLIMITED,
					.schema = &entry_schema,
				},
			},
			.data_offset = offsetof(struct target_struct, value),
			.count_size = 9,
			.count_offset = offsetof(
					struct target_struct,
					value_count),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size (9) for sequence count. */
static bool test_err_schema_bad_data_size_6(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 2\n"
		"  - 4\n";
	struct target_struct {
		int value[4];
		unsigned value_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_SEQUENCE,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = sizeof(*(data_tgt->value)),
				.sequence = {
					.min = 0,
					.max = 4,
					.schema = &entry_schema,
				},
			},
			.data_offset = offsetof(struct target_struct, value),
			.count_size = 9,
			.count_offset = offsetof(
					struct target_struct,
					value_count),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with sequence fixed with unequal min and max. */
static bool test_err_schema_sequence_min_max(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"sequence:\n"
		"    - \n";
	struct target_struct {
		unsigned *seq;
		uint32_t seq_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_UINT(CYAML_FLAG_DEFAULT, *(data_tgt->seq)),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "sequence",
			.value = {
				.type = CYAML_SEQUENCE_FIXED,
				.flags = CYAML_FLAG_POINTER,
				.data_size = sizeof(*(data_tgt->seq)),
				.sequence = {
					.schema = &entry_schema,
					.min = 0,
					.max = CYAML_UNLIMITED,

				},
			},
			.data_offset = offsetof(struct target_struct, seq),
			.count_offset = offsetof(struct target_struct, seq_count),
			.count_size = sizeof(data_tgt->seq_count),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_SEQUENCE_FIXED_COUNT) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with data size for float. */
static bool test_err_schema_bad_data_size_float(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: 1\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		{
			.key = "key",
			.value = {
				.type = CYAML_FLOAT,
				.flags = CYAML_FLAG_DEFAULT,
				.data_size = 7,
			},
			.data_offset = offsetof(struct target_struct, value),
		},
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_DATA_SIZE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading with schema with sequence in sequence. */
static bool test_err_schema_sequence_in_sequence(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"- -\n";
	unsigned **seq = NULL;
	unsigned count = 0;
	static const struct cyaml_schema_type inner_entry_schema = {
		CYAML_TYPE_UINT(CYAML_FLAG_DEFAULT, **seq),
	};
	static const struct cyaml_schema_type outer_entry_schema = {
		CYAML_TYPE_SEQUENCE(CYAML_FLAG_POINTER, unsigned,
				&inner_entry_schema, 0, CYAML_UNLIMITED)
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_SEQUENCE(CYAML_FLAG_POINTER, unsigned *,
				&outer_entry_schema, 0, CYAML_UNLIMITED)
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &seq,
		.seq_count = &count,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &seq, &count);
	if (err != CYAML_ERR_SEQUENCE_IN_SEQUENCE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (seq != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects unit, but value is invalid. */
static bool test_err_schema_invalid_value_unit(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: scalar\n";
	struct target_struct {
		unsigned a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects float but value is out of range. */
static bool test_err_schema_invalid_value_float_range(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 3.5e+38\n";
	struct target_struct {
		float a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLOAT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects double but value is out of range. */
static bool test_err_schema_invalid_value_double_range(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 1.8e+308\n";
	struct target_struct {
		double a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLOAT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects uint but value is out of range. */
static bool test_err_schema_invalid_value_unit_range_1(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: -1\n";
	struct target_struct {
		unsigned char a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects uint but value is out of range. */
static bool test_err_schema_invalid_value_unit_range_2(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 256\n";
	struct target_struct {
		unsigned char a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects uint but value is out of range. */
static bool test_err_schema_invalid_value_unit_range_3(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 0x10000\n";
	struct target_struct {
		uint16_t a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects uint but value is out of range. */
static bool test_err_schema_invalid_value_unit_range_4(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 0x100000000\n";
	struct target_struct {
		uint32_t a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects uint but value is out of range. */
static bool test_err_schema_invalid_value_unit_range_5(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 0x10000000000000000\n";
	struct target_struct {
		uint32_t a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_UINT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects string, but it's too short. */
static bool test_err_schema_string_min_length(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: foo\n";
	struct target_struct {
		char *a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_STRING_PTR("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a, 4, 4),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_STRING_LENGTH_MIN) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects string, but it's too long. */
static bool test_err_schema_string_max_length(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: fifth\n";
	struct target_struct {
		char *a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_STRING_PTR("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a, 4, 4),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_STRING_LENGTH_MAX) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects mapping field which is not present. */
static bool test_err_schema_missing_mapping_field(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"a: 2\n";
	struct target_struct {
		int a;
		int b;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("a", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_INT("b", CYAML_FLAG_DEFAULT,
				struct target_struct, b),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_MAPPING_FIELD_MISSING) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema disallows mapping field. */
static bool test_err_schema_unknown_mapping_field(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"wrong_key: 2\n";
	struct target_struct {
		int a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("key", CYAML_FLAG_DEFAULT,
				struct target_struct, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_KEY) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects sequence, it has too few entries. */
static bool test_err_schema_sequence_min_entries(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 1\n"
		"  - 2\n";
	struct target_struct {
		int *a;
		unsigned a_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, *(data_tgt->a)),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("key", CYAML_FLAG_POINTER,
				struct target_struct, a, &entry_schema,
				3, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_SEQUENCE_ENTRIES_MIN) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects sequence, it has too many entries. */
static bool test_err_schema_sequence_max_entries(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 1\n"
		"  - 2\n"
		"  - 3\n";
	struct target_struct {
		int *a;
		unsigned a_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, *(data_tgt->a)),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("key", CYAML_FLAG_POINTER,
				struct target_struct, a, &entry_schema,
				2, 2),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_SEQUENCE_ENTRIES_MAX) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects flags and finds a mapping inside. */
static bool test_err_schema_flags_mapping(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	enum test_flags {
		TEST_FLAGS_NONE   = 0,
		TEST_FLAGS_FIRST  = (1 << 0),
		TEST_FLAGS_SECOND = (1 << 1),
		TEST_FLAGS_THIRD  = (1 << 2),
		TEST_FLAGS_FOURTH = (1 << 3),
		TEST_FLAGS_FIFTH  = (1 << 4),
		TEST_FLAGS_SIXTH  = (1 << 5),
	};
	static const char * const strings[] = {
		"first",
		"second",
		"third",
		"fourth",
		"fifth",
		"sixth",
	};
	static const unsigned char yaml[] =
		"key:\n"
		"    - first\n"
		"    - map:\n"
		"        a:\n"
		"        b:\n";
	struct target_struct {
		enum test_flags a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLAGS("key", CYAML_FLAG_STRICT,
				struct target_struct, a,
				strings, CYAML_ARRAY_LEN(strings)),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_UNEXPECTED_EVENT) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects enum, but string is not allowed. */
static bool test_err_schema_enum_bad_string(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	enum test_enum {
		TEST_ENUM_FIRST,
		TEST_ENUM_SECOND,
		TEST_ENUM_THIRD,
		TEST_ENUM__COUNT,
	};
	static const char * const strings[TEST_ENUM__COUNT] = {
		[TEST_ENUM_FIRST]  = "first",
		[TEST_ENUM_SECOND] = "second",
		[TEST_ENUM_THIRD]  = "third",
	};
	static const unsigned char yaml[] =
		"key: fourth\n";
	struct target_struct {
		enum test_enum a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_ENUM("key", CYAML_FLAG_DEFAULT,
				struct target_struct, a,
				strings, TEST_ENUM__COUNT),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects flags but YAML has bad flag string. */
static bool test_err_schema_flags_bad_string(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	enum test_flags {
		TEST_FLAGS_NONE   = 0,
		TEST_FLAGS_FIRST  = (1 << 0),
		TEST_FLAGS_SECOND = (1 << 1),
		TEST_FLAGS_THIRD  = (1 << 2),
		TEST_FLAGS_FOURTH = (1 << 3),
		TEST_FLAGS_FIFTH  = (1 << 4),
		TEST_FLAGS_SIXTH  = (1 << 5),
	};
	static const char * const strings[] = {
		"first",
		"second",
		"third",
		"fourth",
		"fifth",
		"sixth",
	};
	static const unsigned char yaml[] =
		"key:\n"
		"    - first\n"
		"    - seventh\n";
	struct target_struct {
		enum test_flags a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLAGS("key", CYAML_FLAG_DEFAULT,
				struct target_struct, a,
				strings, CYAML_ARRAY_LEN(strings)),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects strict enum but YAML has bad string. */
static bool test_err_schema_strict_enum_bad_string(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	enum test_enum {
		TEST_ENUM_FIRST,
		TEST_ENUM_SECOND,
		TEST_ENUM_THIRD,
		TEST_ENUM__COUNT,
	};
	static const char * const strings[TEST_ENUM__COUNT] = {
		[TEST_ENUM_FIRST]  = "first",
		[TEST_ENUM_SECOND] = "second",
		[TEST_ENUM_THIRD]  = "third",
	};
	static const unsigned char yaml[] =
		"key: fourth\n";
	struct target_struct {
		enum test_enum a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_ENUM("key", CYAML_FLAG_STRICT,
				struct target_struct, a,
				strings, TEST_ENUM__COUNT),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects strict flags but YAML has bad string. */
static bool test_err_schema_strict_flags_bad_string(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	enum test_flags {
		TEST_FLAGS_NONE   = 0,
		TEST_FLAGS_FIRST  = (1 << 0),
		TEST_FLAGS_SECOND = (1 << 1),
		TEST_FLAGS_THIRD  = (1 << 2),
		TEST_FLAGS_FOURTH = (1 << 3),
		TEST_FLAGS_FIFTH  = (1 << 4),
		TEST_FLAGS_SIXTH  = (1 << 5),
	};
	static const char * const strings[] = {
		"first",
		"second",
		"third",
		"fourth",
		"fifth",
		"sixth",
	};
	static const unsigned char yaml[] =
		"key:\n"
		"    - first\n"
		"    - seventh\n";
	struct target_struct {
		enum test_flags a;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLAGS("key", CYAML_FLAG_STRICT,
				struct target_struct, a,
				strings, CYAML_ARRAY_LEN(strings)),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects int, but YAML has sequence. */
static bool test_err_schema_expect_int_read_seq(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n"
		"  - 90";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("key", CYAML_FLAG_DEFAULT,
				struct target_struct, value),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects int, but YAML ends. */
static bool test_err_schema_expect_int_read_end_1(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key:\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("key", CYAML_FLAG_DEFAULT,
				struct target_struct, value),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects int, but YAML ends. */
static bool test_err_schema_expect_int_read_end_2(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: |"
		"...";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_INT("key", CYAML_FLAG_DEFAULT,
				struct target_struct, value),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_LIBYAML_PARSER) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects flags, but YAML has scalar. */
static bool test_err_schema_expect_flags_read_scalar(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const char * const strings[] = {
		"first",
		"second",
		"third",
		"fourth",
		"fifth",
		"sixth",
	};
	static const unsigned char yaml[] =
		"key: first\n";
	struct target_struct {
		int value;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_FLAGS("key", CYAML_FLAG_DEFAULT,
				struct target_struct, value,
				strings, CYAML_ARRAY_LEN(strings)),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects mapping, but YAML has scalar. */
static bool test_err_schema_expect_mapping_read_scalar(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: scalar\n";
	struct value_s {
		int a;
	};
	struct target_struct {
		struct value_s test_value_mapping;
	} *data_tgt = NULL;
	static const struct cyaml_schema_mapping test_mapping_schema[] = {
		CYAML_MAPPING_INT("a", CYAML_FLAG_DEFAULT, struct value_s, a),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_MAPPING("key", CYAML_FLAG_DEFAULT,
				struct target_struct, test_value_mapping,
				test_mapping_schema),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading when schema expects sequence, but YAML has scalar. */
static bool test_err_schema_expect_sequence_read_scalar(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	static const unsigned char yaml[] =
		"key: foo\n";
	struct target_struct {
		int *a;
		unsigned a_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, *(data_tgt->a)),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("key", CYAML_FLAG_POINTER,
				struct target_struct, a, &entry_schema,
				0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = config,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	err = cyaml_load_data(yaml, YAML_LEN(yaml), config, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_ERR_INVALID_VALUE) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (data_tgt != NULL) {
		return ttest_fail(&tc, "Data non-NULL on error.");
	}

	return ttest_pass(&tc);
}

/* Test loading, with all memory allocation failure at every possible point. */
static bool test_err_free_null(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	ttest_ctx_t tc = ttest_start(report, __func__, NULL, NULL);

	UNUSED(config);

	cyaml_mem(NULL, 0);

	return ttest_pass(&tc);
}

/** Context for allocation failure tests. */
static struct test_cyaml_mem_ctx_s {
	unsigned required;
	unsigned fail;
	unsigned current;
} test_cyaml_mem_ctx;

/* Allocation counter */
static void * test_cyaml_mem_count_allocs(
		void *ptr,
		size_t size)
{
	if (size == 0) {
		free(ptr);
		return NULL;
	}

	test_cyaml_mem_ctx.required++;

	return realloc(ptr, size);
}

/* Allocation failure tester */
static void * test_cyaml_mem_fail(
		void *ptr,
		size_t size)
{
	if (size == 0) {
		free(ptr);
		return NULL;
	}

	if (test_cyaml_mem_ctx.current == test_cyaml_mem_ctx.fail) {
		return NULL;
	}

	test_cyaml_mem_ctx.current++;

	return realloc(ptr, size);
}

/* Test loading, with all memory allocation failure at every possible point. */
static bool test_err_load_alloc_oom_1(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	cyaml_config_t cfg = *config;
	static const unsigned char yaml[] =
		"animals:\n"
		"  - kind: cat\n"
		"    sound: meow\n"
		"    position: [ 1, 2, 1]\n"
		"  - kind: snake\n"
		"    sound: hiss\n"
		"    position: [ 3, 1, 0]\n";
	struct animal_s {
		char *kind;
		char *sound;
		int *position;
	};
	struct target_struct {
		struct animal_s **animal;
		uint32_t animal_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type position_entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_DEFAULT, int),
	};
	static const struct cyaml_schema_mapping animal_schema[] = {
		CYAML_MAPPING_STRING_PTR("kind", CYAML_FLAG_POINTER,
				struct animal_s, kind, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_STRING_PTR("sound", CYAML_FLAG_POINTER,
				struct animal_s, sound, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE_FIXED("position", CYAML_FLAG_POINTER,
				struct animal_s, position,
				&position_entry_schema, 3),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type animal_entry_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER, **(data_tgt->animal),
				animal_schema),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("animals", CYAML_FLAG_POINTER,
				struct target_struct, animal,
				&animal_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = &cfg,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	/*
	 * First we load the YAML with the counting allocation function,
	 * to find the number of allocations required to load the document.
	 * This is deterministic.
	 */
	cfg.mem_fn = test_cyaml_mem_count_allocs;
	test_cyaml_mem_ctx.required = 0;

	err = cyaml_load_data(yaml, YAML_LEN(yaml), &cfg, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_OK) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (test_cyaml_mem_ctx.required == 0) {
		return ttest_fail(&tc, "There were no allocations.");
	}

	/* Now free what was loaded. */
	cyaml_free(config, &top_schema, data_tgt, 0);
	data_tgt = NULL;

	/*
	 * Now we load the document multiple times, forcing every possible
	 * allocation to fail.
	 */
	cfg.mem_fn = test_cyaml_mem_fail;

	for (test_cyaml_mem_ctx.fail = 0;
			test_cyaml_mem_ctx.fail < test_cyaml_mem_ctx.required;
			test_cyaml_mem_ctx.fail++) {
		test_cyaml_mem_ctx.current = 0;
		err = cyaml_load_data(yaml, YAML_LEN(yaml), &cfg, &top_schema,
				(cyaml_data_t **) &data_tgt, NULL);
		if (err != CYAML_ERR_OOM) {
			return ttest_fail(&tc, cyaml_strerror(err));
		}

		/* Now free what was loaded. */
		cyaml_free(config, &top_schema, data_tgt, 0);
		data_tgt = NULL;
	}

	return ttest_pass(&tc);
}

/* Test loading, with all memory allocation failure at every possible point. */
static bool test_err_load_alloc_oom_2(
		ttest_report_ctx_t *report,
		const cyaml_config_t *config)
{
	cyaml_config_t cfg = *config;
	static const unsigned char yaml[] =
		"animals:\n"
		"  - kind: cat\n"
		"    sound: meow\n"
		"    position: [ 1, 2, 1]\n"
		"  - kind: snake\n"
		"    sound: hiss\n"
		"    position: [ 3, 1, 0]\n";
	struct animal_s {
		char *kind;
		char *sound;
		int **position;
		unsigned position_count;
	};
	struct target_struct {
		struct animal_s **animal;
		uint32_t animal_count;
	} *data_tgt = NULL;
	static const struct cyaml_schema_type position_entry_schema = {
		CYAML_TYPE_INT(CYAML_FLAG_POINTER, int),
	};
	static const struct cyaml_schema_mapping animal_schema[] = {
		CYAML_MAPPING_STRING_PTR("kind", CYAML_FLAG_POINTER,
				struct animal_s, kind, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_STRING_PTR("sound", CYAML_FLAG_POINTER,
				struct animal_s, sound, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_SEQUENCE("position", CYAML_FLAG_POINTER,
				struct animal_s, position,
				&position_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type animal_entry_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER, **(data_tgt->animal),
				animal_schema),
	};
	static const struct cyaml_schema_mapping mapping_schema[] = {
		CYAML_MAPPING_SEQUENCE("animals", CYAML_FLAG_POINTER,
				struct target_struct, animal,
				&animal_entry_schema, 0, CYAML_UNLIMITED),
		CYAML_MAPPING_END
	};
	static const struct cyaml_schema_type top_schema = {
		CYAML_TYPE_MAPPING(CYAML_FLAG_POINTER,
				struct target_struct, mapping_schema),
	};
	test_data_t td = {
		.data = (cyaml_data_t **) &data_tgt,
		.config = &cfg,
		.schema = &top_schema,
	};
	cyaml_err_t err;

	ttest_ctx_t tc = ttest_start(report, __func__, cyaml_cleanup, &td);

	/*
	 * First we load the YAML with the counting allocation function,
	 * to find the number of allocations required to load the document.
	 * This is deterministic.
	 */
	cfg.mem_fn = test_cyaml_mem_count_allocs;
	test_cyaml_mem_ctx.required = 0;

	err = cyaml_load_data(yaml, YAML_LEN(yaml), &cfg, &top_schema,
			(cyaml_data_t **) &data_tgt, NULL);
	if (err != CYAML_OK) {
		return ttest_fail(&tc, cyaml_strerror(err));
	}

	if (test_cyaml_mem_ctx.required == 0) {
		return ttest_fail(&tc, "There were no allocations.");
	}

	/* Now free what was loaded. */
	cyaml_free(config, &top_schema, data_tgt, 0);
	data_tgt = NULL;

	/*
	 * Now we load the document multiple times, forcing every possible
	 * allocation to fail.
	 */
	cfg.mem_fn = test_cyaml_mem_fail;

	for (test_cyaml_mem_ctx.fail = 0;
			test_cyaml_mem_ctx.fail < test_cyaml_mem_ctx.required;
			test_cyaml_mem_ctx.fail++) {
		test_cyaml_mem_ctx.current = 0;
		err = cyaml_load_data(yaml, YAML_LEN(yaml), &cfg, &top_schema,
				(cyaml_data_t **) &data_tgt, NULL);
		if (err != CYAML_ERR_OOM) {
			return ttest_fail(&tc, cyaml_strerror(err));
		}

		/* Now free what was loaded. */
		cyaml_free(config, &top_schema, data_tgt, 0);
		data_tgt = NULL;
	}

	return ttest_pass(&tc);
}

/**
 * Run the CYAML error unit tests.
 *
 * \param[in]  rc         The ttest report context.
 * \param[in]  log_level  CYAML log level.
 * \param[in]  log_fn     CYAML logging function, or NULL.
 * \return true iff all unit tests pass, otherwise false.
 */
bool errs_tests(
		ttest_report_ctx_t *rc,
		cyaml_log_t log_level,
		cyaml_log_fn_t log_fn)
{
	bool pass = true;
	cyaml_config_t config = {
		.log_fn = log_fn,
		.mem_fn = cyaml_mem,
		.log_level = log_level,
		.flags = CYAML_CFG_DEFAULT,
	};

	/* Since we expect loads of error logging for these tests,
	 * suppress log output if required log level is greater
	 * than \ref CYAML_LOG_INFO.
	 */
	if (log_level > CYAML_LOG_INFO) {
		config.log_fn = NULL;
	}

	ttest_heading(rc, "Bad parameter tests");

	pass &= test_err_load_null_data(rc, &config);
	pass &= test_err_load_null_config(rc, &config);
	pass &= test_err_load_null_mem_fn(rc, &config);
	pass &= test_err_load_null_schema(rc, &config);
	pass &= test_err_schema_top_level_non_pointer(rc, &config);
	pass &= test_err_schema_top_level_sequence_no_count(rc, &config);
	pass &= test_err_schema_top_level_not_sequence_count(rc, &config);

	ttest_heading(rc, "Bad schema tests");

	pass &= test_err_schema_bad_type(rc, &config);
	pass &= test_err_schema_string_min_max(rc, &config);
	pass &= test_err_schema_bad_data_size_1(rc, &config);
	pass &= test_err_schema_bad_data_size_2(rc, &config);
	pass &= test_err_schema_bad_data_size_3(rc, &config);
	pass &= test_err_schema_bad_data_size_4(rc, &config);
	pass &= test_err_schema_bad_data_size_5(rc, &config);
	pass &= test_err_schema_bad_data_size_6(rc, &config);
	pass &= test_err_schema_sequence_min_max(rc, &config);
	pass &= test_err_schema_bad_data_size_float(rc, &config);
	pass &= test_err_schema_sequence_in_sequence(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: bad values");

	pass &= test_err_schema_invalid_value_unit(rc, &config);
	pass &= test_err_schema_invalid_value_float_range(rc, &config);
	pass &= test_err_schema_invalid_value_double_range(rc, &config);
	pass &= test_err_schema_invalid_value_unit_range_1(rc, &config);
	pass &= test_err_schema_invalid_value_unit_range_2(rc, &config);
	pass &= test_err_schema_invalid_value_unit_range_3(rc, &config);
	pass &= test_err_schema_invalid_value_unit_range_4(rc, &config);
	pass &= test_err_schema_invalid_value_unit_range_5(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: string lengths");

	pass &= test_err_schema_string_min_length(rc, &config);
	pass &= test_err_schema_string_max_length(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: mapping fields");

	pass &= test_err_schema_missing_mapping_field(rc, &config);
	pass &= test_err_schema_unknown_mapping_field(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: sequence counts");

	pass &= test_err_schema_sequence_min_entries(rc, &config);
	pass &= test_err_schema_sequence_max_entries(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: bad flags/enum strings");

	pass &= test_err_schema_flags_mapping(rc, &config);
	pass &= test_err_schema_enum_bad_string(rc, &config);
	pass &= test_err_schema_flags_bad_string(rc, &config);
	pass &= test_err_schema_strict_enum_bad_string(rc, &config);
	pass &= test_err_schema_strict_flags_bad_string(rc, &config);

	ttest_heading(rc, "YAML / schema mismatch: expected value type tests");

	pass &= test_err_schema_expect_int_read_seq(rc, &config);
	pass &= test_err_schema_expect_int_read_end_1(rc, &config);
	pass &= test_err_schema_expect_int_read_end_2(rc, &config);
	pass &= test_err_schema_expect_flags_read_scalar(rc, &config);
	pass &= test_err_schema_expect_mapping_read_scalar(rc, &config);
	pass &= test_err_schema_expect_sequence_read_scalar(rc, &config);

	ttest_heading(rc, "Memory allocation handling tests");

	pass &= test_err_free_null(rc, &config);
	pass &= test_err_load_alloc_oom_1(rc, &config);
	pass &= test_err_load_alloc_oom_2(rc, &config);

	return pass;
}